#include "layer.h"

#include <cmath>
#include <iostream>
#include <sstream>

#include "utilities.h"

using namespace std;

void Layer::fold_add_scope(Layer* new_scope_input) {
	this->input_layers.push_back(new_scope_input);

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int layer_size = (int)new_scope_input->acti_vals.size();

		vector<double> layer_weights;
		vector<double> layer_weight_updates;
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			layer_weights.push_back((randuni()-0.5)*0.02);
			layer_weight_updates.push_back(0.0);
		}
		this->weights[n_index].push_back(layer_weights);
		this->weight_updates[n_index].push_back(layer_weight_updates);
	}
}

void Layer::fold_pop_scope() {
	this->input_layers.pop_back();

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index].pop_back();
		this->weight_updates[n_index].pop_back();
	}
}

void Layer::fold_activate(int fold_index) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		double sum_val = this->constants[n_index];

		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				sum_val += this->input_layers[l_index]->acti_vals[ln_index]
						   *this->weights[n_index][l_index][ln_index];
			}
		}

		if (sum_val > 0.0) {
			this->acti_vals[n_index] = sum_val;
		} else {
			this->acti_vals[n_index] = 0.01*sum_val;
		}
	}
}

void Layer::fold_backprop_weights_with_no_error_signal(int fold_index) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}
		this->constant_updates[n_index] += this->errors[n_index];

		this->errors[n_index] = 0.0;
	}
}

void Layer::fold_backprop_last_state(int fold_index) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}
		this->constant_updates[n_index] += this->errors[n_index];

		// state_size > 0
		int layer_size = (int)this->input_layers.back()->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			this->input_layers.back()->errors[ln_index] +=
				this->errors[n_index]*this->weights[n_index].back()[ln_index];
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::fold_get_max_update(int fold_index,
								double& max_update_size) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update_size = abs(this->weight_updates[n_index][l_index][ln_index]);
				if (update_size > max_update_size) {
					max_update_size = update_size;
				}
			}
		}

		double update_size = abs(this->constant_updates[n_index]);
		if (update_size > max_update_size) {
			max_update_size = update_size;
		}
	}
}

void Layer::fold_update_weights(int fold_index,
								double learning_rate) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update = this->weight_updates[n_index][l_index][ln_index]
								*learning_rate;
				this->weight_updates[n_index][l_index][ln_index] = 0.0;
				this->weights[n_index][l_index][ln_index] += update;
			}
		}

		double update = this->constant_updates[n_index]
						*learning_rate;
		this->constant_updates[n_index] = 0.0;
		this->constants[n_index] += update;
	}
}
