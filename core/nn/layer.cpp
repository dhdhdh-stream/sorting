#include "layer.h"

#include <cmath>
#include <iostream>
#include <random>
#include <sstream>

#include "constants.h"
#include "globals.h"

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
	uniform_real_distribution<double> distribution(-0.01, 0.01);
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		vector<vector<double>> node_weights;
		vector<vector<double>> node_weight_updates;

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();

			vector<double> layer_weights;
			vector<double> layer_weight_updates;
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				layer_weights.push_back(distribution(generator));
				layer_weight_updates.push_back(0.0);
			}
			node_weights.push_back(layer_weights);
			node_weight_updates.push_back(layer_weight_updates);
		}

		this->weights.push_back(node_weights);
		this->constants.push_back(distribution(generator));
		this->weight_updates.push_back(node_weight_updates);
		this->constant_updates.push_back(0.0);
	}
}

void Layer::add_input(Layer* layer) {
	this->input_layers.push_back(layer);

	int layer_size = (int)layer->acti_vals.size();
	uniform_real_distribution<double> distribution(-0.01, 0.01);
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		vector<double> layer_weights;
		vector<double> layer_weight_updates;
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			layer_weights.push_back(distribution(generator));
			layer_weight_updates.push_back(0.0);
		}
		this->weights[n_index].push_back(layer_weights);
		this->weight_updates[n_index].push_back(layer_weight_updates);
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

void Layer::load_weights_from(ifstream& input_file) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			string line;
			getline(input_file, line);
			stringstream stream;
			stream.str(line);
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				string item;
				getline(stream, item, ',');
				this->weights[n_index][l_index][ln_index] = stod(item);
			}
		}
		string line;
		getline(input_file, line);
		this->constants[n_index] = stod(line);
	}
}

void Layer::activate() {
	switch (this->type) {
	case LINEAR_LAYER:
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

		break;
	case RELU_LAYER:
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

		break;
	case LEAKY_LAYER:
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
				this->acti_vals[n_index] = 0.01*sum_val;
			}
		}

		break;
	case SIGMOID_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			double sum_val = this->constants[n_index];

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					sum_val += this->input_layers[l_index]->acti_vals[ln_index]
							   *this->weights[n_index][l_index][ln_index];
				}
			}

			this->acti_vals[n_index] = 1.0 / (1.0 + exp(-sum_val));
		}

		break;
	}
}

void Layer::backprop() {
	switch (this->type) {
	case LINEAR_LAYER:
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

		break;
	case RELU_LAYER:
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

		break;
	case LEAKY_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] < 0.0) {
				this->errors[n_index] *= 0.01;
			}

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

		break;
	case SIGMOID_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			this->errors[n_index] *= (this->acti_vals[n_index] * (1.0 - this->acti_vals[n_index]));

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

		break;
	}
}

void Layer::get_max_update(double& max_update_size) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
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

void Layer::update_weights(double learning_rate) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
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

void Layer::save_weights(ofstream& output_file) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				if (abs(this->weights[n_index][l_index][ln_index]) < MIN_WEIGHT) {
					this->weights[n_index][l_index][ln_index] = 0.0;
				}
				output_file << this->weights[n_index][l_index][ln_index] << ",";
			}
			output_file << endl;
		}
		if (abs(this->constants[n_index]) < MIN_WEIGHT) {
			this->constants[n_index] = 0.0;
		}
		output_file << this->constants[n_index] << endl;
	}
}
