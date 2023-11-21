#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem& problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								AbstractNode*& exit_node,
								RunHelper& run_helper) {
	map<int, StateStatus> input_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			}
		} else {
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
		}
	}

	if (this->verify_key == run_helper.verify_key) {
		if (this->verify_input_state_vals[0] != input_state_vals) {
			cout << "this->verify_input_state_vals[0]" << endl;
			for (map<int, StateStatus>::iterator it = this->verify_input_state_vals[0].begin();
					it != this->verify_input_state_vals[0].end(); it++) {
				cout << "it->first: " << it->first << endl;
				cout << "it->second.val: " << it->second.val << endl;
				cout << "it->second.last_network: " << it->second.last_network << endl;
			}

			cout << "input_state_vals" << endl;
			for (map<int, StateStatus>::iterator it = input_state_vals.begin();
					it != input_state_vals.end(); it++) {
				cout << "it->first: " << it->first << endl;
				cout << "it->second.val: " << it->second.val << endl;
				cout << "it->second.last_network: " << it->second.last_network << endl;
			}

			throw invalid_argument("scope node verify fail");
		}

		this->verify_input_state_vals.erase(this->verify_input_state_vals.begin());
	}

	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->inner_scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->inner_scope->verify_activate(problem,
									   context,
									   inner_exit_depth,
									   inner_exit_node,
									   run_helper);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2].input_state_vals.end()) {
					outer_it->second = inner_it->second;
				}
			}
		}
	}
	/**
	 * - intuitively, pass by reference out
	 *   - so keep even if early exit
	 * 
	 * - also will be how inner branches affect outer scopes on early exit
	 */

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
