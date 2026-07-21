#include "init_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

InitNetwork::InitNetwork(vector<int>& init_states,
						 int num_obs) {
	this->type = NETWORK_TYPE_INIT;

	this->init_states = init_states;

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(this->init_states.size());
	this->state_input->errors.resize(this->init_states.size());
	this->state_input->errors.setConstant(0.0);

	this->raw_obs_input = new Layer(LINEAR_LAYER);
	this->raw_obs_input->acti_vals.resize(num_obs);
	this->raw_obs_input->errors.resize(num_obs);
	this->raw_obs_input->errors.setConstant(0.0);

	this->obs_input_means.resize(num_obs);
	this->obs_input_means.setConstant(0.0);
	this->obs_input_deviations.resize(num_obs);
	this->obs_input_deviations.setConstant(1.0);

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

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
	this->last_update_iter = -1;
}

InitNetwork::InitNetwork(InitNetwork* original) {
	this->type = NETWORK_TYPE_INIT;

	this->init_states = original->init_states;

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(original->state_input->acti_vals.size());
	this->state_input->errors.resize(original->state_input->errors.size());
	this->state_input->errors.setConstant(0.0);

	this->raw_obs_input = new Layer(LINEAR_LAYER);
	this->raw_obs_input->acti_vals.resize(original->raw_obs_input->acti_vals.size());
	this->raw_obs_input->errors.resize(original->raw_obs_input->errors.size());
	this->raw_obs_input->errors.setConstant(0.0);

	this->obs_input_means = original->obs_input_means;
	this->obs_input_deviations = original->obs_input_deviations;

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

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
	this->last_update_iter = -1;
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
	this->state_input->acti_vals.resize(this->init_states.size());
	this->state_input->errors.resize(this->init_states.size());
	this->state_input->errors.setConstant(0.0);

	string num_obs_line;
	getline(input_file, num_obs_line);
	int num_obs = stoi(num_obs_line);

	this->raw_obs_input = new Layer(LINEAR_LAYER);
	this->raw_obs_input->acti_vals.resize(num_obs);
	this->raw_obs_input->errors.resize(num_obs);
	this->raw_obs_input->errors.setConstant(0.0);

	this->obs_input_means.resize(num_obs);
	this->obs_input_deviations.resize(num_obs);
	for (int i_index = 0; i_index < num_obs; i_index++) {
		string mean_line;
		getline(input_file, mean_line);
		this->obs_input_means(i_index) = stod(mean_line);

		string deviation_line;
		getline(input_file, deviation_line);
		this->obs_input_deviations(i_index) = stod(deviation_line);
	}

	this->obs_input = new Layer(LINEAR_LAYER);
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

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
	this->last_update_iter = -1;
}

InitNetwork::~InitNetwork() {
	delete this->state_input;
	delete this->raw_obs_input;
	delete this->obs_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void InitNetwork::init_activate(vector<double>& new_state_vals,
								vector<double>& obs_input_vals) {
	for (int s_index = 0; s_index < (int)new_state_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = new_state_vals[s_index];
	}

	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		this->raw_obs_input->acti_vals(i_index) = obs_input_vals[i_index];
	}
	this->obs_input->acti_vals = (this->raw_obs_input->acti_vals - this->obs_input_means).cwiseQuotient(this->obs_input_deviations);

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		new_state_vals[i_index] += this->output->acti_vals(i_index);
	}
}

void InitNetwork::init_backprop(vector<double>& new_state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->output->errors(i_index) = new_state_errors[i_index];
	}
	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->obs_input_means = 0.99999*this->obs_input_means + 0.00001*this->raw_obs_input->acti_vals;
	this->obs_input_deviations = 0.99999*this->obs_input_deviations
		+ 0.00001*(this->raw_obs_input->acti_vals - this->obs_input_means).cwiseAbs();

	for (int s_index = 0; s_index < (int)new_state_errors.size(); s_index++) {
		new_state_errors[s_index] += this->state_input->errors(s_index);
		this->state_input->errors(s_index) = 0.0;
	}
}

