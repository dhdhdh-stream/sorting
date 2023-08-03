#include "sequence.h"

using namespace std;

void Sequence::wrapup_activate_pull(vector<double>& input_vals,
									vector<ForwardContextLayer>& context) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_index_translations[i_index] != -1) {
			input_vals[i_index] = context.back().state_vals[this->input_index_translations[i_index]];
		}
	}
}

void Sequence::wrapup_activate_reset(vector<double>& input_vals,
									 vector<ForwardContextLayer>& context) {
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_index_translations[i_index] != -1) {
			context.back().state_vals[this->input_index_translations[i_index]] = input_vals[i_index];
		}
	}
}
