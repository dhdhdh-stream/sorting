#include "negate_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

NegateNetwork::NegateNetwork(vector<int>& init_states) {
	this->type = NETWORK_TYPE_NEGATE;

	this->init_states = init_states;

	this->weights = vector<double>(this->init_states.size(), -1.0);

	this->state_vals = vector<double>(this->init_states.size());

	this->weight_updates = vector<double>(this->init_states.size(), 0.0);

	this->epoch_iter = 0;
	this->average_max_updates = vector<double>(this->init_states.size(), 0.0);
	this->last_update_iter = -1;
}

NegateNetwork::NegateNetwork(NegateNetwork* original) {
	this->type = NETWORK_TYPE_NEGATE;

	this->init_states = original->init_states;

	this->weights = original->weights;

	this->state_vals = vector<double>(this->init_states.size());

	this->weight_updates = vector<double>(this->init_states.size(), 0.0);

	this->epoch_iter = 0;
	this->average_max_updates = vector<double>(this->init_states.size(), 0.0);
	this->last_update_iter = -1;
}

NegateNetwork::NegateNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_NEGATE;

	string num_init_states_line;
	getline(input_file, num_init_states_line);
	int num_init_states = stoi(num_init_states_line);
	for (int i_index = 0; i_index < num_init_states; i_index++) {
		string state_line;
		getline(input_file, state_line);
		this->init_states.push_back(stoi(state_line));
	}

	for (int i_index = 0; i_index < num_init_states; i_index++) {
		string weight_line;
		getline(input_file, weight_line);
		this->weights.push_back(stod(weight_line));
	}

	this->state_vals = vector<double>(this->init_states.size());

	this->weight_updates = vector<double>(this->init_states.size(), 0.0);

	this->epoch_iter = 0;
	this->average_max_updates = vector<double>(this->init_states.size(), 0.0);
	this->last_update_iter = -1;
}

void NegateNetwork::activate(vector<double>& state_vals) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->state_vals[i_index] = state_vals[this->init_states[i_index]];

		state_vals[this->init_states[i_index]] += this->weights[i_index] * this->state_vals[i_index];
	}
}

void NegateNetwork::save(NegateNetworkHistory* history) {
	history->state_vals_history = vector<double>(this->state_vals.size());
	for (int i_index = 0; i_index < (int)this->state_vals.size(); i_index++) {
		history->state_vals_history[i_index] = this->state_vals[i_index];
	}
}

void NegateNetwork::load(NegateNetworkHistory* history) {
	for (int i_index = 0; i_index < (int)this->state_vals.size(); i_index++) {
		this->state_vals[i_index] = history->state_vals_history[i_index];
	}
}

void NegateNetwork::backprop(vector<double>& state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		double error = state_errors[this->init_states[i_index]];
		state_errors[this->init_states[i_index]] += this->weights[i_index] * error;
		this->weight_updates[i_index] += this->state_vals[i_index] * error;
	}
}

void NegateNetwork::update() {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->average_max_updates[i_index] = 0.999*this->average_max_updates[i_index]+0.001*this->weight_updates[i_index];
		if (this->weight_updates[i_index] > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->average_max_updates[i_index];
			if (learning_rate*this->weight_updates[i_index] > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/this->weight_updates[i_index];
			}
			double update = this->weight_updates[i_index] * learning_rate;
			this->weight_updates[i_index] = 0.0;
			this->weights[i_index] += update;
		}
	}
}

void NegateNetwork::save(ofstream& output_file) {
	output_file << this->init_states.size() << endl;
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		output_file << this->init_states[i_index] << endl;
	}
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		output_file << this->weights[i_index] << endl;
	}
}

NegateNetworkHistory::NegateNetworkHistory(NegateNetwork* network) {
	this->network = network;
}
