#include "layer.h"

using namespace std;

void Layer::state_hidden_scaled_update_weights(double learning_rate) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		// obs
		{
			double update = this->weight_updates[n_index][0][0]
							*learning_rate;
			this->weight_updates[n_index][0][0] = 0.0;
			this->weights[n_index][0][0] += update;
		}

		// state
		{
			int layer_size = (int)this->input_layers[1]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update = this->weight_updates[n_index][1][ln_index]
								*learning_rate;
				this->weight_updates[n_index][1][ln_index] = 0.0;
				this->weights[n_index][1][ln_index] += update;
			}
		}

		// new state
		{
			for (int ln_index = 0; ln_index < 10; ln_index++) {
				double update = this->weight_updates[n_index][2][ln_index]
								*learning_rate;
				this->weight_updates[n_index][2][ln_index] = 0.0;

				if (this->weights[n_index][2][ln_index] > 0.0
						&& update > 0.0) {
					double scale = 0.5 + 0.05*(10-ln_index);
					this->weights[n_index][2][ln_index] += scale*update;
				} else if (this->weights[n_index][2][ln_index] < 0.0
						&& update < 0.0) {
					double scale = 0.5 + 0.05*(10-ln_index);
					this->weights[n_index][2][ln_index] += scale*update;
				} else {
					this->weights[n_index][2][ln_index] += update;
				}
			}
		}

		// new input
		{
			int layer_size = (int)this->input_layers[3]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update = this->weight_updates[n_index][3][ln_index]
								*learning_rate;
				this->weight_updates[n_index][3][ln_index] = 0.0;
				this->weights[n_index][3][ln_index] += update;
			}
		}

		double update = this->constant_updates[n_index]
						*learning_rate;
		this->constant_updates[n_index] = 0.0;
		this->constants[n_index] += update;
	}
}

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

		this->weights[n_index][3].clear();
		this->weight_updates[n_index][3].clear();
	}
}
