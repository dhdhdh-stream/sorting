#include "negate_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

NegateNetwork::NegateNetwork(int state_index) {
	this->type = NETWORK_TYPE_NEGATE;

	this->state_index = state_index;
}

NegateNetwork::NegateNetwork(NegateNetwork* original) {
	this->type = NETWORK_TYPE_NEGATE;

	this->state_index = original->state_index;
}

NegateNetwork::NegateNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_NEGATE;

	string state_index_line;
	getline(input_file, state_index_line);
	this->state_index = stoi(state_index_line);
}

void NegateNetwork::save(ofstream& output_file) {
	output_file << this->state_index << endl;
}

NegateNetworkHistory::NegateNetworkHistory(NegateNetwork* network) {
	this->network = network;
}
