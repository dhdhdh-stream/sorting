#include "layer.h"

#include <cmath>
#include <iostream>
#include <random>
#include <sstream>

#include "constants.h"
#include "globals.h"

using namespace std;

const double M_CONSTANT = 0.9;
const double V_CONSTANT = 0.999;
const double EPSILON = 0.00000001;

Layer::Layer(int type) {
	this->type = type;

	this->m_bch = 1.0;
	this->v_bch = 1.0;
}

void Layer::update_structure(double multiplier) {
	uniform_real_distribution<double> distribution(-multiplier, multiplier);
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		if ((int)this->weights.size() < n_index+1) {
			this->weights.push_back(vector<Eigen::VectorXf>());
		}
		if ((int)this->constants.size() < n_index+1) {
			this->constants.push_back(0.0);
		}
		if ((int)this->weight_updates.size() < n_index+1) {
			this->weight_updates.push_back(vector<Eigen::VectorXf>());
		}
		if ((int)this->constant_updates.size() < n_index+1) {
			this->constant_updates.push_back(0.0);
		}
		if ((int)this->weight_ms.size() < n_index+1) {
			this->weight_ms.push_back(vector<Eigen::VectorXf>());
		}
		if ((int)this->constant_ms.size() < n_index+1) {
			this->constant_ms.push_back(0.0);
		}
		if ((int)this->weight_vs.size() < n_index+1) {
			this->weight_vs.push_back(vector<Eigen::VectorXf>());
		}
		if ((int)this->constant_vs.size() < n_index+1) {
			this->constant_vs.push_back(0.0);
		}

		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			if ((int)this->weights[n_index].size() < l_index+1) {
				this->weights[n_index].push_back(Eigen::VectorXf());
			}
			if ((int)this->weight_updates[n_index].size() < l_index+1) {
				this->weight_updates[n_index].push_back(Eigen::VectorXf());
			}
			if ((int)this->weight_ms[n_index].size() < l_index+1) {
				this->weight_ms[n_index].push_back(Eigen::VectorXf());
			}
			if ((int)this->weight_vs[n_index].size() < l_index+1) {
				this->weight_vs[n_index].push_back(Eigen::VectorXf());
			}

			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				if ((int)this->weights[n_index][l_index].size() < ln_index+1) {
					this->weights[n_index][l_index].conservativeResize(this->weights[n_index][l_index].size()+1);
					this->weights[n_index][l_index](ln_index) = distribution(generator);
				}
				if ((int)this->weight_updates[n_index][l_index].size() < ln_index+1) {
					this->weight_updates[n_index][l_index].conservativeResize(this->weight_updates[n_index][l_index].size()+1);
					this->weight_updates[n_index][l_index](ln_index) = 0.0;
				}
				if ((int)this->weight_ms[n_index][l_index].size() < ln_index+1) {
					this->weight_ms[n_index][l_index].conservativeResize(this->weight_ms[n_index][l_index].size()+1);
					this->weight_ms[n_index][l_index](ln_index) = 0.0;
				}
				if ((int)this->weight_vs[n_index][l_index].size() < ln_index+1) {
					this->weight_vs[n_index][l_index].conservativeResize(this->weight_vs[n_index][l_index].size()+1);
					this->weight_vs[n_index][l_index](ln_index) = 0.0;
				}
			}
		}
	}
}

