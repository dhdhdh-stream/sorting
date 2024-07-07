#include "cnn.h"

#include "globals.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

CNN::CNN() {
	this->conv_inputs = vector<vector<double>>(16);
	for (int c_index = 0; c_index < 16; c_index++) {
		this->conv_inputs[c_index] = vector<double>(25);
	}
	this->weights = vector<vector<double>>(4);
	uniform_real_distribution<double> distribution(-0.01, 0.01);
	for (int o_index = 0; o_index < 4; o_index++) {
		this->weights[o_index] = vector<double>(25);
		for (int i_index = 0; i_index < 25; i_index++) {
			this->weights[o_index][i_index] = distribution(generator);
		}
	}
	this->constants = vector<double>(4, 0.0);
	this->weight_updates = vector<vector<double>>(4);
	for (int o_index = 0; o_index < 4; o_index++) {
		this->weight_updates[o_index] = vector<double>(25, 0.0);
	}
	this->constant_updates = vector<double>(4, 0.0);

	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < 64; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < 10; n_index++) {
		this->hidden->acti_vals.push_back(0.0);
		this->hidden->errors.push_back(0.0);
	}
	this->hidden->input_layers.push_back(this->input);
	this->hidden->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->conv_average_max_update = 0.0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

CNN::~CNN() {
	delete this->input;
	delete this->hidden;
	delete this->output;
}

void CNN::activate(vector<vector<double>>& input_vals) {
	for (int cx_index = 0; cx_index < 4; cx_index++) {
		for (int cy_index = 0; cy_index < 4; cy_index++) {
			int conv_index = 4*cx_index+cy_index;

			for (int x_index = 0; x_index < 5; x_index++) {
				for (int y_index = 0; y_index < 5; y_index++) {
					this->conv_inputs[conv_index][5*x_index+y_index]
						= input_vals[2*cx_index + x_index][2*cy_index + y_index];
				}
			}

			for (int n_index = 0; n_index < 4; n_index++) {
				double sum_val = this->constants[n_index];

				for (int ln_index = 0; ln_index < 25; ln_index++) {
					sum_val += this->conv_inputs[conv_index][ln_index]
						* this->weights[n_index][ln_index];
				}

				if (sum_val > 0.0) {
					this->input->acti_vals[4*conv_index + n_index] = sum_val;
				} else {
					this->input->acti_vals[4*conv_index + n_index] = 0.01*sum_val;
				}
			}
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void CNN::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();
	this->hidden->backprop();
	for (int conv_index = 0; conv_index < 16; conv_index++) {
		for (int n_index = 0; n_index < 4; n_index++) {
			double error = this->input->errors[4*conv_index + n_index];
			if (error < 0.0) {
				error *= 0.01;
			}
			this->input->errors[4*conv_index + n_index] = 0.0;

			for (int ln_index = 0; ln_index < 25; ln_index++) {
				this->weight_updates[n_index][ln_index] +=
					error*this->conv_inputs[conv_index][ln_index];
			}

			this->constant_updates[n_index] += error;
		}
	}

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
				output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}
