#include "state.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state_network.h"

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
		ifstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->networks.push_back(new StateNetwork(network_save_file,
												  this,
												  n_index));
		network_save_file.close();
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
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->networks[n_index]->save(network_save_file);
		network_save_file.close();
	}
}
