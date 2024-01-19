#include "potential_scope_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

PotentialScopeNode::PotentialScopeNode() {
	this->scope_node_placeholder = NULL;

	this->is_cleaned = true;
}

PotentialScopeNode::~PotentialScopeNode() {
	if (this->scope != NULL) {
		delete this->scope;
	}

	if (this->scope_node_placeholder != NULL) {
		delete this->scope_node_placeholder;
	}
}

void PotentialScopeNode::activate(Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  PotentialScopeNodeHistory* history) {
	if (!run_helper.has_exited && !run_helper.exceeded_limit) {
		map<int, StateStatus> input_state_vals;
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				if (this->input_outer_is_local[i_index]) {
					StateStatus state_status;
					map<int, StateStatus>::iterator it = context[context.size()-1
						- this->input_scope_depths[i_index]].local_state_vals.find(this->input_outer_indexes[i_index]);
					if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
						state_status = it->second;
					}
					input_state_vals[this->input_inner_indexes[i_index]] = state_status;

					if (!this->is_cleaned) {
						input_state_vals[this->input_inner_indexes[i_index]]
							.impacted_potential_scopes[this->scope] = {
								set<int>({this->input_inner_indexes[i_index]}),
								set<int>()
							};
					}
				} else {
					map<int, StateStatus>::iterator it = context[context.size()-1
						- this->input_scope_depths[i_index]].input_state_vals.find(this->input_outer_indexes[i_index]);
					if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
						input_state_vals[this->input_inner_indexes[i_index]] = it->second;

						if (!this->is_cleaned) {
							input_state_vals[this->input_inner_indexes[i_index]]
								.impacted_potential_scopes[this->scope] = {
									set<int>({this->input_inner_indexes[i_index]}),
									set<int>()
								};
						}
					}
				}
			} else {
				input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);

				if (!this->is_cleaned) {
					input_state_vals[this->input_inner_indexes[i_index]]
						.impacted_potential_scopes[this->scope] = {
							set<int>({this->input_inner_indexes[i_index]}),
							set<int>()
						};
				}
			}
		}

		context.back().node = this->scope_node_placeholder;

		context.push_back(ContextLayer());

		context.back().scope = this->scope;
		context.back().node = NULL;

		context.back().input_state_vals = input_state_vals;

		ScopeHistory* scope_history = new ScopeHistory(this->scope);
		history->scope_history = scope_history;
		context.back().scope_history = scope_history;

		// unused
		int inner_exit_depth = -1;
		AbstractNode* inner_exit_node = NULL;

		this->scope->activate(problem,
							  context,
							  inner_exit_depth,
							  inner_exit_node,
							  run_helper,
							  scope_history);

		if (!run_helper.has_exited && !run_helper.exceeded_limit) {
			for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
				map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
				if (inner_it != context.back().input_state_vals.end()) {
					if (this->output_outer_is_local[o_index]) {
						context[context.size()-2 - this->output_scope_depths[o_index]].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
					} else {
						map<int, StateStatus>::iterator outer_it  = context[context.size()-2
							- this->output_scope_depths[o_index]].input_state_vals.find(this->output_outer_indexes[o_index]);
						if (outer_it != context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals.end()) {
							outer_it->second = inner_it->second;
						}
					}
				}
			}
		}

		context.pop_back();

		context.back().node = NULL;
	}
}

