#include "layer.h"

using namespace std;

void Layer::score_hidden_finalize_new_state(int new_total_states) {
	int size_diff = new_total_states - (int)this->input_layers[0]->acti_vals.size();
	int new_state_size = (int)this->input_layers[1]->acti_vals.size();
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < size_diff; s_index++) {
			this->weights[n_index][0].push_back(0.0);
			this->weight_updates[n_index][0].push_back(0.0);
		}

		for (int s_index = 0; s_index < new_state_size; s_index++) {
			this->weights[n_index][0][new_total_states - new_state_size + s_index]
				= this->weights[n_index][1][s_index];
		}
	}

	this->weights[n_index][1].clear();
	this->weight_updates[n_index][1].clear();
}

void Layer::score_hidden_add_state() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back(0.0);
		this->weight_updates[n_index][0].push_back(0.0);
	}
}
