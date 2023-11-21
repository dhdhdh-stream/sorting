#include "potential_scope_node.h"

#include <iostream>

#include "scope.h"
#include "scope_node.h"

using namespace std;

PotentialScopeNode::PotentialScopeNode() {
	this->scope_node_placeholder = NULL;
}

PotentialScopeNode::~PotentialScopeNode() {
	if (this->scope != NULL) {
		delete this->scope;
	}

	if (this->scope_node_placeholder != NULL) {
		delete this->scope_node_placeholder;
	}
}

void PotentialScopeNode::activate(Problem& problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  PotentialScopeNodeHistory* history) {
	map<int, StateStatus> input_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_types[i_index] == OUTER_TYPE_INPUT) {
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].input_state_vals.find((long)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			} else if (this->input_outer_types[i_index] == OUTER_TYPE_LOCAL) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].local_state_vals.find((long)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
					state_status = it->second;
				}
				input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<State*, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].temp_state_vals.find((State*)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].temp_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			}
		} else {
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
		}
	}

	context.back().node = this->scope_node_placeholder;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;

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
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_types[o_index] == OUTER_TYPE_INPUT) {
				map<int, StateStatus>::iterator outer_it  = context[context.size()-2
					- this->output_scope_depths[o_index]].input_state_vals.find((long)this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals.end()) {
					context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals[(long)this->output_outer_indexes[o_index]] = inner_it->second;
				}
			} else if (this->output_outer_types[o_index] == OUTER_TYPE_LOCAL) {
				context[context.size()-2 - this->output_scope_depths[o_index]].local_state_vals[(long)this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				context[context.size()-2 - this->output_scope_depths[o_index]].temp_state_vals[(State*)this->output_outer_indexes[o_index]] = inner_it->second;
			}
		}
	}

	context.pop_back();

	context.back().node = NULL;
}

void PotentialScopeNode::capture_verify_activate(Problem& problem,
												 vector<ContextLayer>& context,
												 RunHelper& run_helper) {
	map<int, StateStatus> input_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_types[i_index] == OUTER_TYPE_INPUT) {
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].input_state_vals.find((long)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			} else if (this->input_outer_types[i_index] == OUTER_TYPE_LOCAL) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].local_state_vals.find((long)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
					state_status = it->second;
				}
				input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<State*, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].temp_state_vals.find((State*)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].temp_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			}
		} else {
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
		}
	}

	this->scope_node_placeholder->verify_input_state_vals.push_back(input_state_vals);

	context.back().node = this->scope_node_placeholder;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;

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

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_types[o_index] == OUTER_TYPE_INPUT) {
				map<int, StateStatus>::iterator outer_it  = context[context.size()-2
					- this->output_scope_depths[o_index]].input_state_vals.find((long)this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals.end()) {
					context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals[(long)this->output_outer_indexes[o_index]] = inner_it->second;
				}
			} else if (this->output_outer_types[o_index] == OUTER_TYPE_LOCAL) {
				context[context.size()-2 - this->output_scope_depths[o_index]].local_state_vals[(long)this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				context[context.size()-2 - this->output_scope_depths[o_index]].temp_state_vals[(State*)this->output_outer_indexes[o_index]] = inner_it->second;
			}
		}
	}

	context.pop_back();

	context.back().node = NULL;
}

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