#if defined(MDEBUG) && MDEBUG
void PotentialScopeNode::capture_verify_activate(Problem* problem,
												 vector<ContextLayer>& context,
												 RunHelper& run_helper) {
	if (!run_helper.has_exited && !run_helper.exceeded_limit) {
		// cout << "this->scope_node_placeholder->id: " << this->scope_node_placeholder->id << endl;

		// problem->print();

		// cout << "solution->max_depth: " << solution->max_depth << endl;

		// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

		map<int, StateStatus> input_state_vals;
		vector<double> full_input_state_vals;
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				if (this->input_outer_is_local[i_index]) {
					StateStatus state_status;
					map<int, StateStatus>::iterator it = context[context.size()-1
						- this->input_scope_depths[i_index]].local_state_vals.find(this->input_outer_indexes[i_index]);
					if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
						state_status = it->second;
					}
					input_state_vals[this->input_inner_indexes[i_index]] = state_status;
					full_input_state_vals.push_back(state_status.val);
				} else {
					map<int, StateStatus>::iterator it = context[context.size()-1
						- this->input_scope_depths[i_index]].input_state_vals.find(this->input_outer_indexes[i_index]);
					if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
						input_state_vals[this->input_inner_indexes[i_index]] = it->second;
						full_input_state_vals.push_back(it->second.val);
					}
				}
			} else {
				input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
				full_input_state_vals.push_back(this->input_init_vals[i_index]);
			}
		}

		for (set<State*>::iterator it = this->used_experiment_states.begin();
				it != this->used_experiment_states.end(); it++) {
			map<State*, StateStatus>::iterator temp_it = context[context.size() - this->experiment_scope_depth].temp_state_vals.find(*it);
			if (temp_it != context[context.size() - this->experiment_scope_depth].temp_state_vals.end()) {
				full_input_state_vals.push_back(temp_it->second.val);
			} else {
				full_input_state_vals.push_back(0.0);
			}
		}

		this->scope_node_placeholder->verify_input_state_vals.push_back(full_input_state_vals);

		context.back().node = this->scope_node_placeholder;

		context.push_back(ContextLayer());

		context.back().scope = this->scope;
		context.back().node = NULL;

		context.back().input_state_vals = input_state_vals;

		ScopeHistory* scope_history = new ScopeHistory(this->scope);
		context.back().scope_history = scope_history;

		// unused
		int inner_exit_depth = -1;
		AbstractNode* inner_exit_node = NULL;

		this->scope->activate(problem,
							  context,
							  inner_exit_depth,
							  inner_exit_node,
							  run_helper,
							  scope_history);
		delete scope_history;

		vector<double> output_state_vals;
		if (!run_helper.has_exited && !run_helper.exceeded_limit) {
			for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
				map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
				if (inner_it != context.back().input_state_vals.end()) {
					if (this->output_outer_is_local[o_index]) {
						context[context.size()-2 - this->output_scope_depths[o_index]].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
						output_state_vals.push_back(inner_it->second.val);
					} else {
						map<int, StateStatus>::iterator outer_it  = context[context.size()-2
							- this->output_scope_depths[o_index]].input_state_vals.find(this->output_outer_indexes[o_index]);
						if (outer_it != context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals.end()) {
							outer_it->second = inner_it->second;
							output_state_vals.push_back(inner_it->second.val);
						}
					}
				}
			}
		}

		context.pop_back();

		context.back().node = NULL;

		if (!run_helper.has_exited && !run_helper.exceeded_limit) {
			// cout << "this->scope_node_placeholder->id: " << this->scope_node_placeholder->id << endl;

			// problem->print();

			// cout << "solution->max_depth: " << solution->max_depth << endl;

			// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

			// cout << "context scope" << endl;
			// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			// 	if (context[c_index].scope != NULL) {
			// 		cout << c_index << ": " << context[c_index].scope->id << endl;
			// 	}
			// }
			// cout << "context node" << endl;
			// for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			// 	cout << c_index << ": " << context[c_index].node->id << endl;
			// }

			for (set<State*>::iterator it = this->used_experiment_states.begin();
					it != this->used_experiment_states.end(); it++) {
				map<State*, StateStatus>::iterator temp_it = context[context.size() - this->experiment_scope_depth].temp_state_vals.find(*it);
				if (temp_it != context[context.size() - this->experiment_scope_depth].temp_state_vals.end()) {
					output_state_vals.push_back(temp_it->second.val);
				} else {
					output_state_vals.push_back(0.0);
				}
			}

			this->scope_node_placeholder->verify_output_state_vals.push_back(output_state_vals);
		}
	}
}
#endif /* MDEBUG */

PotentialScopeNodeHistory::PotentialScopeNodeHistory(PotentialScopeNode* potential_scope_node) {
	this->potential_scope_node = potential_scope_node;

	this->scope_history = NULL;
}

PotentialScopeNodeHistory::PotentialScopeNodeHistory(PotentialScopeNodeHistory* original) {
	this->potential_scope_node = original->potential_scope_node;

	if (original->scope_history == NULL) {
		this->scope_history = NULL;
	} else {
		this->scope_history = new ScopeHistory(original->scope_history);
	}
}

PotentialScopeNodeHistory::~PotentialScopeNodeHistory() {
	if (this->scope_history != NULL) {
		delete this->scope_history;
	}
}
