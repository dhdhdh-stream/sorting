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

void Layer::state_hidden_update_state_sizes(int input_state_size_increase,
											int local_state_size_increase) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int i_index = 0; i_index < input_state_size_increase; i_index++) {
			this->weights[n_index][1].push_back(0.0);
			this->weight_updates[n_index][1].push_back(0.0);
		}

		for (int l_index = 0; l_index < local_state_size_increase; l_index++) {
			this->weights[n_index][2].push_back(0.0);
			this->weight_updates[n_index][2].push_back(0.0);
		}
	}
}

void Layer::state_hidden_new_weights_to_local() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][2].push_back(this->weights[n_index][3][0]);
		this->weights[n_index][3].clear();

		this->weight_updates[n_index][2].push_back(0.0);
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_new_weights_to_input() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].push_back(this->weights[n_index][3][0]);
		this->weights[n_index][3].clear();

		this->weight_updates[n_index][1].push_back(0.0);
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_split_new(int split_index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int original_local_state_size = (int)this->input_layers[2]->acti_vals.size();
		for (int ln_index = 0; ln_index < original_local_state_size; ln_index++) {
			this->weights[n_index][1].push_back(this->weights[n_index][2][ln_index]);
			this->weight_updates[n_index][1].push_back(0.0);
		}
		this->weights[n_index][2].clear();
		this->weight_updates[n_index][2].clear();

		int original_new_state_size = (int)this->input_layers[3]->acti_vals.size();
		for (int ln_index = 0; ln_index < split_index; ln_index++) {
			this->weights[n_index][1].push_back(this->weights[n_index][3][ln_index]);
			this->weight_updates[n_index][1].push_back(0.0);
		}
		for (int ln_index = split_index; ln_index < original_new_state_size; ln_index++) {
			this->weights[n_index][2].push_back(this->weights[n_index][3][ln_index]);
			this->weight_updates[n_index][2].push_back(0.0);
		}
		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();
	}
}

void Layer::state_hidden_remove_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].erase(this->weights[n_index][1].begin()+index);
		this->weight_updates[n_index][1].erase(this->weight_updates[n_index][1].begin()+index);
	}
}

void Layer::state_hidden_remove_local(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][2].erase(this->weights[n_index][2].begin()+index);
		this->weight_updates[n_index][2].erase(this->weight_updates[n_index][2].begin()+index);
	}
}
