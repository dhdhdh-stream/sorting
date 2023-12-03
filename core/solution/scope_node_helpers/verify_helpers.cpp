#include "scope_node.h"

#include <algorithm>
#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem& problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								AbstractNode*& exit_node,
								RunHelper& run_helper) {
	map<int, StateStatus> input_state_vals;
	vector<double> verify_input_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				input_state_vals[this->input_inner_indexes[i_index]] = state_status;
				verify_input_state_vals.push_back(state_status.val);
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
					verify_input_state_vals.push_back(it->second.val);
				}
			}
		} else {
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index],
																			   this->input_init_index_vals[i_index]);
			verify_input_state_vals.push_back(this->input_init_vals[i_index]);
		}
	}

	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->inner_scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	if (this->is_loop) {
		int iter_index = 0;
		while (true) {
			if (iter_index >= this->max_iters) {
				break;
			}

			double continue_score = this->continue_score_mod;
			double halt_score = this->halt_score_mod;

			vector<double> factors;

			for (int s_index = 0; s_index < (int)this->loop_state_is_local.size(); s_index++) {
				if (this->loop_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;

							factors.push_back(normalized);
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;

							if (it->second.val != 0.0) {
								factors.push_back(it->second.val);
							}
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context[context.size()-2].input_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;

							factors.push_back(normalized);
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;

							if (it->second.val != 0.0) {
								factors.push_back(it->second.val);
							}
						}
					}
				}
			}

			if (this->verify_key == run_helper.verify_key) {
				// cout << "loop problem:";
				// for (int s_index = 0; s_index < (int)problem.initial_world.size(); s_index++) {
				// 	cout << " " << problem.initial_world[s_index];
				// }
				// cout << endl;

				// cout << "curr_problem:" << endl;
				// problem.print();

				// cout << "solution->max_depth: " << solution->max_depth << endl;

				// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

				sort(factors.begin(), factors.end());
				sort(this->verify_factors[0].begin(), this->verify_factors[0].end());

				if (this->verify_continue_scores[0] != continue_score
						|| this->verify_halt_scores[0] != halt_score
						|| this->verify_factors[0] != factors) {
					cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

					cout << "this->verify_continue_scores[0]: " << this->verify_continue_scores[0] << endl;
					cout << "continue_score: " << continue_score << endl;

					cout << "this->verify_halt_scores[0]: " << this->verify_halt_scores[0] << endl;
					cout << "halt_score: " << halt_score << endl;

					cout << "this->verify_factors[0]" << endl;
					for (int f_index = 0; f_index < (int)this->verify_factors[0].size(); f_index++) {
						cout << f_index << ": " << this->verify_factors[0][f_index] << endl;
					}
					cout << "factors" << endl;
					for (int f_index = 0; f_index < (int)factors.size(); f_index++) {
						cout << f_index << ": " << factors[f_index] << endl;
					}

					throw invalid_argument("loop verify fail");
				}

				this->verify_continue_scores.erase(this->verify_continue_scores.begin());
				this->verify_halt_scores.erase(this->verify_halt_scores.begin());
				this->verify_factors.erase(this->verify_factors.begin());
			}

			#if defined(MDEBUG) && MDEBUG
			bool decision_is_halt;
			if (run_helper.curr_run_seed%2 == 0) {
				decision_is_halt = false;
			} else {
				decision_is_halt = true;
			}
			/**
			 * - reverse to match BranchExperiment capture_verify()
			 */
			run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
			#else
			bool decision_is_halt = halt_score > continue_score;
			#endif /* MDEBUG */

			if (decision_is_halt) {
				break;
			} else {
				if (this->verify_key == run_helper.verify_key) {
					// cout << "loop problem:";
					// for (int s_index = 0; s_index < (int)problem.initial_world.size(); s_index++) {
					// 	cout << " " << problem.initial_world[s_index];
					// }
					// cout << endl;

					// cout << "curr_problem:" << endl;
					// problem.print();

					// cout << "solution->max_depth: " << solution->max_depth << endl;

					// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

					sort(verify_input_state_vals.begin(), verify_input_state_vals.end());
					sort(this->verify_input_state_vals[0].begin(), this->verify_input_state_vals[0].end());

					if (this->verify_input_state_vals[0] != verify_input_state_vals) {
						cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

						cout << "this->parent->id: " << this->parent->id << endl;
						cout << "this->id: " << this->id << endl;

						cout << "this->verify_input_state_vals[0]" << endl;
						for (int v_index = 0; v_index < (int)this->verify_input_state_vals[0].size(); v_index++) {
							cout << v_index << ": " << this->verify_input_state_vals[0][v_index] << endl;
						}

						cout << "verify_input_state_vals" << endl;
						for (int v_index = 0; v_index < (int)verify_input_state_vals.size(); v_index++) {
							cout << v_index << ": " << verify_input_state_vals[v_index] << endl;
						}

						throw invalid_argument("scope node verify fail");
					}

					this->verify_input_state_vals.erase(this->verify_input_state_vals.begin());
				}

				this->inner_scope->verify_activate(problem,
												   context,
												   inner_exit_depth,
												   inner_exit_node,
												   run_helper);

				vector<double> output_state_vals;
				for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
					map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
					if (inner_it != context.back().input_state_vals.end()) {
						if (this->output_outer_is_local[o_index]) {
							context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
							output_state_vals.push_back(inner_it->second.val);
						} else {
							map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
							if (outer_it != context[context.size()-2].input_state_vals.end()) {
								outer_it->second = inner_it->second;
								output_state_vals.push_back(inner_it->second.val);
							}
						}
					}
				}

				if (this->verify_key == run_helper.verify_key) {
					// cout << "scope problem:";
					// for (int s_index = 0; s_index < (int)problem.initial_world.size(); s_index++) {
					// 	cout << " " << problem.initial_world[s_index];
					// }
					// cout << endl;

					// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

					// cout << "this->id: " << this->id << endl;

					sort(output_state_vals.begin(), output_state_vals.end());
					sort(this->verify_output_state_vals[0].begin(), this->verify_output_state_vals[0].end());

					if (this->verify_output_state_vals[0] != output_state_vals) {
						cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

						cout << "this->verify_output_state_vals[0]" << endl;
						for (int v_index = 0; v_index < (int)this->verify_output_state_vals[0].size(); v_index++) {
							cout << v_index << ": " << this->verify_output_state_vals[0][v_index] << endl;
						}

						cout << "output_state_vals" << endl;
						for (int v_index = 0; v_index < (int)output_state_vals.size(); v_index++) {
							cout << v_index << ": " << output_state_vals[v_index] << endl;
						}

						throw invalid_argument("scope node verify fail");
					}

					this->verify_output_state_vals.erase(this->verify_output_state_vals.begin());
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
	} else {
		if (this->verify_key == run_helper.verify_key) {
			// cout << "scope problem:";
			// for (int s_index = 0; s_index < (int)problem.initial_world.size(); s_index++) {
			// 	cout << " " << problem.initial_world[s_index];
			// }
			// cout << endl;

			// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

			// cout << "context scope" << endl;
			// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			// 	cout << c_index << ": " << context[c_index].scope->id << endl;
			// }
			// cout << "context node" << endl;
			// for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			// 	cout << c_index << ": " << context[c_index].node->id << endl;
			// }
			// cout << "current_world";
			// for (int s_index = 0; s_index < (int)problem.current_world.size(); s_index++) {
			// 	cout << " " << problem.current_world[s_index];
			// }
			// cout << endl;
			// cout << "problem.current_pointer: " << problem.current_pointer << endl;

			sort(verify_input_state_vals.begin(), verify_input_state_vals.end());
			sort(this->verify_input_state_vals[0].begin(), this->verify_input_state_vals[0].end());

			if (this->verify_input_state_vals[0] != verify_input_state_vals) {
				cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

				cout << "this->parent->id: " << this->parent->id << endl;
				cout << "this->id: " << this->id << endl;

				cout << "this->verify_input_state_vals[0]" << endl;
				for (int v_index = 0; v_index < (int)this->verify_input_state_vals[0].size(); v_index++) {
					cout << v_index << ": " << this->verify_input_state_vals[0][v_index] << endl;
				}

				cout << "verify_input_state_vals" << endl;
				for (int v_index = 0; v_index < (int)verify_input_state_vals.size(); v_index++) {
					cout << v_index << ": " << verify_input_state_vals[v_index] << endl;
				}

				throw invalid_argument("scope node verify fail");
			}

			this->verify_input_state_vals.erase(this->verify_input_state_vals.begin());
		}

		this->inner_scope->verify_activate(problem,
										   context,
										   inner_exit_depth,
										   inner_exit_node,
										   run_helper);

		vector<double> output_state_vals;
		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
			if (inner_it != context.back().input_state_vals.end()) {
				if (this->output_outer_is_local[o_index]) {
					context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
					output_state_vals.push_back(inner_it->second.val);
				} else {
					map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
					if (outer_it != context[context.size()-2].input_state_vals.end()) {
						outer_it->second = inner_it->second;
						output_state_vals.push_back(inner_it->second.val);
					}
				}
			}
		}

		if (this->verify_key == run_helper.verify_key) {
			// cout << "scope problem:";
			// for (int s_index = 0; s_index < (int)problem.initial_world.size(); s_index++) {
			// 	cout << " " << problem.initial_world[s_index];
			// }
			// cout << endl;

			// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

			// cout << "this->id: " << this->id << endl;

			sort(output_state_vals.begin(), output_state_vals.end());
			sort(this->verify_output_state_vals[0].begin(), this->verify_output_state_vals[0].end());

			if (this->verify_output_state_vals[0] != output_state_vals) {
				cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

				cout << "this->verify_output_state_vals[0]" << endl;
				for (int v_index = 0; v_index < (int)this->verify_output_state_vals[0].size(); v_index++) {
					cout << v_index << ": " << this->verify_output_state_vals[0][v_index] << endl;
				}

				cout << "output_state_vals" << endl;
				for (int v_index = 0; v_index < (int)output_state_vals.size(); v_index++) {
					cout << v_index << ": " << output_state_vals[v_index] << endl;
				}

				throw invalid_argument("scope node verify fail");
			}

			this->verify_output_state_vals.erase(this->verify_output_state_vals.begin());
		}
	}

	context.pop_back();

	context.back().node = NULL;

	if (inner_exit_depth == -1) {
		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
