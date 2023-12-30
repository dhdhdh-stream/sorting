#if defined(MDEBUG) && MDEBUG

#include "retrain_loop_experiment.h"

using namespace std;

void RetrainLoopExperiment::capture_verify_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		int& inner_exit_depth,
		AbstractNode*& inner_exit_node,
		RunHelper& run_helper,
		ScopeNodeHistory* parent_scope_node_history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	int iter_index = 0;
	while (true) {
		if (iter_index >= this->scope_node->max_iters) {
			break;
		}

		double continue_score = this->scope_node->continue_score_mod;
		double halt_score = this->scope_node->halt_score_mod;

		vector<double> factors;

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

					if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
						factors.push_back(normalized);
					}
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;

					if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
						if (it->second.val != 0.0) {
							factors.push_back(it->second.val);
						}
					}
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

					if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
						factors.push_back(normalized);
					}
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;

					if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
						if (it->second.val != 0.0) {
							factors.push_back(it->second.val);
						}
					}
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

					if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
						factors.push_back(normalized);
					}
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;

					if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
						factors.push_back(it->second.val);
					}
				}
			}
		}

		this->verify_continue_scores.push_back(continue_score);
		this->verify_halt_scores.push_back(halt_score);
		this->verify_factors.push_back(factors);

		bool decision_is_halt;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_halt = false;
		} else {
			decision_is_halt = true;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

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

void RetrainLoopExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		solution->verify_key = this;
		solution->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		solution->verify_seeds = this->verify_seeds;

		finalize();
	}
}

#endif /* MDEBUG */