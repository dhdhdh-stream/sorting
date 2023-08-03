#include "sequence.h"

using namespace std;

void Sequence::explore_activate_pull(vector<double>& input_vals,
									 vector<ForwardContextLayer>& context,
									 vector<vector<double>>& previous_vals,
									 RunHelper& run_helper) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			input_vals[i_index] += context[context.size()-1 - this->input_local_scope_depths[i_index]]
				.state_vals->at(this->input_local_input_indexes[i_index]);
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			input_vals[i_index] += previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]];
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LAST_SEEN) {
			map<int, double>::iterator it = run_helper.last_seen_vals.find(this->input_last_seen_class_ids[i_index]);
			if (it != run_helper.last_seen_vals.end()) {
				input_vals[i_index] += it->second;
			}
		}
	}
}

void Sequence::explore_activate_reset(vector<double>& input_vals,
									  vector<ForwardContextLayer>& context,
									  vector<vector<double>>& previous_vals) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			context[context.size()-1 - this->input_local_scope_depths[i_index]]
				.state_vals->at(this->input_local_input_indexes[i_index]) = input_vals[i_index];
		} else if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_PREVIOUS) {
			previous_vals[this->input_previous_step_index[i_index]][this->input_previous_input_index[i_index]] = input_vals[i_index];
		}
	}
}
