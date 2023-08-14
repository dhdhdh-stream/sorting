#include "layer.h"

using namespace std;

void Layer::score_hidden_scaled_update_weights(double learning_rate) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		// state
		{
			int layer_size = (int)this->input_layers[0]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update = this->weight_updates[n_index][0][ln_index]
								*learning_rate;
				this->weight_updates[n_index][0][ln_index] = 0.0;
				this->weights[n_index][0][ln_index] += update;
			}
		}

		// new state
		{
			for (int ln_index = 0; ln_index < 10; ln_index++) {
				double update = this->weight_updates[n_index][1][ln_index]
								*learning_rate;
				this->weight_updates[n_index][1][ln_index] = 0.0;
				
				if (this->weights[n_index][1][ln_index] > 0.0
						&& update > 0.0) {
					double scale = 0.5 + 0.05*(9-ln_index);
					this->weights[n_index][1][ln_index] += scale*update;
				} else if (this->weights[n_index][1][ln_index] < 0.0
						&& update < 0.0) {
					double scale = 0.5 + 0.05*(9-ln_index);
					this->weights[n_index][1][ln_index] += scale*update;
				} else {
					this->weights[n_index][1][ln_index] += update;
				}
			}
		}

		double update = this->constant_updates[n_index]
						*learning_rate;
		this->constant_updates[n_index] = 0.0;
		this->constants[n_index] += update;
	}
}

void Layer::score_hidden_finalize_new_state(int new_state_index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back(this->weights[n_index][1][new_state_index]);
		this->weight_updates[n_index][0].push_back(0.0);
	}
}

void Layer::score_hidden_cleanup_new_states() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].clear();
		this->weight_updates[n_index][1].clear();
	}
}

void Layer::score_hidden_add_state() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back(0.0);
		this->weight_updates[n_index][0].push_back(0.0);
	}
}
