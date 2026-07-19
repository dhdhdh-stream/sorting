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
}

NegateNetwork::NegateNetwork(NegateNetwork* original) {
	this->type = NETWORK_TYPE_NEGATE;

	this->init_states = original->init_states;
	this->weights = original->weights;

	this->state_vals = vector<double>(this->init_states.size());

	this->weight_updates = vector<double>(this->init_states.size(), 0.0);
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
}

void NegateNetwork::activate(vector<double>& state_vals) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->state_vals[i_index] = state_vals[this->init_states[i_index]];

		state_vals[this->init_states[i_index]] += this->weights[i_index] * this->state_vals[i_index];
	}
}

void NegateNetwork::save(NegateNetworkHistory* history) {
	history->state_vals_history = this->state_vals;
}

void NegateNetwork::load(NegateNetworkHistory* history) {
	this->state_vals = history->state_vals_history;
}

void NegateNetwork::backprop(vector<double>& state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		double error = state_errors[this->init_states[i_index]];
		state_errors[this->init_states[i_index]] += this->weights[i_index] * error;
		this->weight_updates[i_index] += this->state_vals[i_index] * error;
	}
}

void NegateNetwork::update_weights(double learning_rate) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		this->weights[i_index] += learning_rate * this->weight_updates[i_index];
		this->weight_updates[i_index] = 0.0;
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
