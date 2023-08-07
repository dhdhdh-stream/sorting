#include "layer.h"

using namespace std;

void Layer::state_hidden_finalize_new_state(int input_layer_index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].push_back(this->weights[n_index][2][input_layer_index]);
		this->weight_updates[n_index][1].push_back(0.0);

		this->weights[n_index][2].erase(this->weights[n_index][2].begin()+input_layer_index);
		this->weight_updates[n_index][2].erase(this->weight_updates[n_index][2].begin()+input_layer_index);
	}
}

void Layer::state_hidden_finalize_new_input() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].push_back(this->weights[n_index][3][0]);
		this->weight_updates[n_index][1].push_back(0.0);
	}

	this->weights[n_index][3].clear();
	this->weight_updates[n_index][3].clear();
}
