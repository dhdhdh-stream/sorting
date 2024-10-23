#include "action_network.h"

#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.002;

ActionNetwork::ActionNetwork(int num_states,
							 int num_actions) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < num_states; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 10; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	for (int o_index = 0; o_index < num_actions; o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}
	this->output->input_layers.push_back(this->input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

ActionNetwork::ActionNetwork(ifstream& input_file) {
	this->input = new Layer(LINEAR_LAYER);
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	for (int i_index = 0; i_index < hidden_1_size; i_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	for (int i_index = 0; i_index < hidden_2_size; i_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	string output_size_line;
	getline(input_file, output_size_line);
	int output_size = stoi(output_size_line);
	for (int o_index = 0; o_index < output_size; o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}
	this->output->input_layers.push_back(this->input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

ActionNetwork::~ActionNetwork() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void ActionNetwork::activate(vector<double>& state_vals) {
	for (int i_index = 0; i_index < (int)state_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = state_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	double max_val = numeric_limits<double>::lowest();
	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		if (this->output->acti_vals[o_index] > max_val) {
			max_val = this->output->acti_vals[o_index];
		}
	}

	double sum_vals = 0.0;
	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		this->output->acti_vals[o_index] = exp(this->output->acti_vals[o_index] - max_val);
		sum_vals += this->output->acti_vals[o_index];
	}
	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		this->output->acti_vals[o_index] /= sum_vals;
	}
}

void ActionNetwork::activate(vector<double>& state_vals,
							 ActionNetworkHistory* history) {
	for (int i_index = 0; i_index < (int)state_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = state_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	double max_val = numeric_limits<double>::lowest();
	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		if (this->output->acti_vals[o_index] > max_val) {
			max_val = this->output->acti_vals[o_index];
		}
	}

	double sum_vals = 0.0;
	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		this->output->acti_vals[o_index] = exp(this->output->acti_vals[o_index] - max_val);
		sum_vals += this->output->acti_vals[o_index];
	}
	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		this->output->acti_vals[o_index] /= sum_vals;
	}

	history->input_histories = vector<double>(this->input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->input->acti_vals.size(); i_index++) {
		history->input_histories[i_index] = this->input->acti_vals[i_index];
	}
	history->hidden_1_histories = vector<double>(this->hidden_1->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_1->acti_vals.size(); n_index++) {
		history->hidden_1_histories[n_index] = this->hidden_1->acti_vals[n_index];
	}
	history->hidden_2_histories = vector<double>(this->hidden_2->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_2->acti_vals.size(); n_index++) {
		history->hidden_2_histories[n_index] = this->hidden_2->acti_vals[n_index];
	}
	history->output_histories = vector<double>(this->output->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->output->acti_vals.size(); n_index++) {
		history->output_histories[n_index] = this->output->acti_vals[n_index];
	}
}

void ActionNetwork::backprop(int target,
							 vector<double>& state_errors,
							 ActionNetworkHistory* history) {
	for (int i_index = 0; i_index < (int)this->input->acti_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = history->input_histories[i_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_1->acti_vals.size(); n_index++) {
		this->hidden_1->acti_vals[n_index] = history->hidden_1_histories[n_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_2->acti_vals.size(); n_index++) {
		this->hidden_2->acti_vals[n_index] = history->hidden_2_histories[n_index];
	}
	for (int n_index = 0; n_index < (int)this->output->acti_vals.size(); n_index++) {
		this->output->acti_vals[n_index] = history->output_histories[n_index];
	}

	// double cross_entropy_loss = -log(this->output->acti_vals[target]);
	// for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
	// 	if (o_index == target) {
	// 		this->output->errors[o_index] = cross_entropy_loss * (this->output->acti_vals[o_index] - 1.0);
	// 	} else {
	// 		this->output->errors[o_index] = cross_entropy_loss * this->output->acti_vals[o_index];
	// 	}
	// }

	for (int o_index = 0; o_index < (int)this->output->acti_vals.size(); o_index++) {
		if (o_index == target) {
			this->output->errors[o_index] = this->output->acti_vals[o_index] * (1.0 - this->output->acti_vals[o_index]);
		} else {
			this->output->errors[o_index] = -this->output->acti_vals[o_index] * this->output->acti_vals[target];
		}
	}

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	for (int i_index = 0; i_index < (int)state_errors.size(); i_index++) {
		state_errors[i_index] += this->input->errors[i_index];
		this->input->errors[i_index] = 0;
	}
}

void ActionNetwork::update_weights() {
	double hidden_1_max_update = 0.0;
	this->hidden_1->get_max_update(hidden_1_max_update);
	this->hidden_1_average_max_update = 0.999*this->hidden_1_average_max_update+0.001*hidden_1_max_update;
	if (hidden_1_max_update > 0.0) {
		double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_1_average_max_update;
		if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
		}
		this->hidden_1->update_weights(hidden_1_learning_rate);
	}

	double hidden_2_max_update = 0.0;
	this->hidden_2->get_max_update(hidden_2_max_update);
	this->hidden_2_average_max_update = 0.999*this->hidden_2_average_max_update+0.001*hidden_2_max_update;
	if (hidden_2_max_update > 0.0) {
		double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_2_average_max_update;
		if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
		}
		this->hidden_2->update_weights(hidden_2_learning_rate);
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
}

void ActionNetwork::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}
