#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <algorithm>
#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
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
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
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

	if (this->verify_key == run_helper.verify_key) {
		// cout << "this->id: " << this->id << endl;

		// problem->print();

		// cout << "solution->max_depth: " << solution->max_depth << endl;

		// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

		// cout << "context scope" << endl;
		// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		// 	cout << c_index << ": " << context[c_index].scope->id << endl;
		// }
		// cout << "context node" << endl;
		// for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		// 	cout << c_index << ": " << context[c_index].node->id << endl;
		// }

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

	if (!run_helper.has_exited && !run_helper.exceeded_limit) {
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
			if (inner_exit_depth != -1 || inner_exit_node != NULL) {
				cout << "inner_exit_depth: " << inner_exit_depth << endl;
				if (inner_exit_node != NULL) {
					cout << "inner_exit_node->id: " << inner_exit_node->id << endl;
				}

				cout << "context scope" << endl;
				for (int c_index = 0; c_index < (int)context.size(); c_index++) {
					cout << c_index << ": " << context[c_index].scope->id << endl;
				}
				cout << "context node" << endl;
				for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
					cout << c_index << ": " << context[c_index].node->id << endl;
				}

				throw invalid_argument("inner_exit_depth != -1 || inner_exit_node != NULL");
			}

			// problem->print();

			// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

			// cout << "this->id: " << this->id << endl;

			// cout << "context scope" << endl;
			// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			// 	cout << c_index << ": " << context[c_index].scope->id << endl;
			// }
			// cout << "context node" << endl;
			// for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			// 	cout << c_index << ": " << context[c_index].node->id << endl;
			// }

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

#endif /* MDEBUG */