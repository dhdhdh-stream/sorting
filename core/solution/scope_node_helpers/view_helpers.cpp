#include "scope_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "full_network.h"
#include "scope.h"
#include "state.h"

using namespace std;

void ScopeNode::view_activate(AbstractNode*& curr_node,
							  Problem& problem,
							  vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper) {
	cout << "scope node #" << this->id << endl;

	map<int, StateStatus> input_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				cout << "inner #" << this->input_inner_indexes[i_index] << ": " << state_status.val << endl;
				input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					cout << "inner #" << this->input_inner_indexes[i_index] << ": " << it->second.val << endl;
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			}
		} else {
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index],
																			   this->input_init_index_vals[i_index]);
			cout << "inner #" << this->input_inner_indexes[i_index] << ": " << this->input_init_vals[i_index] << endl;
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
			cout << "iter_index: " << iter_index << endl;

			if (iter_index >= this->max_iters) {
				cout << "loop exceeded limit" << endl;
				break;
			}

			double continue_score = this->continue_score_mod;
			double halt_score = this->halt_score_mod;

			for (int s_index = 0; s_index < (int)this->loop_state_is_local.size(); s_index++) {
				cout << "s_index: " << s_index << endl;
				if (this->loop_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;
							cout << "local normalized: " << normalized << endl;
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;
							cout << "local it->second.val: " << it->second.val << endl;
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
							cout << "input normalized: " << normalized << endl;
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;
							cout << "input it->second.val: " << it->second.val << endl;
						}
					}
				}
			}

			cout << "continue_score: " << continue_score << endl;
			cout << "halt_score: " << halt_score << endl;

			if (halt_score > continue_score) {
				break;
			} else {
				this->inner_scope->view_activate(problem,
												 context,
												 inner_exit_depth,
												 inner_exit_node,
												 run_helper);

				for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
					map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
					if (inner_it != context.back().input_state_vals.end()) {
						if (this->output_outer_is_local[o_index]) {
							context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
							cout << "o local #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
						} else {
							map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
							if (outer_it != context[context.size()-2].input_state_vals.end()) {
								outer_it->second = inner_it->second;
								cout << "o input #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
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
	} else {
		this->inner_scope->view_activate(problem,
										 context,
										 inner_exit_depth,
										 inner_exit_node,
										 run_helper);

		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
			if (inner_it != context.back().input_state_vals.end()) {
				if (this->output_outer_is_local[o_index]) {
					context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
					cout << "o local #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
				} else {
					map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
					if (outer_it != context[context.size()-2].input_state_vals.end()) {
						outer_it->second = inner_it->second;
						cout << "o input #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
					}
				}
			}
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

	cout << endl;
}
