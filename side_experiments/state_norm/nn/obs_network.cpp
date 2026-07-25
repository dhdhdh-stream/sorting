#include "obs_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

ObsNetwork::ObsNetwork(int num_states,
					   int num_obs) {
	this->type = NETWORK_TYPE_OBS;

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
	 */

	this->run_num_instances = 0;
	this->last_get_max_update_iter = -1;
	this->last_update_weights_iter = -1;
}

ObsNetwork::ObsNetwork(ObsNetwork* original) {
	this->type = NETWORK_TYPE_OBS;

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

	this->run_num_instances = 0;
	this->last_get_max_update_iter = -1;
	this->last_update_weights_iter = -1;
}

ObsNetwork::ObsNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_OBS;

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

	this->run_num_instances = 0;
	this->last_get_max_update_iter = -1;
	this->last_update_weights_iter = -1;
}

ObsNetwork::~ObsNetwork() {
	delete this->state_input;
	delete this->obs_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void ObsNetwork::activate(Eigen::VectorXf& state_norms,
						  Eigen::VectorXf& state_vals,
						  vector<double>& obs_input_vals) {
	this->state_norms = state_norms;

	this->state_input->acti_vals = state_vals.cwiseQuotient(state_norms);

	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		this->obs_input->acti_vals(i_index) = obs_input_vals[i_index];
	}

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	state_vals += this->output->acti_vals;
}

void ObsNetwork::activate_w_drop(Eigen::VectorXf& state_norms,
								 Eigen::VectorXf& state_vals,
								 vector<double>& obs_input_vals) {
	this->state_norms = state_norms;

	this->state_input->acti_vals = state_vals.cwiseQuotient(state_norms);

	uniform_int_distribution<int> drop_distribution(0, 19);
	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		if (drop_distribution(generator) == 0) {
			this->obs_input->acti_vals(i_index) = 0.0;
		} else {
			this->obs_input->acti_vals(i_index) = obs_input_vals[i_index];
		}
	}

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	state_vals += this->output->acti_vals;
}

void ObsNetwork::save(ObsNetworkHistory* history) {
	history->state_norms_history = this->state_norms;
	history->state_input_history = this->state_input->acti_vals;
	history->obs_input_history = this->obs_input->acti_vals;
	history->hidden_1_history = this->hidden_1->acti_vals;
	history->hidden_2_history = this->hidden_2->acti_vals;
	history->output_history = this->output->acti_vals;
}

void ObsNetwork::load(ObsNetworkHistory* history) {
	this->state_norms = history->state_norms_history;
	this->state_input->acti_vals = history->state_input_history;
	this->obs_input->acti_vals = history->obs_input_history;
	this->hidden_1->acti_vals = history->hidden_1_history;
	this->hidden_2->acti_vals = history->hidden_2_history;
	this->output->acti_vals = history->output_history;
}

void ObsNetwork::backprop(Eigen::VectorXf& state_errors) {
	this->output->errors = state_errors;

	this->output->errors -= this->output->acti_vals * STATE_NORM_CONSTANT;
	/**
	 * - don't norm overall state
	 *   - can lead to thrashing/instability
	 * - only norm self
	 */

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	state_errors += this->state_input->errors.cwiseQuotient(this->state_norms);
	this->state_input->errors.setConstant(0.0);

	this->run_num_instances++;
}

void ObsNetwork::get_max_update(double& max_update_size) {
	this->hidden_1->get_max_update(this->run_num_instances,
								   max_update_size);
	this->hidden_2->get_max_update(this->run_num_instances,
								   max_update_size);
	this->output->get_max_update(this->run_num_instances,
								 max_update_size);

	this->run_num_instances = 0;
}

void ObsNetwork::update_weights(double learning_rate) {
	this->hidden_1->update_weights(learning_rate);
	this->hidden_2->update_weights(learning_rate);
	this->output->update_weights(learning_rate);
}

void ObsNetwork::clear_update_weights() {
	this->hidden_1->clear_update_weights();
	this->hidden_2->clear_update_weights();
	this->output->clear_update_weights();
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

ObsNetworkHistory::ObsNetworkHistory(ObsNetwork* network) {
	this->network = network;
}
