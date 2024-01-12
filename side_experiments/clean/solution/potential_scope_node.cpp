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
	map<int, StateStatus> local_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
					state_status = it->second;
				}
				local_state_vals[this->input_inner_indexes[i_index]] = state_status;

				if (!this->is_cleaned) {
					it->second.impacted_potential_scopes[this->scope] = set<int>({this->input_inner_indexes[i_index]});
				}
			} else {
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
					local_state_vals[this->input_inner_indexes[i_index]] = it->second;

					if (!this->is_cleaned) {
						it->second.impacted_potential_scopes[this->scope] = set<int>({this->input_inner_indexes[i_index]});
					}
				}
			}
		} else {
			local_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);

			if (!this->is_cleaned) {
				local_state_vals[this->input_inner_indexes[i_index]].impacted_potential_scopes[this->scope] = set<int>({this->input_inner_indexes[i_index]});
			}
		}
	}

	context.back().node = this->scope_node_placeholder;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	context.back().local_state_vals = local_state_vals;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	// no need to set context.back().scope_history

	// unused
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->activate(problem,
						  context,
						  inner_exit_depth,
						  inner_exit_node,
						  run_helper,
						  scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().local_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().local_state_vals.end()) {
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

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		if (it->second.used) {
			for (map<Scope*, set<int>>::iterator scope_it = it->second.impacted_potential_scopes.begin();
					scope_it != it->second.impacted_potential_scopes.end(); scope_it++) {
				for (set<int>::iterator index_it = scope_it->second.begin();
						index_it != scope_it->second.end(); index_it++) {
					scope_it->first->used_states[*index_it] = true;
				}
			}
		}
	}

	context.pop_back();

	context.back().node = NULL;
}

#if defined(MDEBUG) && MDEBUG
void PotentialScopeNode::capture_verify_activate(Problem* problem,
												 vector<ContextLayer>& context,
												 RunHelper& run_helper) {
	// cout << "this->scope_node_placeholder->id: " << this->scope_node_placeholder->id << endl;

	// problem->print();

	// cout << "solution->max_depth: " << solution->max_depth << endl;

	// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;

	map<int, StateStatus> local_state_vals;
	vector<double> full_local_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
					state_status = it->second;
				}
				local_state_vals[this->input_inner_indexes[i_index]] = state_status;
				full_local_state_vals.push_back(state_status.val);
			} else {
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
					local_state_vals[this->input_inner_indexes[i_index]] = it->second;
					full_local_state_vals.push_back(it->second.val);
				}
			}
		} else {
			local_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			full_local_state_vals.push_back(this->input_init_vals[i_index]);
		}
	}

	vector<double> full_input_state_vals;
	for (set<State*>::iterator it = this->used_experiment_states.begin();
			it != this->used_experiment_states.end(); it++) {
		map<State*, StateStatus>::iterator temp_it = context[context.size() - this->experiment_scope_depth].temp_state_vals.find(*it);
		if (temp_it != context[context.size() - this->experiment_scope_depth].temp_state_vals.end()) {
			full_input_state_vals.push_back(temp_it->second.val);
		} else {
			full_input_state_vals.push_back(0.0);
		}
	}

	this->scope_node_placeholder->verify_input_input_state_vals.push_back(full_input_state_vals);
	this->scope_node_placeholder->verify_input_local_state_vals.push_back(full_local_state_vals);

	context.back().node = this->scope_node_placeholder;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	context.back().local_state_vals = local_state_vals;

	// unused
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	this->scope->activate(problem,
						  context,
						  inner_exit_depth,
						  inner_exit_node,
						  run_helper,
						  scope_history);
	delete scope_history;

	vector<double> output_local_state_vals;
	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().local_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().local_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2 - this->output_scope_depths[o_index]].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				output_local_state_vals.push_back(inner_it->second.val);
			} else {
				map<int, StateStatus>::iterator outer_it  = context[context.size()-2
					- this->output_scope_depths[o_index]].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals.end()) {
					outer_it->second = inner_it->second;
					output_local_state_vals.push_back(inner_it->second.val);
				}
			}
		}
	}

	context.pop_back();

	context.back().node = NULL;

	vector<double> output_input_state_vals;
	for (set<State*>::iterator it = this->used_experiment_states.begin();
			it != this->used_experiment_states.end(); it++) {
		map<State*, StateStatus>::iterator temp_it = context[context.size() - this->experiment_scope_depth].temp_state_vals.find(*it);
		if (temp_it != context[context.size() - this->experiment_scope_depth].temp_state_vals.end()) {
			output_input_state_vals.push_back(temp_it->second.val);
		} else {
			output_input_state_vals.push_back(0.0);
		}
	}

	this->scope_node_placeholder->verify_output_input_state_vals.push_back(output_input_state_vals);
	this->scope_node_placeholder->verify_output_local_state_vals.push_back(output_local_state_vals);
}
#endif /* MDEBUG */

PotentialScopeNodeHistory::PotentialScopeNodeHistory(PotentialScopeNode* potential_scope_node) {
	this->potential_scope_node = potential_scope_node;
}

PotentialScopeNodeHistory::PotentialScopeNodeHistory(PotentialScopeNodeHistory* original) {
	this->potential_scope_node = original->potential_scope_node;

	this->scope_history = new ScopeHistory(original->scope_history);
}

PotentialScopeNodeHistory::~PotentialScopeNodeHistory() {
	delete this->scope_history;
}
