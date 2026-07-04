#include "network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const int HIDDEN_1_NUM_STATES_MULTIPLE = 4;
const int HIDDEN_2_NUM_STATES_MULTIPLE = 2;
const int HIDDEN_3_NUM_STATES_MULTIPLE = 1;

Network::Network(vector<int>& input_sizes,
				 int output_size) {
	this->inputs = vector<Layer*>(input_sizes.size());
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->inputs[l_index] = new Layer(LINEAR_LAYER);
		this->inputs[l_index]->acti_vals.resize(input_sizes[l_index]);
		this->inputs[l_index]->errors.resize(input_sizes[l_index]);
		this->inputs[l_index]->errors.setConstant(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(HIDDEN_1_NUM_STATES_MULTIPLE * input_sizes[0]);
	this->hidden_1->errors.resize(HIDDEN_1_NUM_STATES_MULTIPLE * input_sizes[0]);
	this->hidden_1->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_1->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(HIDDEN_2_NUM_STATES_MULTIPLE * input_sizes[0]);
	this->hidden_2->errors.resize(HIDDEN_2_NUM_STATES_MULTIPLE * input_sizes[0]);
	this->hidden_2->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_2->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(HIDDEN_3_NUM_STATES_MULTIPLE * input_sizes[0]);
	this->hidden_3->errors.resize(HIDDEN_3_NUM_STATES_MULTIPLE * input_sizes[0]);
	this->hidden_3->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_3->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(output_size);
	this->output->errors.resize(output_size);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();
}

Network::Network(Network* original) {
	this->inputs = vector<Layer*>(original->inputs.size());
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->inputs[l_index] = new Layer(LINEAR_LAYER);
		this->inputs[l_index]->acti_vals.resize(original->inputs[l_index]->acti_vals.size());
		this->inputs[l_index]->errors.resize(original->inputs[l_index]->errors.size());
		this->inputs[l_index]->errors.setConstant(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_1->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_2->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(original->hidden_3->acti_vals.size());
	this->hidden_3->errors.resize(original->hidden_3->errors.size());
	this->hidden_3->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_3->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();
	this->hidden_3->copy_weights_from(original->hidden_3);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);
}

Network::Network(ifstream& input_file) {
	string input_num_layers_line;
	getline(input_file, input_num_layers_line);
	this->inputs = vector<Layer*>(stoi(input_num_layers_line));
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->inputs[l_index] = new Layer(LINEAR_LAYER);
		string input_size_line;
		getline(input_file, input_size_line);
		int input_size = stoi(input_size_line);
		this->inputs[l_index]->acti_vals.resize(input_size);
		this->inputs[l_index]->errors.resize(input_size);
		this->inputs[l_index]->errors.setConstant(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_1->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_2->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	string hidden_3_size_line;
	getline(input_file, hidden_3_size_line);
	int hidden_3_size = stoi(hidden_3_size_line);
	this->hidden_3->acti_vals.resize(hidden_3_size);
	this->hidden_3->errors.resize(hidden_3_size);
	this->hidden_3->errors.setConstant(0.0);
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		this->hidden_3->input_layers.push_back(this->inputs[l_index]);
	}
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	string output_size_line;
	getline(input_file, output_size_line);
	int output_size = stoi(output_size_line);
	this->output->acti_vals.resize(output_size);
	this->output->errors.resize(output_size);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->hidden_3->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

Network::~Network() {
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		delete this->inputs[l_index];
	}
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void Network::activate() {
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void Network::activate(NetworkHistory* history) {
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();

	history->input_history = vector<vector<double>>(this->inputs.size());
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		history->input_history[l_index] = vector<double>(this->inputs[l_index]->acti_vals.size());
		for (int i_index = 0; i_index < (int)this->inputs[l_index]->acti_vals.size(); i_index++) {
			history->input_history[l_index][i_index] = this->inputs[l_index]->acti_vals(i_index);
		}
	}
	history->hidden_1_history = vector<double>(this->hidden_1->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		history->hidden_1_history[h_index] = this->hidden_1->acti_vals(h_index);
	}
	history->hidden_2_history = vector<double>(this->hidden_2->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		history->hidden_2_history[h_index] = this->hidden_2->acti_vals(h_index);
	}
	history->hidden_3_history = vector<double>(this->hidden_3->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_3->acti_vals.size(); h_index++) {
		history->hidden_3_history[h_index] = this->hidden_3->acti_vals(h_index);
	}
}

void Network::backprop() {
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();
}

void Network::backprop(NetworkHistory* history) {
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		for (int i_index = 0; i_index < (int)this->inputs[l_index]->acti_vals.size(); i_index++) {
			this->inputs[l_index]->acti_vals(i_index) = history->input_history[l_index][i_index];
		}
	}
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		this->hidden_1->acti_vals(h_index) = history->hidden_1_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		this->hidden_2->acti_vals(h_index) = history->hidden_2_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_3->acti_vals.size(); h_index++) {
		this->hidden_3->acti_vals(h_index) = history->hidden_3_history[h_index];
	}

	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();
}

