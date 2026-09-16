#include "obs_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

ObsNetwork::ObsNetwork(int num_states,
					   int num_obs) {
	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->obs_input = new Layer(LINEAR_LAYER);
	this->obs_input->acti_vals.resize(num_obs);
	this->obs_input->errors.resize(num_obs);
	this->obs_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(16);
	this->hidden_1->errors.resize(16);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(8);
	this->hidden_2->errors.resize(8);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(num_states);
	this->output->errors.resize(num_states);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(0.0);
	/**
	 * - don't directly connect input to output
	 *   - update size will be large even when network has no impact
	 *   - hidden updates will be small
	 *   - and/or less robust to change?
	 *     - noise gets directly connected to error
	 *       - destroying previous signal
	 *   - whereas with indirect, noise goes through hidden and is weakened
	 */

	this->num_instances = 0;
	this->last_update_iter = -1;
	this->epoch_iter = 0;
}

ObsNetwork::ObsNetwork(ObsNetwork* original) {
	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(original->state_input->acti_vals.size());
	this->state_input->errors.resize(original->state_input->errors.size());
	this->state_input->errors.setConstant(0.0);

	this->obs_input = new Layer(LINEAR_LAYER);
	this->obs_input->acti_vals.resize(original->obs_input->acti_vals.size());
	this->obs_input->errors.resize(original->obs_input->errors.size());
	this->obs_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(0.0);
	this->output->copy_weights_from(original->output);

	this->num_instances = 0;
	this->last_update_iter = -1;
	this->epoch_iter = 0;
}

ObsNetwork::ObsNetwork(ifstream& input_file) {
	this->state_input = new Layer(LINEAR_LAYER);
	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->obs_input = new Layer(LINEAR_LAYER);
	string num_obs_line;
	getline(input_file, num_obs_line);
	int num_obs = stoi(num_obs_line);
	this->obs_input->acti_vals.resize(num_obs);
	this->obs_input->errors.resize(num_obs);
	this->obs_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(num_states);
	this->output->errors.resize(num_states);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(0.0);

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->num_instances = 0;
	this->last_update_iter = -1;
	this->epoch_iter = 0;
}

ObsNetwork::~ObsNetwork() {
	delete this->state_input;
	delete this->obs_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void ObsNetwork::activate(Eigen::VectorXf& state_vals,
						  vector<double>& obs_input_vals) {
	this->state_input->acti_vals = state_vals;

	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		this->obs_input->acti_vals(i_index) = obs_input_vals[i_index];
	}

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	state_vals += this->output->acti_vals;
}

void ObsNetwork::save(ObsNetworkHistory* history) {
	history->state_input_history = this->state_input->acti_vals;
	history->obs_input_history = this->obs_input->acti_vals;
	history->hidden_1_history = this->hidden_1->acti_vals;
	history->hidden_2_history = this->hidden_2->acti_vals;
}

void ObsNetwork::load(ObsNetworkHistory* history) {
	this->state_input->acti_vals = history->state_input_history;
	this->obs_input->acti_vals = history->obs_input_history;
	this->hidden_1->acti_vals = history->hidden_1_history;
	this->hidden_2->acti_vals = history->hidden_2_history;
}

void ObsNetwork::backprop(Eigen::VectorXf& state_errors) {
	this->output->errors = state_errors;

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	state_errors += this->state_input->errors;
	this->state_input->errors.setConstant(0.0);

	this->num_instances++;
}

void ObsNetwork::update() {
	this->epoch_iter++;
	if (this->epoch_iter == UPDATE_EPOCH_SIZE) {
		this->hidden_1->update(this->num_instances,
							   STATE_LEARNING_RATE);
		this->hidden_2->update(this->num_instances,
							   STATE_LEARNING_RATE);
		this->output->update(this->num_instances,
							 STATE_LEARNING_RATE);

		this->num_instances = 0;
		this->epoch_iter = 0;
	}
}

void ObsNetwork::clear_momentum() {
	this->hidden_1->clear_momentum();
	this->hidden_2->clear_momentum();
	this->output->clear_momentum();

	this->num_instances = 0;
	this->epoch_iter = 0;
}

void ObsNetwork::add_states(int new_num_states) {
	this->state_input->acti_vals.resize(new_num_states);
	this->state_input->errors.resize(new_num_states);
	this->state_input->errors.setConstant(0.0);

	this->output->acti_vals.resize(new_num_states);
	this->output->errors.resize(new_num_states);
	this->output->errors.setConstant(0.0);

	this->hidden_1->update_structure(0.0);
	this->hidden_2->update_structure(0.0);
	this->output->update_structure(0.0);
}

void ObsNetwork::save(ofstream& output_file) {
	output_file << this->state_input->acti_vals.size() << endl;

	output_file << this->obs_input->acti_vals.size() << endl;

	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}
