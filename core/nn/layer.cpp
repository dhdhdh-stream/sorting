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
				layer_weights.push_back((randuni()-0.5)*0.02);
				layer_weight_updates.push_back(0.0);
				layer_prev_weight_updates.push_back(0.0);
			}
			node_weights.push_back(layer_weights);
			node_weight_updates.push_back(layer_weight_updates);
			node_prev_weight_updates.push_back(layer_prev_weight_updates);
		}

		this->weights.push_back(node_weights);
		this->constants.push_back((randuni()-0.5)*0.02);
		this->weight_updates.push_back(node_weight_updates);
		this->constant_updates.push_back(0.0);
		this->prev_weight_updates.push_back(node_prev_weight_updates);
		this->prev_constant_updates.push_back(0.0);
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
				this->weights[n_index][l_index][ln_index] = stof(item);
			}
		}
		string line;
		getline(input_file, line);
		this->constants[n_index] = stof(line);
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

void Layer::calc_max_update(double& max_update_size,
							double learning_rate,
							double momentum) {
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
}

void Layer::update_weights(double factor,
						   double learning_rate,
						   double momentum) {
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
}

void Layer::save_weights(ofstream& output_file) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				output_file << this->weights[n_index][l_index][ln_index] << ",";
			}
			output_file << endl;
		}
		output_file << this->constants[n_index] << endl;
	}
}

void Layer::backprop_errors_with_no_weight_change() {
	if (this->type == LINEAR_LAYER) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					this->input_layers[l_index]->errors[ln_index] +=
						this->errors[n_index]*this->weights[n_index][l_index][ln_index];
				}
			}

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
					}
				}
			}

			this->errors[n_index] = 0.0;
		}
	}
}

void Layer::insert_input_layer(int layer_index,
							   Layer* layer) {
	this->input_layers.insert(this->input_layers.begin() + layer_index, layer);

	int layer_size = (int)layer->acti_vals.size();

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		vector<double> layer_weights;
		vector<double> layer_weight_updates;
		vector<double> layer_prev_weight_updates;
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			layer_weights.push_back((randuni()-0.5)*0.02);
			layer_weight_updates.push_back(0.0);
			layer_prev_weight_updates.push_back(0.0);
		}
		this->weights[n_index].insert(this->weights[n_index].begin() + layer_index, layer_weights);
		this->weight_updates[n_index].insert(this->weight_updates[n_index].begin() + layer_index, layer_weight_updates);
		this->prev_weight_updates[n_index].insert(this->prev_weight_updates[n_index].begin() + layer_index, layer_prev_weight_updates);
	}
}

void Layer::add_nodes(int num_nodes) {
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		this->acti_vals.push_back(0.0);
		this->errors.push_back(0.0);

		vector<vector<double>> node_weights;
		vector<vector<double>> node_weight_updates;
		vector<vector<double>> node_prev_weight_updates;

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();

			vector<double> layer_weights;
			vector<double> layer_weight_updates;
			vector<double> layer_prev_weight_updates;
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				layer_weights.push_back((randuni()-0.5)*0.02);
				layer_weight_updates.push_back(0.0);
				layer_prev_weight_updates.push_back(0.0);
			}
			node_weights.push_back(layer_weights);
			node_weight_updates.push_back(layer_weight_updates);
			node_prev_weight_updates.push_back(layer_prev_weight_updates);
		}

		this->weights.push_back(node_weights);
		this->constants.push_back((randuni()-0.5)*0.02);
		this->weight_updates.push_back(node_weight_updates);
		this->constant_updates.push_back(0.0);
		this->prev_weight_updates.push_back(node_prev_weight_updates);
		this->prev_constant_updates.push_back(0.0);
	}
}

void Layer::output_input_extend(int num_nodes) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int nn_index = 0; nn_index < num_nodes; nn_index++) {
			this->weights[n_index][0].push_back((randuni()-0.5)*0.02);
			this->weight_updates[n_index][0].push_back(0.0);
			this->prev_weight_updates[n_index][0].push_back(0.0);
		}
	}
}

void Layer::backprop_fold_state() {
	// this->type == RELU_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] > 0.0) {
			int layer_size = (int)this->input_layers[2]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->input_layers[2]->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index][2][ln_index];
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][2][ln_index] +=
					this->errors[n_index]*this->input_layers[2]->acti_vals[ln_index];
			}
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::calc_max_update_fold_state(double& max_update_size,
									   double learning_rate,
									   double momentum) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int layer_size = (int)this->input_layers[2]->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			double update = this->weight_updates[n_index][2][ln_index]
							*learning_rate
							+this->prev_weight_updates[n_index][2][ln_index]
							*momentum;
			double update_size = abs(update);
			if (update_size > max_update_size) {
				max_update_size = update_size;
			}
		}
	}
}

void Layer::update_weights_fold_state(double factor,
									  double learning_rate,
									  double momentum) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		int layer_size = (int)this->input_layers[2]->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			double update = this->weight_updates[n_index][2][ln_index]
							*learning_rate
							+this->prev_weight_updates[n_index][2][ln_index]
							*momentum;
			update *= factor;
			this->weight_updates[n_index][2][ln_index] = 0.0;
			this->weights[n_index][2][ln_index] += update;
			this->prev_weight_updates[n_index][2][ln_index] = update;
		}
	}
}
