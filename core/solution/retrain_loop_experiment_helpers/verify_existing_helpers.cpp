#include "retrain_loop_experiment.h"

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void RetrainLoopExperiment::verify_existing_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		int& inner_exit_depth,
		AbstractNode*& inner_exit_node,
		RunHelper& run_helper,
		ScopeNodeHistory* parent_scope_node_history) {
	int iter_index = 0;
	while (true) {
		if (iter_index >= this->scope_node->max_iters) {
			break;
		}

		double continue_score = this->scope_node->continue_score_mod;
		double halt_score = this->scope_node->halt_score_mod;

		for (int s_index = 0; s_index < (int)this->scope_node->loop_state_is_local.size(); s_index++) {
			if (this->scope_node->loop_state_is_local[s_index]) {
				map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
				if (it != context[context.size()-2].local_state_vals.end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						continue_score += this->scope_node->loop_continue_weights[s_index] * normalized;
						halt_score += this->scope_node->loop_halt_weights[s_index] * normalized;
					} else {
						continue_score += this->scope_node->loop_continue_weights[s_index] * it->second.val;
						halt_score += this->scope_node->loop_halt_weights[s_index] * it->second.val;
					}
				}
			} else {
				map<int, StateStatus>::iterator it = context[context.size()-2].input_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
				if (it != context[context.size()-2].input_state_vals.end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						continue_score += this->scope_node->loop_continue_weights[s_index] * normalized;
						halt_score += this->scope_node->loop_halt_weights[s_index] * normalized;
					} else {
						continue_score += this->scope_node->loop_continue_weights[s_index] * it->second.val;
						halt_score += this->scope_node->loop_halt_weights[s_index] * it->second.val;
					}
				}
			}
		}

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_halt;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_halt = false;
		} else {
			decision_is_halt = true;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_halt;
		if (abs(halt_score - continue_score) > DECISION_MIN_SCORE_IMPACT * this->scope_node->decision_standard_deviation) {
			decision_is_halt = halt_score > continue_score;
		} else {
			uniform_int_distribution<int> distribution(0, 1);
			decision_is_halt = distribution(generator);
		}
		#endif /* MDEBUG */

		if (decision_is_halt) {
			break;
		} else {
			this->scope_node->inner_scope->activate(problem,
													context,
													inner_exit_depth,
													inner_exit_node,
													run_helper,
													iter_index,
													parent_scope_node_history->inner_scope_history);

			for (int o_index = 0; o_index < (int)this->scope_node->output_inner_indexes.size(); o_index++) {
				if (this->scope_node->output_inner_is_local[o_index]) {
					map<int, StateStatus>::iterator inner_it = context.back().local_state_vals.find(this->scope_node->output_inner_indexes[o_index]);
					if (inner_it != context.back().local_state_vals.end()) {
						if (this->scope_node->output_outer_is_local[o_index]) {
							context[context.size()-2].local_state_vals[this->scope_node->output_outer_indexes[o_index]] = inner_it->second;
						} else {
							map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->scope_node->output_outer_indexes[o_index]);
							if (outer_it != context[context.size()-2].input_state_vals.end()) {
								outer_it->second = inner_it->second;
							}
						}
					}
				} else {
					map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->scope_node->output_inner_indexes[o_index]);
					if (inner_it != context.back().input_state_vals.end()) {
						if (this->scope_node->output_outer_is_local[o_index]) {
							context[context.size()-2].local_state_vals[this->scope_node->output_outer_indexes[o_index]] = inner_it->second;
						} else {
							map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->scope_node->output_outer_indexes[o_index]);
							if (outer_it != context[context.size()-2].input_state_vals.end()) {
								outer_it->second = inner_it->second;
							}
						}
					}
				}
			}

			if (inner_exit_depth != -1
					|| run_helper.exceeded_limit) {
				break;
			} else {
				iter_index++;
				// continue
			}
		}
	}
}

void RetrainLoopExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;

			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
		}
	}

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->verify_existing_average_score = sum_scores / solution->curr_num_datapoints;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->verify_existing_score_variance = sum_score_variance / solution->curr_num_datapoints;

		this->o_target_val_histories.clear();

		this->state = RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
	}
}
