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

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();

			vector<double> layer_weights;
			vector<double> layer_weight_updates;
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				layer_weights.push_back((randuni()-0.5)*0.02);
				layer_weight_updates.push_back(0.0);
			}
			node_weights.push_back(layer_weights);
			node_weight_updates.push_back(layer_weight_updates);
		}

		this->weights.push_back(node_weights);
		this->constants.push_back((randuni()-0.5)*0.02);
		this->weight_updates.push_back(node_weight_updates);
		this->constant_updates.push_back(0.0);
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
	} else if (this->type == RELU_LAYER) {
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
	} else {
		// this->type == LEAKY_LAYER
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
	} else if (this->type == RELU_LAYER) {
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
	} else {
		// this->type == LEAKY_LAYER
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
	} else if (this->type == RELU_LAYER) {
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
	} else {
		// this->type == LEAKY_LAYER
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] < 0.0) {
				this->errors[n_index] *= 0.01;
			}

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
				for (int ln_index = 0; ln_index < layer_size; ln_index++) {
					this->input_layers[l_index]->errors[ln_index] +=
						this->errors[n_index]*this->weights[n_index][l_index][ln_index];
				}
			}

			this->errors[n_index] = 0.0;
		}
	}
}

void Layer::backprop_weights_with_no_error_signal() {
	if (this->type == LINEAR_LAYER) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
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
	} else if (this->type == RELU_LAYER) {
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] > 0.0) {
				for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
					int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
					for (int ln_index = 0; ln_index < layer_size; ln_index++) {
						// multiply by this->errors[n_index] for MSE weight updates
						this->weight_updates[n_index][l_index][ln_index] +=
							this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
					}
				}

				this->constant_updates[n_index] += this->errors[n_index];
			}

			this->errors[n_index] = 0.0;
		}
	} else {
		// this->type == LEAKY_LAYER
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] < 0.0) {
				this->errors[n_index] *= 0.01;
			}

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
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
}

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

void Layer::fold_backprop_no_state(int fold_index,
								   int state_size) {
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

		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size()-state_size; l_index++) {
			int layer_size = (int)this->input_layers.back()->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->input_layers.back()->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index].back()[ln_index];
			}
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::fold_backprop_last_state(int fold_index,
									 int state_size) {
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

		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size()-state_size; l_index++) {
			int layer_size = (int)this->input_layers.back()->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->input_layers.back()->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index].back()[ln_index];
			}
		}

		if (state_size > 0) {
			int layer_size = (int)this->input_layers.back()->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->input_layers.back()->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index].back()[ln_index];
			}
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::fold_backprop_last_state_with_no_weight_change() {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		int layer_size = (int)this->input_layers.back()->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			this->input_layers.back()->errors[ln_index] +=
				this->errors[n_index]*this->weights[n_index].back()[ln_index];
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::fold_backprop(int fold_index,
						  int state_size) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->input_layers.back()->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index].back()[ln_index];
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}
		this->constant_updates[n_index] += this->errors[n_index];

		this->errors[n_index] = 0.0;
	}
}

// void Layer::fold_backprop_state(int fold_index,
// 								int state_size) {
// 	// this->type == LEAKY_LAYER
// 	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
// 		if (this->acti_vals[n_index] < 0.0) {
// 			this->errors[n_index] *= 0.01;
// 		}

// 		for (int l_index = fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
// 			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
// 			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
// 				// multiply by this->errors[n_index] for MSE weight updates
// 				this->weight_updates[n_index][l_index][ln_index] +=
// 					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
// 			}
// 		}
// 		this->constant_updates[n_index] += this->errors[n_index];

// 		for (int l_index = (int)this->input_layers.size() - state_size; l_index < (int)this->input_layers.size(); l_index++) {
// 			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
// 			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
// 				this->input_layers[l_index]->errors[ln_index] +=
// 					this->errors[n_index]*this->weights[n_index][l_index][ln_index];
// 			}
// 		}

// 		this->errors[n_index] = 0.0;
// 	}
// }

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

void Layer::subfold_add_s_input(int layer,
								int num_state) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < num_state; s_index++) {
			this->weights[n_index][layer].push_back((randuni()-0.5)*0.02);
			this->weight_updates[n_index][layer].push_back(0.0);
		}
	}
}

void Layer::subfold_activate(int fold_index,
							 int num_layers) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		double sum_val = this->constants[n_index];

		for (int l_index = fold_index+1; l_index < num_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				sum_val += this->input_layers[l_index]->acti_vals[ln_index]
						   *this->weights[n_index][l_index][ln_index];
			}
		}

		for (int l_index = num_layers+fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
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

void Layer::subfold_backprop_new_s_input(int fold_index,
										 int num_layers,
										 int layer,
										 int new_input_size) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		for (int l_index = fold_index+1; l_index < num_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}

		for (int l_index = num_layers+fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}

		this->constant_updates[n_index] += this->errors[n_index];

		int layer_size = (int)this->input_layers[layer]->acti_vals.size();
		for (int ln_index = layer_size-new_input_size; ln_index < layer_size; ln_index++) {
			this->input_layers[layer]->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index][layer][ln_index];
		}

		this->errors[n_index] = 0.0;
	}
}

void Layer::subfold_backprop(int fold_index,
							 int num_layers) {
	// this->type == LEAKY_LAYER
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->acti_vals[n_index] < 0.0) {
			this->errors[n_index] *= 0.01;
		}

		for (int l_index = fold_index+1; l_index < num_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				this->input_layers[l_index]->errors[ln_index] +=
					this->errors[n_index]*this->weights[n_index][l_index][ln_index];
				// multiply by this->errors[n_index] for MSE weight updates
				this->weight_updates[n_index][l_index][ln_index] +=
					this->errors[n_index]*this->input_layers[l_index]->acti_vals[ln_index];
			}
		}

		for (int l_index = num_layers+fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
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
}

void Layer::subfold_get_max_update(int fold_index,
								   int num_layers,
								   double& max_update_size) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = fold_index+1; l_index < num_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update_size = abs(this->weight_updates[n_index][l_index][ln_index]);
				if (update_size > max_update_size) {
					max_update_size = update_size;
				}
			}
		}

		for (int l_index = num_layers+fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
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

void Layer::subfold_update_weights(int fold_index,
								   int num_layers,
								   double learning_rate) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = fold_index+1; l_index < num_layers; l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				double update = this->weight_updates[n_index][l_index][ln_index]
								*learning_rate;
				this->weight_updates[n_index][l_index][ln_index] = 0.0;
				this->weights[n_index][l_index][ln_index] += update;
			}
		}

		for (int l_index = num_layers+fold_index+1; l_index < (int)this->input_layers.size(); l_index++) {
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
