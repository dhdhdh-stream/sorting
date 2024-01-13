#include "state.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "state_network.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

State::State() {
	// do nothing
}

State::State(ifstream& input_file,
			 int id) {
	this->id = id;

	string num_networks_line;
	getline(input_file, num_networks_line);
	int num_networks = stoi(num_networks_line);
	for (int n_index = 0; n_index < num_networks; n_index++) {
		this->networks.push_back(new StateNetwork(input_file,
												  this,
												  n_index));
	}
}

State::~State() {
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		delete this->networks[n_index];
	}
}

void State::save(ofstream& output_file) {
	output_file << this->networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		this->networks[n_index]->save(output_file);
	}
}
