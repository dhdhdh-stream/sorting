#include "layer.h"

using namespace std;

const double LASSO_PERCENTAGE = 0.1;

void Layer::state_hidden_lasso_update_weights(double learning_rate,
											  double target_max_update) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		// obs
		{
			int layer_size = (int)this->input_layers[0]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update = this->weight_updates[n_index][0][ln_index]
								*learning_rate;
				this->weight_updates[n_index][0][ln_index] = 0.0;
				this->weights[n_index][0][ln_index] += update;
			}
		}

		// state
		{
			double update = this->weight_updates[n_index][1][0]
							*learning_rate;
			this->weight_updates[n_index][1][0] = 0.0;
			this->weights[n_index][1][0] += update;

			double lasso_weight = LASSO_PERCENTAGE*target_max_update;
			if (this->weights[n_index][1][0] > lasso_weight) {
				this->weights[n_index][1][0] -= lasso_weight;
			} else if (this->weights[n_index][1][0] < -lasso_weight) {
				this->weights[n_index][1][0] += lasso_weight;
			} else {
				this->weights[n_index][1][0] = 0.0;
			}
		}

		double update = this->constant_updates[n_index]
						*learning_rate;
		this->constant_updates[n_index] = 0.0;
		this->constants[n_index] += update;
	}
}