void Layer::copy_weights_from(Layer* original) {
	this->weights = original->weights;
	this->constants = original->constants;
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
				this->weights[n_index][l_index](ln_index) = stod(item);
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
				sum_val += this->input_layers[l_index]->acti_vals.dot(
					this->weights[n_index][l_index]);
			}

			this->acti_vals[n_index] = sum_val;
		}

		break;
	case RELU_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			double sum_val = this->constants[n_index];

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				sum_val += this->input_layers[l_index]->acti_vals.dot(
					this->weights[n_index][l_index]);
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
				sum_val += this->input_layers[l_index]->acti_vals.dot(
					this->weights[n_index][l_index]);
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
				sum_val += this->input_layers[l_index]->acti_vals.dot(
					this->weights[n_index][l_index]);
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
				this->input_layers[l_index]->errors += this->errors[n_index]
					* this->weights[n_index][l_index];
				this->weight_updates[n_index][l_index] += this->errors[n_index]
					* this->input_layers[l_index]->acti_vals;
			}

			this->constant_updates[n_index] += this->errors[n_index];

			this->errors[n_index] = 0.0;
		}

		break;
	case RELU_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] > 0.0) {
				for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
					this->input_layers[l_index]->errors += this->errors[n_index]
						* this->weights[n_index][l_index];
					this->weight_updates[n_index][l_index] += this->errors[n_index]
						* this->input_layers[l_index]->acti_vals;
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
				this->input_layers[l_index]->errors += this->errors[n_index]
					* this->weights[n_index][l_index];
				this->weight_updates[n_index][l_index] += this->errors[n_index]
					* this->input_layers[l_index]->acti_vals;
			}

			this->constant_updates[n_index] += this->errors[n_index];

			this->errors[n_index] = 0.0;
		}

		break;
	case SIGMOID_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			this->errors[n_index] *= (this->acti_vals[n_index] * (1.0 - this->acti_vals[n_index]));

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				this->input_layers[l_index]->errors += this->errors[n_index]
					* this->weights[n_index][l_index];
				this->weight_updates[n_index][l_index] += this->errors[n_index]
					* this->input_layers[l_index]->acti_vals;
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
				this->input_layers[l_index]->errors += this->errors[n_index]
					* this->weights[n_index][l_index];
			}

			this->errors[n_index] = 0.0;
		}

		break;
	case RELU_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			if (this->acti_vals[n_index] > 0.0) {
				for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
					this->input_layers[l_index]->errors += this->errors[n_index]
						* this->weights[n_index][l_index];
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
				this->input_layers[l_index]->errors += this->errors[n_index]
					* this->weights[n_index][l_index];
			}

			this->errors[n_index] = 0.0;
		}

		break;
	case SIGMOID_LAYER:
		for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
			this->errors[n_index] *= (this->acti_vals[n_index] * (1.0 - this->acti_vals[n_index]));

			for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
				this->input_layers[l_index]->errors += this->errors[n_index]
					* this->weights[n_index][l_index];
			}

			this->errors[n_index] = 0.0;
		}

		break;
	}
}

void Layer::update(int num_instances,
				   double learning_rate) {
	this->m_bch *= M_CONSTANT;
	this->v_bch *= V_CONSTANT;

	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			this->weight_updates[n_index][l_index] /= num_instances;

			this->weight_ms[n_index][l_index] = M_CONSTANT * this->weight_ms[n_index][l_index]
				+ (1.0 - M_CONSTANT) * this->weight_updates[n_index][l_index];
			this->weight_vs[n_index][l_index] = V_CONSTANT * this->weight_vs[n_index][l_index]
				+ (1.0 - V_CONSTANT) * this->weight_updates[n_index][l_index].cwiseProduct(this->weight_updates[n_index][l_index]);

			this->weights[n_index][l_index] += (learning_rate
				* this->weight_ms[n_index][l_index] / (1.0 - this->m_bch))
				.cwiseQuotient(
					(Eigen::VectorXf)(this->weight_vs[n_index][l_index].cwiseSqrt().array() / (1.0 - this->v_bch) + EPSILON));

			this->weight_updates[n_index][l_index].setConstant(0.0);
		}

		this->constant_updates[n_index] /= num_instances;

		this->constant_ms[n_index] = M_CONSTANT * this->constant_ms[n_index]
			+ (1.0 - M_CONSTANT) * this->constant_updates[n_index];
		this->constant_vs[n_index] = V_CONSTANT * this->constant_vs[n_index]
			+ (1.0 - V_CONSTANT) * this->constant_updates[n_index] * this->constant_updates[n_index];

		this->constants[n_index] += learning_rate
			* this->constant_ms[n_index] / (1.0 - this->m_bch)
			/ (sqrt(this->constant_vs[n_index] / (1.0 - this->v_bch)) + EPSILON);

		this->constant_updates[n_index] = 0.0;
	}
}

void Layer::clear_momentum() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			this->weight_updates[n_index][l_index].setConstant(0.0);
			this->weight_ms[n_index][l_index].setConstant(0.0);
			this->weight_vs[n_index][l_index].setConstant(0.0);
		}

		this->constant_updates[n_index] = 0.0;
		this->constant_ms[n_index] = 0.0;
		this->constant_vs[n_index] = 0.0;
	}

	this->m_bch = 1.0;
	this->v_bch = 1.0;
}

void Layer::save_weights(ofstream& output_file) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		for (int l_index = 0; l_index < (int)this->input_layers.size(); l_index++) {
			int layer_size = (int)this->input_layers[l_index]->acti_vals.size();
			for (int ln_index = 0; ln_index < layer_size; ln_index++) {
				if (abs(this->weights[n_index][l_index](ln_index)) < MIN_WEIGHT) {
					this->weights[n_index][l_index](ln_index) = 0.0;
				}
				output_file << this->weights[n_index][l_index](ln_index) << ",";
			}
			output_file << endl;
		}
		if (abs(this->constants[n_index]) < MIN_WEIGHT) {
			this->constants[n_index] = 0.0;
		}
		output_file << this->constants[n_index] << endl;
	}
}
