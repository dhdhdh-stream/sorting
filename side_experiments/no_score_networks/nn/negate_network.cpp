#include "negate_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

NegateNetwork::NegateNetwork(int state) {
	this->type = NETWORK_TYPE_NEGATE;

	this->state = state;
}

NegateNetwork::NegateNetwork(NegateNetwork* original) {
	this->type = NETWORK_TYPE_NEGATE;

	this->state = original->state;
}

NegateNetwork::NegateNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_NEGATE;

	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);
}

void NegateNetwork::activate(vector<double>& state_vals) {
	state_vals[this->state] = 0.0;
}

void NegateNetwork::backprop_through(vector<double>& state_errors) {
	state_errors[this->state] = 0.0;
}

void NegateNetwork::save(ofstream& output_file) {
	output_file << this->state << endl;
}

NegateNetworkHistory::NegateNetworkHistory(NegateNetwork* network) {
	this->network = network;
}
