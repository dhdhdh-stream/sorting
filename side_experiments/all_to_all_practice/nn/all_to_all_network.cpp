#include "all_to_all_network.h"

#include <iostream>

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

AllToAllNetwork::AllToAllNetwork() {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < 20; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}
	this->is_output_input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < 20; i_index++) {
		this->is_output_input->acti_vals.push_back(0.0);
		this->is_output_input->errors.push_back(0.0);
	}

	this->input_to_hidden = new Layer(LEAKY_LAYER);
	this->input_to_hidden->input_layers.push_back(this->input);
	this->input_to_hidden->input_layers.push_back(this->is_output_input);
	for (int n_index = 0; n_index < 40; n_index++) {
		this->input_to_hidden->acti_vals.push_back(0.0);
		this->input_to_hidden->errors.push_back(0.0);
	}
	this->input_to_hidden->update_structure();

	this->hidden = new Layer(LINEAR_LAYER);
	this->hidden->input_layers.push_back(this->input_to_hidden);
	for (int n_index = 0; n_index < 10; n_index++) {
		this->hidden->acti_vals.push_back(0.0);
		this->hidden->errors.push_back(0.0);
	}
	this->hidden->update_structure();

	this->hidden_to_output = new Layer(LEAKY_LAYER);
	this->hidden_to_output->input_layers.push_back(this->hidden);
	for (int n_index = 0; n_index < 40; n_index++) {
		this->hidden_to_output->acti_vals.push_back(0.0);
		this->hidden_to_output->errors.push_back(0.0);
	}
	this->hidden_to_output->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->input_layers.push_back(this->hidden_to_output);
	for (int o_index = 0; o_index < 20; o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}
	this->output->update_structure();

	this->output_average_errors = vector<double>(20, 1.0);

	this->epoch_iter = 0;
	this->input_to_hidden_average_max_update = 0.0;
	this->hidden_average_max_update = 0.0;
	this->hidden_to_output_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

AllToAllNetwork::~AllToAllNetwork() {
	delete this->input;
	delete this->is_output_input;
	delete this->input_to_hidden;
	delete this->hidden;
	delete this->hidden_to_output;
	delete this->output;
}

void AllToAllNetwork::activate(vector<double>& input_vals,
							   vector<bool>& is_output) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	for (int i_index = 0; i_index < (int)is_output.size(); i_index++) {
		if (is_output[i_index]) {
			this->is_output_input->acti_vals[i_index] = 1.0;
		} else {
			this->is_output_input->acti_vals[i_index] = -1.0;
		}
	}
	this->input_to_hidden->activate();
	this->hidden->activate();
	this->hidden_to_output->activate();
	this->output->activate();
}

void AllToAllNetwork::backprop(vector<double>& errors,
							   vector<bool>& is_output) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		if (is_output[e_index]) {
			this->output->errors[e_index] = errors[e_index] / this->output_average_errors[e_index];

			this->output_average_errors[e_index] = 0.99*this->output_average_errors[e_index] + 0.01*abs(errors[e_index]);
		} else {
			this->output->errors[e_index] = 0.0;
		}
	}
	this->output->backprop();
	this->hidden_to_output->backprop();
	this->hidden->backprop();
	this->input_to_hidden->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double input_to_hidden_max_update = 0.0;
		this->input_to_hidden->get_max_update(input_to_hidden_max_update);
		this->input_to_hidden_average_max_update = 0.999*this->input_to_hidden_average_max_update+0.001*input_to_hidden_max_update;
		if (input_to_hidden_max_update > 0.0) {
			double input_to_hidden_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->input_to_hidden_average_max_update;
			if (input_to_hidden_learning_rate*input_to_hidden_max_update > NETWORK_TARGET_MAX_UPDATE) {
				input_to_hidden_learning_rate = NETWORK_TARGET_MAX_UPDATE/input_to_hidden_max_update;
			}
			this->input_to_hidden->update_weights(input_to_hidden_learning_rate);
		}

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

		double hidden_to_output_max_update = 0.0;
		this->hidden_to_output->get_max_update(hidden_to_output_max_update);
		this->hidden_to_output_average_max_update = 0.999*this->hidden_to_output_average_max_update+0.001*hidden_to_output_max_update;
		if (hidden_to_output_max_update > 0.0) {
			double hidden_to_output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_to_output_average_max_update;
			if (hidden_to_output_learning_rate*hidden_to_output_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_to_output_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_to_output_max_update;
			}
			this->hidden_to_output->update_weights(hidden_to_output_learning_rate);
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