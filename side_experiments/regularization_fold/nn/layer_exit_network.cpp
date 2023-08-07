#include "layer.h"

using namespace std;

void Layer::exit_hidden_finalize_new_state(int input_layer_index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back(this->weights[n_index][1][input_layer_index]);
		this->weight_updates[n_index][0].push_back(0.0);

		this->weights[n_index][1].erase(this->weights[n_index][1].begin()+input_layer_index);
		this->weight_updates[n_index][1].erase(this->weight_updates[n_index][1].begin()+input_layer_index);
	}
}
