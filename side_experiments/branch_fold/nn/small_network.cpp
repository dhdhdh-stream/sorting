#include "small_network.h"

#include <iostream>

using namespace std;

void SmallNetwork::construct() {
	this->state_input = new Layer(LINEAR_LAYER, this->state_input_size);
	this->s_input_input = new Layer(LINEAR_LAYER, this->s_input_input_size);
	
	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->input_layers.push_back(this->s_input_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->output_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

SmallNetwork::SmallNetwork(int state_input_size,
						   int s_input_input_size,
						   int hidden_size,
						   int output_size) {
	this->state_input_size = state_input_size;
	this->s_input_input_size = s_input_input_size;
	this->hidden_size = hidden_size;
	this->output_size = output_size;

	construct();
}

SmallNetwork::SmallNetwork(SmallNetwork* original) {
	this->state_input_size = original->state_input_size;
	this->s_input_input_size = original->s_input_input_size;
	this->hidden_size = original->hidden_size;
	this->output_size = original->output_size;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

SmallNetwork::SmallNetwork(ifstream& input_file) {
	string state_input_size_line;
	getline(input_file, state_input_size_line);
	this->state_input_size = stoi(state_input_size_line);

	string s_input_input_size_line;
	getline(input_file, s_input_input_size_line);
	this->s_input_input_size = stoi(s_input_input_size_line);

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	string output_size_line;
	getline(input_file, output_size_line);
	this->output_size = stoi(output_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

SmallNetwork::~SmallNetwork() {
	delete this->state_input;
	delete this->s_input_input;

	delete this->hidden;
	delete this->output;
}

void SmallNetwork::activate(vector<double>& state_vals,
							vector<double>& s_input_vals) {
	for (int i = 0; i < this->state_input_size; i++) {
		this->state_input->acti_vals[i] = state_vals[i];
	}
	for (int i = 0; i < this->s_input_input_size; i++) {
		this->s_input_input->acti_vals[i] = s_input_vals[i];
	}

	this->hidden->activate();
	this->output->activate();
}

void SmallNetwork::backprop(vector<double>& errors,
							double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void SmallNetwork::backprop_errors_with_no_weight_change(vector<double>& errors) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();
}

void SmallNetwork::backprop_weights_with_no_error_signal(
		vector<double>& errors,
		double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void SmallNetwork::save(ofstream& output_file) {
	output_file << this->state_input_size << endl;
	output_file << this->s_input_input_size << endl;
	output_file << this->hidden_size << endl;
	output_file << this->output_size << endl;
	
	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
