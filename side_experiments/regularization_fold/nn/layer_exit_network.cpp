#include "layer.h"

using namespace std;

void Layer::exit_hidden_finalize_new_state() {
	int new_state_size = (int)this->input_layers[1]->acti_vals.size();
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < new_state_size; s_index++) {
			this->weights[n_index][0].push_back(this->weights[n_index][1][s_index]);
			this->weight_updates[n_index][0].push_back(0.0);
		}
	}

	this->weights[n_index][1].clear();
	this->weight_updates[n_index][1].clear();
}
