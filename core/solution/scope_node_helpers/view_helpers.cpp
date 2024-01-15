#include "scope_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "state.h"

using namespace std;

void ScopeNode::view_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper) {
	cout << "scope node #" << this->id << endl;

	map<int, StateStatus> input_state_vals;
	map<int, StateStatus> local_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				if (this->input_inner_is_local[i_index]) {
					cout << "inner local #" << this->input_inner_indexes[i_index] << ": " << state_status.val << endl;
					local_state_vals[this->input_inner_indexes[i_index]] = state_status;
				} else {
					cout << "inner input #" << this->input_inner_indexes[i_index] << ": " << state_status.val << endl;
					input_state_vals[this->input_inner_indexes[i_index]] = state_status;
				}
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					if (this->input_inner_is_local[i_index]) {
						cout << "inner local #" << this->input_inner_indexes[i_index] << ": " << it->second.val << endl;
						local_state_vals[this->input_inner_indexes[i_index]] = it->second;
					} else {
						cout << "inner input #" << this->input_inner_indexes[i_index] << ": " << it->second.val << endl;
						input_state_vals[this->input_inner_indexes[i_index]] = it->second;
					}
				}
			}
		} else {
			if (this->input_inner_is_local[i_index]) {
				local_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
				cout << "inner local #" << this->input_inner_indexes[i_index] << ": " << this->input_init_vals[i_index] << endl;
			} else {
				input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
				cout << "inner input #" << this->input_inner_indexes[i_index] << ": " << this->input_init_vals[i_index] << endl;
			}
		}
	}

	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->inner_scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;
	context.back().local_state_vals = local_state_vals;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->inner_scope->view_activate(problem,
									 context,
									 inner_exit_depth,
									 inner_exit_node,
									 run_helper);

	if (!run_helper.has_exited) {
		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			if (this->output_inner_is_local[o_index]) {
				map<int, StateStatus>::iterator inner_it = context.back().local_state_vals.find(this->output_inner_indexes[o_index]);
				if (inner_it != context.back().local_state_vals.end()) {
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
			} else {
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
