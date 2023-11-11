#include "sequence.h"

#include <iostream>

#include "scope.h"
#include "scope_node.h"

using namespace std;

Sequence::Sequence() {
	this->scope_node_placeholder = NULL;
}

Sequence::~Sequence() {
	if (this->scope != NULL) {
		delete this->scope;
	}

	if (this->scope_node_placeholder != NULL) {
		delete this->scope_node_placeholder;
	}
}

void Sequence::activate(Problem& problem,
						vector<ContextLayer>& context,
						RunHelper& run_helper,
						SequenceHistory* history) {
	if (this->scope_node_placeholder != NULL) {
		context.back().node_id = this->scope_node_placeholder->id;
	}

	context.push_back(ContextLayer());

	context.back().scope_id = this->scope->id;
	context.back().node_id = -1;

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_types[i_index] == OUTER_TYPE_INPUT) {
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].input_state_vals.find((long)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
					context.back().input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			} else if (this->input_outer_types[i_index] == OUTER_TYPE_LOCAL) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].local_state_vals.find((long)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
					state_status = it->second;
				}
				context.back().input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<State*, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].temp_state_vals.find((State*)this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].temp_state_vals.end()) {
					context.back().input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			}
		} else {
			context.back().input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
		}
	}

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	// no need to set context.back().scope_history

	vector<AbstractNode*> starting_nodes{this->starting_node};
	vector<map<int, StateStatus>> starting_input_state_vals;
	vector<map<int, StateStatus>> starting_local_state_vals;

	// unused
	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->activate(starting_nodes,
						  starting_input_state_vals,
						  starting_local_state_vals,
						  problem,
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

	context.back().node_id = -1;
}

SequenceHistory::SequenceHistory(Sequence* sequence) {
	this->sequence = sequence;
}

SequenceHistory::SequenceHistory(SequenceHistory* original) {
	this->sequence = original->sequence;

	this->scope_history = new ScopeHistory(original->scope_history);
}

SequenceHistory::~SequenceHistory() {
	delete this->scope_history;
}
