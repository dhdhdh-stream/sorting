#include "sequence.h"

#include <iostream>

#include "scope.h"
#include "scope_node.h"

using namespace std;

Sequence::Sequence() {
	// do nothing
}

Sequence::~Sequence() {
	if (this->scope != NULL) {
		delete this->scope;
	}
}

void Sequence::activate(Problem& problem,
						vector<ContextLayer>& context,
						RunHelper& run_helper,
						SequenceHistory* history) {
	// no need to set context.back().node_id

	context.push_back(ContextLayer());

	context.back().scope_id = this->scope->id;
	context.back().node_id = -1;

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].local_state_vals.end()) {
					state_status = it->second;
				}
				context.back().input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<int, StateStatus>::iterator it = context[context.size()-1
					- this->input_scope_depths[i_index]].input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context[context.size()-1 - this->input_scope_depths[i_index]].input_state_vals.end()) {
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

	vector<int> starting_node_ids{0};
	vector<map<int, StateStatus>> starting_input_state_vals;
	vector<map<int, StateStatus>> starting_local_state_vals;

	// unused
	int inner_exit_depth = -1;
	int inner_exit_node_id = -1;

	this->scope->activate(starting_node_ids,
						  starting_input_state_vals,
						  starting_local_state_vals,
						  problem,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2 - this->output_scope_depths[o_index]].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				map<int, StateStatus>::iterator outer_it  = context[context.size()-2
					- this->output_scope_depths[o_index]].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals.end()) {
					context[context.size()-2 - this->output_scope_depths[o_index]].input_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				}
			}
		}
	}

	for (int n_index = 0; n_index < (int)context.back().added_recursion_protection_flags.size(); n_index++) {
		run_helper.recursion_protection_flags.erase(context.back().added_recursion_protection_flags[n_index]);
	}
	context.pop_back();

	// no need to set context.back().node_id
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
