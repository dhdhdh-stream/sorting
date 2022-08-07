#include "layer.h"

#include <cmath>
#include <iostream>
#include <sstream>

#include "utilities.h"

using namespace std;

Layer::Layer(int type,
			 int num_nodes) {
	this->type = type;

	for (int n_index = 0; n_index < num_nodes; n_index++) {
		this->acti_vals.push_back(0.0);
		this->errors.push_back(0.0);
	}
}

Layer::~Layer() {
	// do nothing
}

void Layer::setup_weights_full() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		vector<vector<double>> node_weights;
		vector<vector<double>> node_weight_updates;
		vector<vector<double>> node_prev_weight_updates;

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();

			vector<double> layer_weights;
			vector<double> layer_weight_updates;
			vector<double> layer_prev_weight_updates;
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				layer_weights.push_back((randnorm()-0.5)*0.02);
				layer_weight_updates.push_back(0.0);
				layer_prev_weight_updates.push_back(0.0);
			}
			node_weights.push_back(layer_weights);
			node_weight_updates.push_back(layer_weight_updates);
			node_prev_weight_updates.push_back(layer_prev_weight_updates);
		}

		this->weights.push_back(node_weights);
		this->constants.push_back((randnorm()-0.5)*0.02);
		this->weight_updates.push_back(node_weight_updates);
		this->constant_updates.push_back(0.0);
		this->prev_weight_updates.push_back(node_prev_weight_updates);
		this->prev_constant_updates.push_back(0.0);
	}
}

void Layer::collapse_input(int input_index,
						   Layer* new_collapse_layer) {
	this->input_layers[input_index] = new_collapse_layer;

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][input_index].clear();
		this->weight_updates[n_index][input_index].clear();
		this->prev_weight_updates[n_index][input_index].clear();

		int layer_size = (int)new_collapse_layer->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			this->weights[n_index][input_index].push_back((randnorm()-0.5)*0.02);
			this->weight_updates[n_index][input_index].push_back(0.0);
			this->prev_weight_updates[n_index][input_index].push_back(0.0);
		}
	}
}

void Layer::fold_input(Layer* new_fold_layer) {
	this->input_layers.erase(this->input_layers.begin());
	this->input_layers[0] = new_fold_layer;

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index].erase(this->weights[n_index].begin());
		this->weights[n_index][0].clear();
		this->weight_updates[n_index].erase(this->weight_updates[n_index].begin());
		this->weight_updates[n_index][0].clear();
		this->prev_weight_updates[n_index].erase(this->prev_weight_updates[n_index].begin());
		this->prev_weight_updates[n_index][0].clear();

		int layer_size = (int)new_fold_layer->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			this->weights[n_index][0].push_back((randnorm()-0.5)*0.02);
			this->weight_updates[n_index][0].push_back(0.0);
			this->prev_weight_updates[n_index][0].push_back(0.0);
		}
	}
}

void Layer::copy_weights_from(Layer* original) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->weights[n_index][l_index][ln_index] = original->weights[n_index][l_index][ln_index];
			}
		}
		this->constants[n_index] = original->constants[n_index];
	}
}

void Layer::activate() {
	if (this->type == LINEAR_LAYER) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			double sum_val = this->constants[n_index];

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					sum_val += this->input_layers[l_index]->acti_vals[ln_index]
							   *this->weights[n_index][l_index][ln_index];
				}
			}

			this->acti_vals[n_index] = sum_val;
		}
	} else {
		// this->type == RELU_LAYER
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			double sum_val = this->constants[n_index];

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					sum_val += this->input_layers[l_index]->acti_vals[ln_index]
							   *this->weights[n_index][l_index][ln_index];
				}
			}

			if (sum_val > 0.0) {
				this->acti_vals[n_index] = sum_val;
			} else {
				this->acti_vals[n_index] = 0.0;
			}
		}
	}
}

void Layer::backprop() {
	if (this->type == LINEAR_LAYER) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					this->input_layers[l_index]->errors[ln_index] +=
						this->errors[n_index]*this->weights[n_index][l_index][ln_index];
					// multiply by this->errors[n_index] for MSE weight updates
					this->weight_updates[n_index][l_index][ln_index] +=
						this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
				}
			}

			this->constant_updates[n_index] += this->errors[n_index];

			this->errors[n_index] = 0.0;
		}
	} else {
		// this->type == RELU_LAYER
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] > 0.0) {
				for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
					int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
					for (int ln_index = 0; ln_index < layer_size; ln_index++) {
						this->input_layers[l_index]->errors[ln_index] +=
							this->errors[n_index]*this->weights[n_index][l_index][ln_index];
						// multiply by this->errors[n_index] for MSE weight updates
						this->weight_updates[n_index][l_index][ln_index] +=
							this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
					}
				}

				this->constant_updates[n_index] += this->errors[n_index];
			}

			this->errors[n_index] = 0.0;
		}
	}
}

void Layer::calc_max_update(int input_index,
							double& max_update_size,
							double learning_rate,
							double momentum) {
	if (input_index == -1) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					double update = this->weight_updates[n_index][l_index][ln_index]
									*learning_rate
									+this->prev_weight_updates[n_index][l_index][ln_index]
									*momentum;
					double update_size = abs(update);
					if (update_size > max_update_size) {
						max_update_size = update_size;
					}
				}
			}

			double update = this->constant_updates[n_index]
							*learning_rate
							+this->prev_constant_updates[n_index]
							*momentum;
			double update_size = abs(update);
			if (update_size > max_update_size) {
				max_update_size = update_size;
			}
		}
	} else {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < input_index+1; l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					double update = this->weight_updates[n_index][l_index][ln_index]
									*learning_rate
									+this->prev_weight_updates[n_index][l_index][ln_index]
									*momentum;
					double update_size = abs(update);
					if (update_size > max_update_size) {
						max_update_size = update_size;
					}
				}
			}
		}
	}
}

void Layer::update_weights(int input_index,
						   double factor,
						   double learning_rate,
						   double momentum) {
	if (input_index == -1) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					double update = this->weight_updates[n_index][l_index][ln_index]
									*learning_rate
									+this->prev_weight_updates[n_index][l_index][ln_index]
									*momentum;
					update *= factor;
					this->weight_updates[n_index][l_index][ln_index] = 0.0;
					this->weights[n_index][l_index][ln_index] += update;
					this->prev_weight_updates[n_index][l_index][ln_index] = update;
				}
			}

			double update = this->constant_updates[n_index]
							*learning_rate
							+this->prev_constant_updates[n_index]
							*momentum;
			update *= factor;
			this->constant_updates[n_index] = 0.0;
			this->constants[n_index] += update;
			this->prev_constant_updates[n_index] = update;
		}
	} else {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < input_index+1; l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					double update = this->weight_updates[n_index][l_index][ln_index]
									*learning_rate
									+this->prev_weight_updates[n_index][l_index][ln_index]
									*momentum;
					update *= factor;
					this->weight_updates[n_index][l_index][ln_index] = 0.0;
					this->weights[n_index][l_index][ln_index] += update;
					this->prev_weight_updates[n_index][l_index][ln_index] = update;
				}
			}
		}
	}
}
