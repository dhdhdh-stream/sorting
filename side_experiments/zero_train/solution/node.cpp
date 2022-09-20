#include "node.h"

#include <iostream>

using namespace std;

Node::Node(int id,
		   bool outputs_state,
		   bool update_existing_scope,
		   int new_scope_size,
		   StateNetwork* state_network,
		   std::vector<CompressionNetwork*> compression_networks) {
	this->id = id;
	this->outputs_state = outputs_state;
	this->update_existing_scope = update_existing_scope;
	this->new_scope_size = new_scope_size;
	this->state_network = state_network;
	this->compression_networks = compression_networks;
}

Node::Node(int id,
		   ifstream& input_file) {
	this->id = id;

	string outputs_state_line;
	getline(input_file, outputs_state_line);
	this->outputs_state = stoi(outputs_state_line);

	string update_existing_scope_line;
	getline(input_file, update_existing_scope_line);
	this->update_existing_scope = stoi(update_existing_scope_line);

	string new_scope_size_line;
	getline(input_file, new_scope_size_line);
	this->new_scope_size = stoi(new_scope_size_line);

	// copy observation if !update_existing_scope for now
	if (this->outputs_state && this->update_existing_scope) {
		ifstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_state.txt");
		this->state_network = new StateNetwork(network_save_file);
		network_save_file.close();
	} else {
		this->state_network = NULL;
	}

	string num_compression_line;
	getline(input_file, num_compression_line);
	int num_compression = stoi(num_compression_line);
	for (int c_index = 0; c_index < num_compression; c_index++) {
		ifstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_post_" + to_string(c_index) + ".txt");
		this->compression_networks.push_back(new CompressionNetwork(network_save_file));
		network_save_file.close();
	}
}

Node::~Node() {
	if (this->state_network != NULL) {
		delete state_network;
	}
	for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
		delete this->compression_networks[c_index];
	}
}

void Node::activate(vector<vector<double>>& state_vals,
					vector<bool>& scopes_on,
					double observation) {
	if (outputs_state) {
		// copy observation if !update_existing_scope for now
		if (!update_existing_scope) {
			state_vals.push_back(vector<double>{observation});
			scopes_on.push_back(true);
		} else {
			vector<double> obs{observation};

			this->state_network->activate(state_vals,
										  scopes_on,
										  obs);
			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
			}
		}

		for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
			this->compression_networks[c_index]->activate(state_vals,
														  scopes_on);

			state_vals.pop_back();
			scopes_on.pop_back();
			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
			}
		}
	}
}

void Node::get_scope_sizes(vector<int>& scope_sizes) {
	if (outputs_state) {
		if (!update_existing_scope) {
			scope_sizes.push_back(this->new_scope_size);
		}

		for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
			scope_sizes.pop_back();
		}
	}
}

void Node::save(ofstream& output_file) {
	if (this->outputs_state) {
		output_file << 1 << endl;
	} else {
		output_file << 0 << endl;
	}

	if (this->update_existing_scope) {
		output_file << 1 << endl;
	} else{
		output_file << 0 << endl;
	}
	output_file << this->new_scope_size << endl;

	if (this->outputs_state && this->update_existing_scope) {
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_state.txt");
		this->state_network->save(network_save_file);
		network_save_file.close();
	}

	output_file << this->compression_networks.size() << endl;
	for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + to_string(this->id) + "_post_" + to_string(c_index) + ".txt");
		this->compression_networks[c_index]->save(network_save_file);
		network_save_file.close();
	}
}
