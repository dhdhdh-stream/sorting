#include "layer.h"

#include <cmath>
#include <iostream>
#include <random>
#include <sstream>

#include "constants.h"
#include "globals.h"

using namespace std;

Layer::Layer(int type) {
	this->type = type;
}

void Layer::update_structure() {
	uniform_real_distribution<double> distribution(-0.01, 0.01);

	if (this->acti_vals.size() > this->weights.size()) {
		while (this->weights.size() < this->acti_vals.size()) {
			this->weights.insert(this->weights.begin(), vector<vector<double>>());
		}
		while (this->constants.size() < this->acti_vals.size()) {
			this->constants.insert(this->constants.begin(), 0.0);
		}
		while (this->weight_updates.size() < this->acti_vals.size()) {
			this->weight_updates.insert(this->weight_updates.begin(), vector<vector<double>>());
		}
		while (this->constant_updates.size() < this->acti_vals.size()) {
			this->constant_updates.insert(this->constant_updates.begin(), 0.0);
		}
	} else if (this->acti_vals.size() < this->weights.size()) {
		int num_remove = this->weights.size() - this->acti_vals.size();
		this->weights.erase(this->weights.begin(), this->weights.begin() + num_remove);
		this->constants.erase(this->constants.begin(), this->constants.begin() + num_remove);
		this->weight_updates.erase(this->weight_updates.begin(), this->weight_updates.begin() + num_remove);
		this->constant_updates.erase(this->constant_updates.begin(), this->constant_updates.begin() + num_remove);
	}

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if (this->input_layers.size() > this->weights[n_index].size()) {
			while (this->weights[n_index].size() < this->input_layers.size()) {
				this->weights[n_index].insert(this->weights[n_index].begin(), vector<double>());
			}
			while (this->weight_updates[n_index].size() < this->input_layers.size()) {
				this->weight_updates[n_index].insert(this->weight_updates[n_index].begin(), vector<double>());
			}
		} else if (this->input_layers.size() < this->weights[n_index].size()) {
			int num_remove = this->weights[n_index].size() - this->input_layers.size();
			this->weights[n_index].erase(this->weights[n_index].begin(), this->weights[n_index].begin() + num_remove);
			this->weight_updates[n_index].erase(this->weight_updates[n_index].begin(), this->weight_updates[n_index].begin() + num_remove);
		}
	}

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			if (layer_size > (int)this->weights[n_index][l_index].size()) {
				while ((int)this->weights[n_index][l_index].size() < layer_size) {
					this->weights[n_index][l_index].insert(this->weights[n_index][l_index].begin(), distribution(generator));
				}
				while ((int)this->weight_updates[n_index][l_index].size() < layer_size) {
					this->weight_updates[n_index][l_index].insert(this->weight_updates[n_index][l_index].begin(), 0.0);
				}
			} else if (layer_size < (int)this->weights[n_index][l_index].size()) {
				int num_remove = (int)this->weights[n_index][l_index].size() - layer_size;
				this->weights[n_index][l_index].erase(this->weights[n_index][l_index].begin(), this->weights[n_index][l_index].begin() + num_remove);
				this->weight_updates[n_index][l_index].erase(this->weight_updates[n_index][l_index].begin(), this->weight_updates[n_index][l_index].begin() + num_remove);
			}
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

void Layer::backprop_through() {
	switch (this->type) {
	case LINEAR_LAYER:
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

		break;
	case RELU_LAYER:
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
				}
			}

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
				}
			}

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

void Layer::remove_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].erase(this->weights[n_index][0].begin() + index);
		this->weight_updates[n_index][0].erase(this->weight_updates[n_index][0].begin() + index);
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
