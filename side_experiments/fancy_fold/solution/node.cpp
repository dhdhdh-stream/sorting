#include "node.h"

#include <iostream>

using namespace std;

Node::Node(int id,
		   bool outputs_state,
		   bool update_existing_scope,
		   int new_scope_size,
		   Network* state_network,
		   std::vector<Network*> compression_networks) {
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
		this->state_network = new Network(network_save_file);
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
		this->compression_networks.push_back(new Network(network_save_file));
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
					double observation) {
	if (outputs_state) {
		// copy observation if !update_existing_scope for now
		if (!update_existing_scope) {
			state_vals.push_back(vector<double>{observation});
		} else {
			vector<double> inputs;
			inputs.push_back(observation);
			for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
					inputs.push_back(state_vals[sc_index][st_index]);
				}
			}
			this->state_network->activate(inputs);
			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
			}
		}

		for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
			vector<double> inputs;
			for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
					inputs.push_back(state_vals[sc_index][st_index]);
				}
			}
			state_vals.pop_back();

			this->compression_networks[c_index]->activate(inputs);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
			}
		}
	}
}

void Node::activate_zero_train(vector<vector<double>>& state_vals,
							   double observation,
							   vector<vector<double>>& zero_train_state_vals) {
	activate(state_vals, observation);
	activate(zero_train_state_vals, observation);
}

// void Node::backprop(vector<vector<double>>& state_errors) {
// 	if (outputs_state) {
// 		for (int c_index = (int)this->compression_networks.size()-1; c_index >= 0; c_index--) {
// 			this->compression_networks[c_index]->backprop(state_errors.back());

// 			state_errors.push_back(vector<double>(this->compressed_scope_sizes[c_index]));
			
// 			int input_index = 0;
// 			for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
// 				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
// 					state_errors[sc_index][st_index] += this->compression_networks[c_index]->input->errors[input_index];
// 					this->compression_networks[c_index]->input->errors[input_index] = 0.0;
// 					input_index++;
// 				}
// 			}
// 		}

// 		// copy observation if !update_existing_scope for now
// 		if (!update_existing_scope) {
// 			state_errors.pop_back();
// 		} else {
// 			this->state_network->backprop(state_errors.back());
// 			int input_index = 0;
// 			for (int sc_index = 0; sc_index < (int)state_errors.size()-1; sc_index++) {
// 				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
// 					state_errors[sc_index][st_index] += this->state_network->input->errors[1+input_index];
// 					this->state_network->input->errors[1+input_index] = 0.0;
// 					input_index++;
// 				}
// 			}
// 			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
// 				state_errors.back()[st_index] = this->state_network->input->errors[1+input_index];
// 				this->state_network->input->errors[1+input_index] = 0.0;
// 				input_index++;
// 			}
// 		}
// 	}
// }

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
