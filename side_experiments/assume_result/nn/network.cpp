#include "network.h"

#include <iostream>

#include "globals.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

Network::Network(int analyze_size) {
	this->input = new Layer(LINEAR_LAYER);
	int input_size = (2*analyze_size + 1) * (2*analyze_size + 1);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden = new Layer(LEAKY_LAYER);
	int hidden_size;
	if (analyze_size == 0) {
		hidden_size = 2;
	} else if (analyze_size == 1) {
		hidden_size = 4;
	} else {
		hidden_size = 10;
	}
	for (int h_index = 0; h_index < hidden_size; h_index++) {
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
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::Network(Network* original) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < (int)original->input->acti_vals.size(); i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden = new Layer(LEAKY_LAYER);
	for (int i_index = 0; i_index < (int)original->hidden->acti_vals.size(); i_index++) {
		this->hidden->acti_vals.push_back(0.0);
		this->hidden->errors.push_back(0.0);
	}
	this->hidden->input_layers.push_back(this->input);
	this->hidden->update_structure();
	this->hidden->copy_weights_from(original->hidden);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::Network(ifstream& input_file) {
	this->input = new Layer(LINEAR_LAYER);
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden = new Layer(LEAKY_LAYER);
	string hidden_size_line;
	getline(input_file, hidden_size_line);
	int hidden_size = stoi(hidden_size_line);
	for (int i_index = 0; i_index < hidden_size; i_index++) {
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

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::~Network() {
	delete this->input;
	delete this->hidden;
	delete this->output;
}

void Network::activate(vector<vector<double>>& input_vals) {
	for (int x_index = 0; x_index < (int)input_vals.size(); x_index++) {
		for (int y_index = 0; y_index < (int)input_vals[x_index].size(); y_index++) {
			this->input->acti_vals[x_index * (int)input_vals[x_index].size() + y_index] = input_vals[x_index][y_index];
		}
	}
	this->hidden->activate();
	this->output->activate();
}

void Network::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();
	this->hidden->backprop();

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

void Network::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden->acti_vals.size() << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
