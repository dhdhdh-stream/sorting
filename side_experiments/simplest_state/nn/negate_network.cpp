#include "negate_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

NegateNetwork::NegateNetwork(vector<int>& init_states) {
	this->type = NETWORK_TYPE_NEGATE;

	this->init_states = init_states;
}

NegateNetwork::NegateNetwork(NegateNetwork* original) {
	this->type = NETWORK_TYPE_NEGATE;

	this->init_states = original->init_states;
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
}

void NegateNetwork::activate(vector<double>& state_vals) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		state_vals[this->init_states[i_index]] = 0.0;
	}
}

void NegateNetwork::backprop_through(vector<double>& state_errors) {
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		state_errors[this->init_states[i_index]] = 0.0;
	}
}

void NegateNetwork::save(ofstream& output_file) {
	output_file << this->init_states.size() << endl;
	for (int i_index = 0; i_index < (int)this->init_states.size(); i_index++) {
		output_file << this->init_states[i_index] << endl;
	}
}

NegateNetworkHistory::NegateNetworkHistory(NegateNetwork* network) {
	this->network = network;
}