void Network::init_update(double& hidden_1_average_max_update,
						  double& hidden_2_average_max_update,
						  double& hidden_3_average_max_update,
						  double& output_average_max_update) {
	double hidden_1_max_update = 0.0;
	this->hidden_1->get_max_update(hidden_1_max_update);
	hidden_1_average_max_update = 0.999*hidden_1_average_max_update+0.001*hidden_1_max_update;
	if (hidden_1_max_update > 0.0) {
		double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_1_average_max_update;
		if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
		}
		this->hidden_1->update_weights(hidden_1_learning_rate);
	}

	double hidden_2_max_update = 0.0;
	this->hidden_2->get_max_update(hidden_2_max_update);
	hidden_2_average_max_update = 0.999*hidden_2_average_max_update+0.001*hidden_2_max_update;
	if (hidden_2_max_update > 0.0) {
		double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_2_average_max_update;
		if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
		}
		this->hidden_2->update_weights(hidden_2_learning_rate);
	}

	double hidden_3_max_update = 0.0;
	this->hidden_3->get_max_update(hidden_3_max_update);
	hidden_3_average_max_update = 0.999*hidden_3_average_max_update+0.001*hidden_3_max_update;
	if (hidden_3_max_update > 0.0) {
		double hidden_3_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_3_average_max_update;
		if (hidden_3_learning_rate*hidden_3_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_3_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_3_max_update;
		}
		this->hidden_3->update_weights(hidden_3_learning_rate);
	}

	double output_max_update = 0.0;
	this->output->get_max_update(output_max_update);
	output_average_max_update = 0.999*output_average_max_update+0.001*output_max_update;
	if (output_max_update > 0.0) {
		double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/output_average_max_update;
		if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
			output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
		}
		this->output->update_weights(output_learning_rate);
	}
}

void Network::get_max_update(double& max_update) {
	this->hidden_1->get_max_update(max_update);
	this->hidden_2->get_max_update(max_update);
	this->hidden_3->get_max_update(max_update);
	this->output->get_max_update(max_update);
}

void Network::update_weights(double learning_rate) {
	this->hidden_1->update_weights(learning_rate);
	this->hidden_2->update_weights(learning_rate);
	this->hidden_3->update_weights(learning_rate);
	this->output->update_weights(learning_rate);
}

void Network::add_inputs(int new_num_states) {
	this->inputs[0]->acti_vals.resize(new_num_states);
	this->inputs[0]->errors.resize(new_num_states);
	this->inputs[0]->errors.setConstant(0.0);

	this->hidden_1->acti_vals.resize(HIDDEN_1_NUM_STATES_MULTIPLE * new_num_states);
	this->hidden_1->errors.resize(HIDDEN_1_NUM_STATES_MULTIPLE * new_num_states);
	this->hidden_1->errors.setConstant(0.0);

	this->hidden_2->acti_vals.resize(HIDDEN_2_NUM_STATES_MULTIPLE * new_num_states);
	this->hidden_2->errors.resize(HIDDEN_2_NUM_STATES_MULTIPLE * new_num_states);
	this->hidden_2->errors.setConstant(0.0);

	this->hidden_3->acti_vals.resize(HIDDEN_3_NUM_STATES_MULTIPLE * new_num_states);
	this->hidden_3->errors.resize(HIDDEN_3_NUM_STATES_MULTIPLE * new_num_states);
	this->hidden_3->errors.setConstant(0.0);

	this->hidden_1->update_structure();
	this->hidden_2->update_structure();
	this->hidden_3->update_structure();
	this->output->update_structure();
}

void Network::add_outputs(int new_num_states) {
	this->output->acti_vals.resize(new_num_states);
	this->output->errors.resize(new_num_states);
	this->output->errors.setConstant(0.0);
}

void Network::save(ofstream& output_file) {
	output_file << this->inputs.size() << endl;
	for (int l_index = 0; l_index < (int)this->inputs.size(); l_index++) {
		output_file << this->inputs[l_index]->acti_vals.size() << endl;
	}
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->hidden_3->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}

NetworkHistory::NetworkHistory(Network* network) {
	this->network = network;
}
