#include "softmax_layer.h"

#include <sstream>

#include "constants.h"
#include "globals.h"
#include "layer.h"

using namespace std;

SoftmaxLayer::SoftmaxLayer(int size) {
	this->acti_vals = vector<double>(size);
}

void SoftmaxLayer::update_structure() {
	uniform_real_distribution<double> distribution(-0.01, 0.01);
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if ((int)this->weights.size() < n_index+1) {
			this->weights.push_back(vector<vector<double>>());
		}
		if ((int)this->constants.size() < n_index+1) {
			this->constants.push_back(0.0);
		}
		if ((int)this->weight_updates.size() < n_index+1) {
			this->weight_updates.push_back(vector<vector<double>>());
		}
		if ((int)this->constant_updates.size() < n_index+1) {
			this->constant_updates.push_back(0.0);
		}

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			if ((int)this->weights[n_index].size() < l_index+1) {
				this->weights[n_index].push_back(vector<double>());
			}
			if ((int)this->weight_updates[n_index].size() < l_index+1) {
				this->weight_updates[n_index].push_back(vector<double>());
			}

			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				if ((int)this->weights[n_index][l_index].size() < ln_index+1) {
					this->weights[n_index][l_index].push_back(distribution(generator));
				}
				if ((int)this->weight_updates[n_index][l_index].size() < ln_index+1) {
					this->weight_updates[n_index][l_index].push_back(0.0);
				}
			}
		}
	}
}

void SoftmaxLayer::load_weights_from(ifstream& input_file) {
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

void SoftmaxLayer::activate() {
	double sum_vals = 0.0;
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		double sum_val = this->constants[n_index];

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				sum_val += this->input_layers[l_index]->acti_vals[ln_index]
						   *this->weights[n_index][l_index][ln_index];
			}
		}

		this->acti_vals[n_index] = exp(sum_val);
		sum_vals += this->acti_vals[n_index];
	}

	uniform_real_distribution<double> index_distribution(0.0, sum_vals);
	double random_val = index_distribution(generator);
	int curr_index = 0;
	while (true) {
		random_val -= this->acti_vals[curr_index];
		if (random_val <= 0.0) {
			break;
		} else {
			curr_index++;
		}
	}
	this->predicted_index = curr_index;
}

void SoftmaxLayer::activate(int action) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		double sum_val = this->constants[n_index];

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				sum_val += this->input_layers[l_index]->acti_vals[ln_index]
						   *this->weights[n_index][l_index][ln_index];
			}
		}

		this->acti_vals[n_index] = exp(sum_val);
	}

	this->predicted_index = action;
}

void SoftmaxLayer::backprop(bool is_better) {
	double error;
	if (is_better) {
		error = this->acti_vals[this->predicted_index] * (1.0 - this->acti_vals[this->predicted_index]);
	} else {
		error = 0.0;
		for (int i_index = 0; i_index < (int)this->acti_vals.size(); i_index++) {
			if (i_index != this->predicted_index) {
				error += -this->acti_vals[this->predicted_index] * this->acti_vals[i_index];
			}
		}
		error /= ((int)this->acti_vals.size() - 1);
	}

	for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
		int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
		for (int ln_index = 0; ln_index < layer_size; ln_index++) {
			this->input_layers[l_index]->errors[ln_index] +=
				error*this->weights[this->predicted_index][l_index][ln_index];
			this->weight_updates[this->predicted_index][l_index][ln_index] +=
				error*this->input_layers[l_index]->acti_vals[ln_index];
		}
	}

	this->constant_updates[this->predicted_index] += error;
}

void SoftmaxLayer::get_max_update(double& max_update_size) {
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

void SoftmaxLayer::update_weights(double learning_rate) {
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

void SoftmaxLayer::save_weights(ofstream& output_file) {
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
