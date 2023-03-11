#include "layer.h"

#include "utilities.h"

using namespace std;

void Layer::state_hidden_backprop_new_state() {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}

		this->constant_updates[n_index] += this->errors[n_index];

		int layer_size = (int)this->input_layers[3]->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			this->input_layers[3]->errors[ln_index] +=
				this->errors[n_index]*this->weights[n_index][3][ln_index];
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::state_hidden_add_new_inner() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][3].push_back((randuni()-0.5)*0.02);
		this->weight_updates[n_index][3].push_back(0.0);
	}
}

void Layer::state_hidden_add_new_outer() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][4].push_back((randuni()-0.5)*0.02);
		this->weight_updates[n_index][4].push_back(0.0);
	}
}

void Layer::state_hidden_zero_new_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][3][index] = 0.0;
		this->weight_updates[n_index][3][index] = 0.0;
	}
}

void Layer::state_hidden_zero_local(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1][index] = 0.0;
		this->weight_updates[n_index][1][index] = 0.0;
	}
}

void Layer::state_hidden_zero_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][2][index] = 0.0;
		this->weight_updates[n_index][2][index] = 0.0;
	}
}

void Layer::state_hidden_zero_new_outer(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][4][index] = 0.0;
		this->weight_updates[n_index][4][index] = 0.0;
	}
}

void Layer::state_hidden_update_state_sizes(int local_state_size_increase,
											int input_state_size_increase) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < local_state_size_increase; l_index++) {
			this->weights[n_index][1].push_back(0.0);
			this->weight_updates[n_index][1].push_back(0.0);
		}

		for (int i_index = 0; i_index < input_state_size_increase; i_index++) {
			this->weights[n_index][2].push_back(0.0);
			this->weight_updates[n_index][2].push_back(0.0);
		}
	}
}

void Layer::state_hidden_new_outer_weights_to_local() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int new_outer_state_size = (int)this->input_layers[4]->acti_vals.size();
		for (int o_index = 0; o_index < new_outer_state_size; o_index++) {
			this->weights[n_index][1].push_back(this->weights[n_index][4][o_index]);
			this->weight_updates[n_index][1].push_back(0.0);
		}

		this->weights[n_index][4].clear();
		this->weight_updates[n_index][4].clear();

		// also clear new_inner in case empty step in sequence
		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_new_outer_weights_to_input() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int new_outer_state_size = (int)this->input_layers[4]->acti_vals.size();
		for (int o_index = 0; o_index < new_outer_state_size; o_index++) {
			this->weights[n_index][2].push_back(this->weights[n_index][4][o_index]);
			this->weight_updates[n_index][2].push_back(0.0);
		}

		this->weights[n_index][4].clear();
		this->weight_updates[n_index][4].clear();

		// also clear new_inner in case empty step in sequence
		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_split_new_inner(int split_index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		// move new_outer to input_state back
		int new_outer_state_size = (int)this->input_layers[4]->acti_vals.size();
		for (int o_index = 0; o_index < new_outer_state_size; o_index++) {
			this->weights[n_index][2].push_back(this->weights[n_index][4][o_index]);
			this->weight_updates[n_index][2].push_back(0.0);
		}
		this->weights[n_index][4].clear();
		this->weight_updates[n_index][4].clear();

		// for 2nd half new_inner and original local_state to input_state, insert back-to-front
		int original_local_state_size = (int)this->input_layers[1]->acti_vals.size();
		for (int ln_index = original_local_state_size-1; ln_index >= 0; ln_index--) {
			this->weights[n_index][2].insert(this->weights[n_index][2].begin(),
				this->weights[n_index][1][ln_index]);
			this->weight_updates[n_index][2].insert(this->weight_updates[n_index][2].begin(), 0.0);
		}
		this->weights[n_index][1].clear();
		this->weight_updates[n_index][1].clear();

		int new_inner_state_size = (int)this->input_layers[3]->acti_vals.size();
		for (int ln_index = new_inner_state_size-1; ln_index >= split_index; ln_index--) {
			this->weights[n_index][2].insert(this->weights[n_index][2].begin(),
				this->weights[n_index][3][ln_index]);
			this->weight_updates[n_index][2].insert(this->weight_updates[n_index][2].begin(), 0.0);
		}

		// move remaining new_inner to local_state
		for (int ln_index = 0; ln_index < split_index; ln_index++) {
			this->weights[n_index][1].push_back(this->weights[n_index][3][ln_index]);
			this->weight_updates[n_index][1].push_back(0.0);
		}
		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_remove_local(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].erase(this->weights[n_index][1].begin()+index);
		this->weight_updates[n_index][1].erase(this->weight_updates[n_index][1].begin()+index);
	}
}

void Layer::state_hidden_remove_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][2].erase(this->weights[n_index][2].begin()+index);
		this->weight_updates[n_index][2].erase(this->weight_updates[n_index][2].begin()+index);
	}
}

void Layer::state_hidden_add_local(int size) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < size; s_index++) {
			this->weights[n_index][1].push_back(0.0);
			this->weight_updates[n_index][1].push_back(0.0);
		}
	}
}

void Layer::state_hidden_add_input(int size) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < size; s_index++) {
			this->weights[n_index][2].push_back(0.0);
			this->weight_updates[n_index][2].push_back(0.0);
		}
	}
}
