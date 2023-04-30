#include "layer.h"

#include "utilities.h"

using namespace std;

void Layer::state_hidden_add_new_inner() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][2].push_back((randuni()-0.5)*0.02);
		this->weight_updates[n_index][2].push_back(0.0);
	}
}

void Layer::state_hidden_add_new_outer() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][3].push_back((randuni()-0.5)*0.02);
		this->weight_updates[n_index][3].push_back(0.0);
	}
}

void Layer::state_hidden_remove_new_outer() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_zero_new_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][2][index] = 0.0;
		this->weight_updates[n_index][2][index] = 0.0;
	}
}

void Layer::state_hidden_zero_state(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1][index] = 0.0;
		this->weight_updates[n_index][1][index] = 0.0;
	}
}

void Layer::state_hidden_zero_new_outer(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][3][index] = 0.0;
		this->weight_updates[n_index][3][index] = 0.0;
	}
}

void Layer::state_hidden_update_state_size(int state_size_increase) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < state_size_increase; s_index++) {
			this->weights[n_index][1].push_back(0.0);
			this->weight_updates[n_index][1].push_back(0.0);
		}
	}
}

void Layer::state_hidden_new_external_weights_to_state() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int new_outer_state_size = (int)this->input_layers[3]->acti_vals.size();
		for (int o_index = 0; o_index < new_outer_state_size; o_index++) {
			this->weights[n_index][1].push_back(this->weights[n_index][3][o_index]);
			this->weight_updates[n_index][1].push_back(0.0);
		}

		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();

		// also clear new_inner in case empty step in sequence
		this->weights[n_index][2].clear();
		this->weight_updates[n_index][2].clear();
	}
}

void Layer::state_hidden_new_sequence_finalize() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		// move new_outer to state back
		int new_outer_state_size = (int)this->input_layers[3]->acti_vals.size();
		for (int o_index = 0; o_index < new_outer_state_size; o_index++) {
			this->weights[n_index][1].push_back(this->weights[n_index][3][o_index]);
			this->weight_updates[n_index][1].push_back(0.0);
		}
		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();

		// for new_inner, insert back-to-front
		int new_inner_state_size = (int)this->input_layers[2]->acti_vals.size();
		for (int ln_index = new_inner_state_size-1; ln_index >= 0; ln_index--) {
			this->weights[n_index][1].insert(this->weights[n_index][1].begin(),
				this->weights[n_index][2][ln_index]);
			this->weight_updates[n_index][1].insert(this->weight_updates[n_index][1].begin(), 0.0);
		}
		this->weights[n_index][2].clear();
		this->weight_updates[n_index][2].clear();
	}
}

void Layer::state_hidden_remove_state(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].erase(this->weights[n_index][1].begin()+index);
		this->weight_updates[n_index][1].erase(this->weight_updates[n_index][1].begin()+index);
	}
}

void Layer::state_hidden_add_state(int size) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < size; s_index++) {
			this->weights[n_index][1].push_back(0.0);
			this->weight_updates[n_index][1].push_back(0.0);
		}
	}
}