void InitNetwork::init_update(double& hidden_1_average_max_update,
							  double& hidden_2_average_max_update,
							  double& output_average_max_update) {
	double hidden_1_max_update = 0.0;
	this->hidden_1->get_max_update(hidden_1_max_update);
	hidden_1_average_max_update = 0.999*hidden_1_average_max_update+0.001*hidden_1_max_update;
	if (hidden_1_max_update > 0.0) {
		double hidden_1_learning_rate = (0.3*NETWORK_INIT_TARGET_MAX_UPDATE)/hidden_1_average_max_update;
		if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_INIT_TARGET_MAX_UPDATE) {
			hidden_1_learning_rate = NETWORK_INIT_TARGET_MAX_UPDATE/hidden_1_max_update;
		}
		this->hidden_1->update_weights(hidden_1_learning_rate);
	}

	double hidden_2_max_update = 0.0;
	this->hidden_2->get_max_update(hidden_2_max_update);
	hidden_2_average_max_update = 0.999*hidden_2_average_max_update+0.001*hidden_2_max_update;
	if (hidden_2_max_update > 0.0) {
		double hidden_2_learning_rate = (0.3*NETWORK_INIT_TARGET_MAX_UPDATE)/hidden_2_average_max_update;
		if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_INIT_TARGET_MAX_UPDATE) {
			hidden_2_learning_rate = NETWORK_INIT_TARGET_MAX_UPDATE/hidden_2_max_update;
		}
		this->hidden_2->update_weights(hidden_2_learning_rate);
	}

	double output_max_update = 0.0;
	this->output->get_max_update(output_max_update);
	output_average_max_update = 0.999*output_average_max_update+0.001*output_max_update;
	if (output_max_update > 0.0) {
		double output_learning_rate = (0.3*NETWORK_INIT_TARGET_MAX_UPDATE)/output_average_max_update;
		if (output_learning_rate*output_max_update > NETWORK_INIT_TARGET_MAX_UPDATE) {
			output_learning_rate = NETWORK_INIT_TARGET_MAX_UPDATE/output_max_update;
		}
		this->output->update_weights(output_learning_rate);
	}
}

void InitNetwork::activate(vector<double>& state_vals,
						   vector<double>& obs_input_vals) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->state_input->acti_vals(i_index) = state_vals[this->init_states[i_index]];
	}

	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		this->raw_obs_input->acti_vals(i_index) = obs_input_vals[i_index];
	}
	this->obs_input->acti_vals = (this->raw_obs_input->acti_vals - this->obs_input_means).cwiseQuotient(this->obs_input_deviations);

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		state_vals[this->init_states[i_index]] += this->output->acti_vals(i_index);
	}
}

void InitNetwork::save(InitNetworkHistory* history) {
	history->state_input_history = vector<double>(this->state_input->acti_vals.size());
	for (int s_index = 0; s_index < (int)this->state_input->acti_vals.size(); s_index++) {
		history->state_input_history[s_index] = this->state_input->acti_vals(s_index);
	}
	history->raw_obs_input_history = vector<double>(this->raw_obs_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->raw_obs_input->acti_vals.size(); i_index++) {
		history->raw_obs_input_history[i_index] = this->raw_obs_input->acti_vals(i_index);
	}
	history->obs_input_history = vector<double>(this->obs_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		history->obs_input_history[i_index] = this->obs_input->acti_vals(i_index);
	}
	history->hidden_1_history = vector<double>(this->hidden_1->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		history->hidden_1_history[h_index] = this->hidden_1->acti_vals(h_index);
	}
	history->hidden_2_history = vector<double>(this->hidden_2->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		history->hidden_2_history[h_index] = this->hidden_2->acti_vals(h_index);
	}
}

void InitNetwork::load(InitNetworkHistory* history) {
	for (int s_index = 0; s_index < (int)this->state_input->acti_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = history->state_input_history[s_index];
	}
	for (int i_index = 0; i_index < (int)this->raw_obs_input->acti_vals.size(); i_index++) {
		this->raw_obs_input->acti_vals(i_index) = history->raw_obs_input_history[i_index];
	}
	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		this->obs_input->acti_vals(i_index) = history->obs_input_history[i_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		this->hidden_1->acti_vals(h_index) = history->hidden_1_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		this->hidden_2->acti_vals(h_index) = history->hidden_2_history[h_index];
	}
}

void InitNetwork::backprop(vector<double>& state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->output->errors(i_index) = state_errors[this->init_states[i_index]];
	}
	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->obs_input_means = 0.99999*this->obs_input_means + 0.00001*this->raw_obs_input->acti_vals;
	this->obs_input_deviations = 0.99999*this->obs_input_deviations
		+ 0.00001*(this->raw_obs_input->acti_vals - this->obs_input_means).cwiseAbs();

	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		state_errors[this->init_states[i_index]] += this->state_input->errors(i_index);
		this->state_input->errors(i_index) = 0.0;
	}
}

void InitNetwork::update() {
	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double max_update = 0.0;
		this->hidden_1->get_max_update(max_update);
		this->hidden_2->get_max_update(max_update);
		this->output->get_max_update(max_update);
		this->average_max_update = 0.999*this->average_max_update+0.001*max_update;
		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}
			this->hidden_1->update_weights(learning_rate);
			this->hidden_2->update_weights(learning_rate);
			this->output->update_weights(learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void InitNetwork::save(ofstream& output_file) {
	output_file << this->init_states.size() << endl;
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		output_file << this->init_states[i_index] << endl;
	}

	output_file << this->obs_input->acti_vals.size() << endl;

	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		output_file << this->obs_input_means[i_index] << endl;
		output_file << this->obs_input_deviations[i_index] << endl;
	}

	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}

InitNetworkHistory::InitNetworkHistory(InitNetwork* network) {
	this->network = network;
}
