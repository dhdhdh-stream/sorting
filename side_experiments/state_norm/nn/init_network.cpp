#include "init_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

InitNetwork::InitNetwork(vector<int>& init_states,
						 int num_states,
						 int num_obs) {
	this->type = NETWORK_TYPE_INIT;

	this->init_states = init_states;

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
	this->output->acti_vals.resize(this->init_states.size());
	this->output->errors.resize(this->init_states.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->last_get_max_update_iter = -1;
	this->last_update_weights_iter = -1;
}

InitNetwork::InitNetwork(InitNetwork* original) {
	this->type = NETWORK_TYPE_INIT;

	this->init_states = original->init_states;

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
	this->output->update_structure(NETWORK_INIT_MULTIPLIER);
	this->output->copy_weights_from(original->output);

	this->last_get_max_update_iter = -1;
	this->last_update_weights_iter = -1;
}

InitNetwork::InitNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_INIT;

	string num_init_states_line;
	getline(input_file, num_init_states_line);
	int num_init_states = stoi(num_init_states_line);
	for (int i_index = 0; i_index < num_init_states; i_index++) {
		string state_line;
		getline(input_file, state_line);
		this->init_states.push_back(stoi(state_line));
	}

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
	this->output->acti_vals.resize(this->init_states.size());
	this->output->errors.resize(this->init_states.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->last_get_max_update_iter = -1;
	this->last_update_weights_iter = -1;
}

InitNetwork::~InitNetwork() {
	delete this->state_input;
	delete this->obs_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void InitNetwork::init_activate(Eigen::VectorXf& state_norms,
								Eigen::VectorXf& state_vals,
								int new_state_norm,
								vector<double>& new_state_vals,
								vector<double>& obs_input_vals) {
	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = state_vals(s_index) / state_norms(s_index);
	}
	for (int s_index = 0; s_index < (int)new_state_vals.size(); s_index++) {
		this->state_input->acti_vals(state_vals.size() + s_index) = new_state_vals[s_index] / new_state_norm;
	}

	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		this->obs_input->acti_vals(i_index) = obs_input_vals[i_index];
	}

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		new_state_vals[i_index] += this->output->acti_vals(i_index);
	}

	this->end_state.resize(this->init_states.size());
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->end_state(i_index) = new_state_vals[i_index];
	}
}

void InitNetwork::init_activate_w_drop(Eigen::VectorXf& state_norms,
									   Eigen::VectorXf& state_vals,
									   int new_state_norm,
									   vector<double>& new_state_vals,
									   vector<double>& obs_input_vals) {
	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = state_vals(s_index) / state_norms(s_index);
	}
	for (int s_index = 0; s_index < (int)new_state_vals.size(); s_index++) {
		this->state_input->acti_vals(state_vals.size() + s_index) = new_state_vals[s_index] / new_state_norm;
	}

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

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		new_state_vals[i_index] += this->output->acti_vals(i_index);
	}

	this->end_state.resize(this->init_states.size());
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->end_state(i_index) = new_state_vals[i_index];
	}
}

void InitNetwork::init_backprop(int new_state_norm,
								vector<double>& new_state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->output->errors(i_index) = new_state_errors[i_index];
	}

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->output->errors(i_index) -= this->end_state(i_index)
			* abs(this->output->acti_vals(i_index)) * STATE_NORM_CONSTANT;
	}

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	for (int s_index = 0; s_index < (int)new_state_errors.size(); s_index++) {
		new_state_errors[new_state_errors.size()-1 - s_index] += this->state_input->errors(this->state_input->errors.size()-1 - s_index) / new_state_norm;
		this->state_input->errors(this->state_input->errors.size()-1 - s_index) = 0.0;
	}
}

void InitNetwork::init_update(double& hidden_1_average_max_update,
							  double& hidden_2_average_max_update,
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

void InitNetwork::activate(Eigen::VectorXf& state_norms,
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

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		state_vals(this->init_states[i_index]) += this->output->acti_vals(i_index);
	}

	this->end_state = state_vals;
}

void InitNetwork::activate_w_drop(Eigen::VectorXf& state_norms,
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

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		state_vals(this->init_states[i_index]) += this->output->acti_vals(i_index);
	}

	this->end_state = state_vals;
}

void InitNetwork::save(InitNetworkHistory* history) {
	history->state_norms_history = this->state_norms;
	history->state_input_history = this->state_input->acti_vals;
	history->obs_input_history = this->obs_input->acti_vals;
	history->hidden_1_history = this->hidden_1->acti_vals;
	history->hidden_2_history = this->hidden_2->acti_vals;
	history->output_history = this->output->acti_vals;

	history->end_state_history = this->end_state;
}

void InitNetwork::load(InitNetworkHistory* history) {
	this->state_norms = history->state_norms_history;
	this->state_input->acti_vals = history->state_input_history;
	this->obs_input->acti_vals = history->obs_input_history;
	this->hidden_1->acti_vals = history->hidden_1_history;
	this->hidden_2->acti_vals = history->hidden_2_history;
	this->output->acti_vals = history->output_history;

	this->end_state = history->end_state_history;
}

void InitNetwork::backprop(Eigen::VectorXf& state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->output->errors(i_index) = state_errors(this->init_states[i_index]);
	}

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->output->errors(i_index) -= this->end_state(this->init_states[i_index])
			* abs(this->output->acti_vals(i_index)) * STATE_NORM_CONSTANT;
	}

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	state_errors += this->state_input->errors.cwiseQuotient(this->state_norms);
	this->state_input->errors.setConstant(0.0);
}

void InitNetwork::get_max_update(double& max_update_size) {
	this->hidden_1->get_max_update(max_update_size);
	this->hidden_2->get_max_update(max_update_size);
	this->output->get_max_update(max_update_size);
}

void InitNetwork::update_weights(double learning_rate) {
	this->hidden_1->update_weights(learning_rate);
	this->hidden_2->update_weights(learning_rate);
	this->output->update_weights(learning_rate);
}

void InitNetwork::clear_update_weights() {
	this->hidden_1->clear_update_weights();
	this->hidden_2->clear_update_weights();
	this->output->clear_update_weights();
}

void InitNetwork::add_states(int new_num_states) {
	this->state_input->acti_vals.resize(new_num_states);
	this->state_input->errors.resize(new_num_states);
	this->state_input->errors.setConstant(0.0);

	/**
	 * - weight for new state + existing hidden is 0.0
	 */
	this->hidden_1->update_structure(0.0);
	this->hidden_2->update_structure(0.0);
}

void InitNetwork::save(ofstream& output_file) {
	output_file << this->init_states.size() << endl;
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		output_file << this->init_states[i_index] << endl;
	}

	output_file << this->state_input->acti_vals.size() << endl;

	output_file << this->obs_input->acti_vals.size() << endl;

	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}

InitNetworkHistory::InitNetworkHistory(InitNetwork* network) {
	this->network = network;
}
