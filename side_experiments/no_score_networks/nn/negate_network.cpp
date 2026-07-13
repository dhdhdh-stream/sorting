#include "negate_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

NegateNetwork::NegateNetwork(int state) {
	this->type = NETWORK_TYPE_NEGATE;

	this->state = state;
	this->weight = -1.0;

	this->weight_update = 0.0;
}

NegateNetwork::NegateNetwork(NegateNetwork* original) {
	this->type = NETWORK_TYPE_NEGATE;

	this->state = original->state;
	this->weight = original->weight;

	this->weight_update = 0.0;
}

NegateNetwork::NegateNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_NEGATE;

	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	string weight_line;
	getline(input_file, weight_line);
	this->weight = stod(weight_line);

	this->weight_update = 0.0;
}

void NegateNetwork::activate(vector<double>& state_vals) {
	this->state_input = state_vals[this->state];

	state_vals[this->state] += this->weight * this->state_input;
}

void NegateNetwork::save(NegateNetworkHistory* history) {
	history->state_input_history = this->state_input;
}

void NegateNetwork::load(NegateNetworkHistory* history) {
	this->state_input = history->state_input_history;
}

void NegateNetwork::backprop(vector<double>& state_errors) {
	double error = state_errors[this->state];
	state_errors[this->state] += this->weight * error;
	this->weight_update += this->state_input * error;
}

void NegateNetwork::update_weights(double learning_rate) {
	this->weight += learning_rate * this->weight_update;
	this->weight_update = 0.0;
}

void NegateNetwork::save(ofstream& output_file) {
	output_file << this->state << endl;
	output_file << this->weight << endl;
}

NegateNetworkHistory::NegateNetworkHistory(NegateNetwork* network) {
	this->network = network;
}
