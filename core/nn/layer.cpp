#include "layer.h"

#include <cmath>
#include <iostream>
#include <sstream>

#include "utilities.h"

using namespace std;

Layer::Layer() {
	this->input_layers = NULL;
}

Layer::~Layer() {
	if (this->input_layers != NULL) {
		delete[] this->input_layers;
	}
	for (int n_index = 0; n_index < (int)this->weights.size(); n_index++) {
		delete[] this->weights[n_index];
		delete[] this->weight_updates[n_index];
		delete[] this->prev_weight_updates[n_index];
	}
}

void Layer::setup(int type,
				  int num_input_layers,
				  Layer** input_layers) {
	this->type = type;
	this->num_input_layers = num_input_layers;
	this->input_layers = input_layers;
}

void Layer::add_node() {
	this->acti_vals.push_back(0.0);
	this->errors.push_back(0.0);
}

void Layer::add_weights() {
	while (this->weights.size() < this->acti_vals.size()) {
		this->weights.push_back(new vector<double>[this->num_input_layers]);
		this->constants.push_back((randnorm()-0.5)*0.02);
		this->weight_updates.push_back(new vector<double>[this->num_input_layers]);
		this->constant_updates.push_back(0.0);
		this->prev_weight_updates.push_back(new vector<double>[this->num_input_layers]);
		this->prev_constant_updates.push_back(0.0);
	}

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			while ((int)this->weights[n_index][l_index].size() < layer_size) {
				this->weights[n_index][l_index].push_back((randnorm()-0.5)*0.02);
				this->weight_updates[n_index][l_index].push_back(0.0);
				this->prev_weight_updates[n_index][l_index].push_back(0.0);
			}
		}
	}
}

void Layer::copy_weights_from(Layer& original) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->weights[n_index][l_index][ln_index] = original.weights[n_index][l_index][ln_index];
			}
		}
		this->constants[n_index] = original.constants[n_index];
	}
}

void Layer::copy_weights_from(ifstream& input_file) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			string line;
			getline(input_file, line);
			stringstream stream;
			stream.str(line);
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				string item;
				getline(stream, item, ',');
				this->weights[n_index][l_index][ln_index] = atof(item.c_str());
			}
		}
		string line;
		getline(input_file, line);
		this->constants[n_index] = atof(line.c_str());
	}
}

void Layer::activate() {
	if (this->type == LINEAR_LAYER) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			double sum_val = this->constants[n_index];

			for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
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

			for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
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
			for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
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
				for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
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

void Layer::calc_max_update(double& max_update_size,
							double learning_rate,
							double momentum) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
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
}

void Layer::update_weights(double factor,
						   double learning_rate,
						   double momentum) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
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
}

void Layer::save_weights(ofstream& output_file) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < this->num_input_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				output_file << this->weights[n_index][l_index][ln_index] << ",";
			}
			output_file << endl;
		}
		output_file << this->constants[n_index] << endl;
	}
}
