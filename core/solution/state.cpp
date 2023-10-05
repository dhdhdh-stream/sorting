#include "state.h"

#include "scale.h"
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

	string resolved_network_indexes_size_line;
	getline(input_file, resolved_network_indexes_size_line);
	int resolved_network_indexes_size = stoi(resolved_network_indexes_size_line);
	for (int i_index = 0; i_index < resolved_network_indexes_size; i_index++) {
		string index_line;
		getline(input_file, index_line);
		this->resolved_network_indexes.insert(stoi(index_line));
	}

	string resolved_standard_deviation_line;
	getline(input_file, resolved_standard_deviation_line);
	this->resolved_standard_deviation = stod(resolved_standard_deviation_line);

	string scale_weight_line;
	getline(input_file, scale_weight_line);
	this->scale = new Scale(stod(scale_weight_line));

	this->nodes = vector<AbstractNode*>(this->networks.size());
	/**
	 * - filled in when nodes are loaded
	 */
}

State::~State() {
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		delete this->networks[n_index];
	}

	delete this->scale;
}

void State::save(ofstream& output_file) {
	output_file << this->networks.size() << endl;
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->networks[n_index]->save(network_save_file);
		network_save_file.close();
	}

	output_file << this->resolved_network_indexes.size() << endl;
	for (set<int>::iterator it = this->resolved_network_indexes.begin();
			it != this->resolved_network_indexes.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->resolved_standard_deviation << endl;

	output_file << this->scale->weight << endl;
}
