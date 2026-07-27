#include "network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const int EPOCH_SIZE = 100;
/**
 * - not meaningful to update weights more often(?)
 */

Network::Network(int input_size) {
	this->input = new Layer(LINEAR_LAYER);
	this->input->acti_vals.resize(input_size);
	this->input->errors.resize(input_size);
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(16);
	this->hidden_1->errors.resize(16);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(8);
	this->hidden_2->errors.resize(8);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(4);
	this->hidden_3->errors.resize(4);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	/**
	 * - if directly connect input with no norm, can dominate hidden
	 *   - and/or less robust to change?
	 *     - noise gets directly connected to error
	 *       - destroying previous signal
	 *   - whereas with indirect, noise goes through hidden and is weakened
	 */
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->num_instances = 0;
	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

Network::Network(Network* original) {
	this->input = new Layer(LINEAR_LAYER);
	this->input->acti_vals.resize(original->input->acti_vals.size());
	this->input->errors.resize(original->input->errors.size());
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(original->hidden_3->acti_vals.size());
	this->hidden_3->errors.resize(original->hidden_3->errors.size());
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
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

	this->num_instances = 0;
	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

Network::Network(ifstream& input_file) {
	this->input = new Layer(LINEAR_LAYER);
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	this->input->acti_vals.resize(input_size);
	this->input->errors.resize(input_size);
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	string hidden_3_size_line;
	getline(input_file, hidden_3_size_line);
	int hidden_3_size = stoi(hidden_3_size_line);
	this->hidden_3->acti_vals.resize(hidden_3_size);
	this->hidden_3->errors.resize(hidden_3_size);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->hidden_3->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->num_instances = 0;
	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

Network::~Network() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void Network::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals(i_index) = input_vals[i_index];
	}

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void Network::init_backprop(double error,
							double& hidden_1_average_max_update,
							double& hidden_2_average_max_update,
							double& hidden_3_average_max_update,
							double& output_average_max_update) {
	this->output->errors(0) = error;
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		this->hidden_1->update(1);
		this->hidden_2->update(1);
		this->hidden_3->update(1);
		this->output->update(1);

		this->epoch_iter = 0;
	}
}

void Network::backprop(double error) {
	this->output->errors(0) = error;
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->num_instances++;
}

void Network::update() {
	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		this->hidden_1->update(this->num_instances);
		this->hidden_2->update(this->num_instances);
		this->hidden_3->update(this->num_instances);
		this->output->update(this->num_instances);

		this->num_instances = 0;
		this->epoch_iter = 0;
	}
}

void Network::clear_momentum() {
	this->hidden_1->clear_momentum();
	this->hidden_2->clear_momentum();
	this->hidden_3->clear_momentum();
	this->output->clear_momentum();
}

void Network::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->hidden_3->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}
