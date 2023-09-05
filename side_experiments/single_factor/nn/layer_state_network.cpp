#include "layer.h"

#include "state_network.h"

using namespace std;

void Layer::state_hidden_backprop() {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < STATE_NETWORK_HIDDEN_SIZE; n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		// obs
		{
			this->weight_updates[n_index][0][0] += this->errors[n_index]*this->input_layers[0]->acti_vals[0];
		}

		// state
		{
			this->input_layers[1]->errors[0] += this->errors[n_index]*this->weights[n_index][1][0];
			this->weight_updates[n_index][1][0] += this->errors[n_index]*this->input_layers[1]->acti_vals[0];
		}

		this->constant_updates[n_index] += this->errors[n_index];

		this->errors[n_index] = 0.0;
	}
}
