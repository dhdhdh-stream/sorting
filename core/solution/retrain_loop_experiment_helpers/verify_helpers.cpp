#include "retrain_loop_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void RetrainLoopExperiment::verify_activate(
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

		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals.begin();
					it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals.end(); it++) {
				double continue_weight = 0.0;
				map<int, double>::iterator continue_weight_it = this->continue_input_state_weights[c_index].find(it->first);
				if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
					continue_weight = continue_weight_it->second;
				}
				double halt_weight = 0.0;
				map<int, double>::iterator halt_weight_it = this->halt_input_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_input_state_weights[c_index].end()) {
					halt_weight = halt_weight_it->second;
				}

				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					continue_score += continue_weight * normalized;
					halt_score += halt_weight * normalized;
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals.begin();
					it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals.end(); it++) {
				double continue_weight = 0.0;
				map<int, double>::iterator continue_weight_it = this->continue_local_state_weights[c_index].find(it->first);
				if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
					continue_weight = continue_weight_it->second;
				}
				double halt_weight = 0.0;
				map<int, double>::iterator halt_weight_it = this->halt_local_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_local_state_weights[c_index].end()) {
					halt_weight = halt_weight_it->second;
				}

				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					continue_score += continue_weight * normalized;
					halt_score += halt_weight * normalized;
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			for (map<State*, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals.begin();
					it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals.end(); it++) {
				double continue_weight = 0.0;
				map<State*, double>::iterator continue_weight_it = this->continue_temp_state_weights[c_index].find(it->first);
				if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
					continue_weight = continue_weight_it->second;
				}
				double halt_weight = 0.0;
				map<State*, double>::iterator halt_weight_it = this->halt_temp_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_temp_state_weights[c_index].end()) {
					halt_weight = halt_weight_it->second;
				}

				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					continue_score += continue_weight * normalized;
					halt_score += halt_weight * normalized;
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;
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
		if (abs(halt_score - continue_score) > DECISION_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
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

void RetrainLoopExperiment::verify_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= 2 * solution->curr_num_datapoints) {
		this->combined_score /= (2 * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(2 * solution->curr_num_datapoints));

		if (combined_improvement_t_score > 2.326) {	// >99%
		#endif /* MDEBUG */
			cout << "Retrain Loop" << endl;
			cout << "verify" << endl;

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;
			cout << "score_standard_deviation: " << score_standard_deviation << endl;
			cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			this->state = RETRAIN_LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			finalize();
			#endif /* MDEBUG */
		} else {
			for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
				delete this->new_states[s_index];
			}
			this->new_states.clear();

			this->state = RETRAIN_LOOP_EXPERIMENT_STATE_FAIL;
		}
	}
}
