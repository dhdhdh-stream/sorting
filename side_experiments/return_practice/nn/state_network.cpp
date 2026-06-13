#include "state_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

StateNetwork::StateNetwork(int input_size,
						   int output_size) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 8; h_index++) {
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
	for (int o_index = 0; o_index < output_size; o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();
}

StateNetwork::StateNetwork(StateNetwork* original) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < (int)original->input->acti_vals.size(); i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int i_index = 0; i_index < (int)original->hidden_1->acti_vals.size(); i_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int i_index = 0; i_index < (int)original->hidden_2->acti_vals.size(); i_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->output = new Layer(LINEAR_LAYER);
	for (int o_index = 0; o_index < (int)original->output->acti_vals.size(); o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);
}

StateNetwork::StateNetwork(ifstream& input_file) {
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
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

StateNetwork::~StateNetwork() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void StateNetwork::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();
}

void StateNetwork::backprop(vector<double>& errors) {
	for (int o_index = 0; o_index < (int)errors.size(); o_index++) {
		this->output->errors[o_index] = errors[o_index];
	}
	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();
}

void StateNetwork::activate(vector<double>& input_vals,
							StateNetworkHistory* history) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	for (int i_index = 0; i_index < (int)this->input->acti_vals.size(); i_index++) {
		history->input_history.push_back(this->input->acti_vals[i_index]);
	}
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		history->hidden_1_history.push_back(this->hidden_1->acti_vals[h_index]);
	}
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		history->hidden_2_history.push_back(this->hidden_2->acti_vals[h_index]);
	}
}

void StateNetwork::backprop(vector<double>& errors,
							StateNetworkHistory* history) {
	for (int i_index = 0; i_index < (int)this->input->acti_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = history->input_history[i_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		this->hidden_1->acti_vals[h_index] = history->hidden_1_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		this->hidden_2->acti_vals[h_index] = history->hidden_2_history[h_index];
	}

	for (int o_index = 0; o_index < (int)errors.size(); o_index++) {
		this->output->errors[o_index] = errors[o_index];
	}
	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();
}

void StateNetwork::get_max_update(double& max_update) {
	this->hidden_1->get_max_update(max_update);
	this->hidden_2->get_max_update(max_update);
	this->output->get_max_update(max_update);
}

void StateNetwork::update_weights(double learning_rate) {
	this->hidden_1->update_weights(learning_rate);
	this->hidden_2->update_weights(learning_rate);
	this->output->update_weights(learning_rate);
}

void StateNetwork::add_inputs(int num_add) {
	for (int i_index = 0; i_index < num_add; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1->update_structure();
	this->hidden_2->update_structure();
	this->output->update_structure();
}

void StateNetwork::add_outputs(int num_add) {
	for (int o_index = 0; o_index < num_add; o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}

	this->hidden_1->update_structure();
	this->hidden_2->update_structure();
	this->output->update_structure();
}

void StateNetwork::resize(int num_states) {
	while ((int)this->hidden_1->acti_vals.size() < 2 * num_states) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}

	while ((int)this->hidden_2->acti_vals.size() < num_states) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}

	this->hidden_1->update_structure();
	this->hidden_2->update_structure();
	this->output->update_structure();
}

void StateNetwork::twiddle() {
	this->hidden_1->twiddle();
	this->hidden_2->twiddle();
	this->output->twiddle();
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}
