#include "node.h"

#include <iostream>

using namespace std;

const double TARGET_MAX_UPDATE = 0.001;

Node::Node(string id,
		   ScoreNetwork* score_network,
		   bool just_score,
		   bool update_existing_scope,
		   int new_scope_size,
		   StateNetwork* state_network,
		   vector<int> compress_num_scopes,
		   vector<int> compress_sizes,
		   vector<CompressionNetwork*> compression_networks,
		   vector<vector<int>> compressed_scope_sizes) {
	this->id = id;
	this->score_network = score_network;
	this->just_score = just_score;
	this->update_existing_scope = update_existing_scope;
	this->new_scope_size = new_scope_size;
	this->state_network = state_network;
	this->compress_num_scopes = compress_num_scopes;
	this->compress_sizes = compress_sizes;
	this->compression_networks = compression_networks;
	this->compressed_scope_sizes = compressed_scope_sizes;
}

Node::Node(string id,
		   ifstream& input_file) {
	this->id = id;

	ifstream network_save_file;
	network_save_file.open("saves/nns/" + this->id + "_score.txt");
	this->score_network = new ScoreNetwork(network_save_file);
	network_save_file.close();

	string just_score_line;
	getline(input_file, just_score_line);
	this->just_score = stoi(just_score_line);

	string update_existing_scope_line;
	getline(input_file, update_existing_scope_line);
	this->update_existing_scope = stoi(update_existing_scope_line);

	string new_scope_size_line;
	getline(input_file, new_scope_size_line);
	this->new_scope_size = stoi(new_scope_size_line);

	// copy observation if !update_existing_scope for now
	if (!this->just_score && this->update_existing_scope) {
		ifstream network_save_file;
		network_save_file.open("saves/nns/" + this->id + "_state.txt");
		this->state_network = new StateNetwork(network_save_file);
		network_save_file.close();
	} else {
		this->state_network = NULL;
	}

	string num_compression_line;
	getline(input_file, num_compression_line);
	int num_compression = stoi(num_compression_line);
	for (int c_index = 0; c_index < num_compression; c_index++) {
		string compress_num_scopes_line;
		getline(input_file, compress_num_scopes_line);
		this->compress_num_scopes.push_back(stoi(compress_num_scopes_line));

		string compress_sizes_line;
		getline(input_file, compress_sizes_line);
		this->compress_sizes.push_back(stoi(compress_sizes_line));

		this->compressed_scope_sizes.push_back(vector<int>(this->compress_num_scopes[c_index]));
		for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
			string compressed_scope_sizes_line;
			getline(input_file, compressed_scope_sizes_line);
			this->compressed_scope_sizes[c_index][s_index] = stoi(compressed_scope_sizes_line);
		}

		ifstream network_save_file;
		network_save_file.open("saves/nns/" + this->id + "_post_" + to_string(c_index) + ".txt");
		this->compression_networks.push_back(new CompressionNetwork(network_save_file));
		network_save_file.close();
	}
}

Node::Node(Node* original) {
	this->id = original->id + "_c";
	this->score_network = new ScoreNetwork(original->score_network);
	this->just_score = original->just_score;
	this->update_existing_scope = original->update_existing_scope;
	this->new_scope_size = original->new_scope_size;
	if (!this->just_score && this->update_existing_scope) {
		this->state_network = new StateNetwork(original->state_network);
	} else {
		this->state_network = NULL;
	}
	this->compress_num_scopes = original->compress_num_scopes;
	this->compress_sizes = original->compress_sizes;
	for (int c_index = 0; c_index < (int)original->compression_networks.size(); c_index++) {
		this->compression_networks.push_back(new CompressionNetwork(original->compression_networks[c_index]));
	}
	this->compressed_scope_sizes = original->compressed_scope_sizes;
}

Node::~Node() {
	delete this->score_network;
	if (this->state_network != NULL) {
		delete this->state_network;
	}
	for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
		delete this->compression_networks[c_index];
	}
}

void Node::activate(vector<vector<double>>& state_vals,
					vector<bool>& scopes_on,
					double observation) {
	vector<double> obs{observation};

	this->score_network->activate(state_vals,
								  obs);
	state_vals[0][0] = this->score_network->output->acti_vals[0];

	if (just_score) {
		state_vals.erase(state_vals.begin()+1, state_vals.end());
		scopes_on.erase(scopes_on.begin()+1, scopes_on.end());
	} else {
		// copy observation if !update_existing_scope for now
		if (!update_existing_scope) {
			state_vals.push_back(vector<double>{observation});
			scopes_on.push_back(true);
		} else {
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

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
			scopes_on.push_back(true);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
			}
		}
	}
}

void Node::backprop(double target_val,
					vector<vector<double>>& state_errors) {
	if (!this->just_score) {
		for (int c_index = (int)this->compression_networks.size()-1; c_index >= 0; c_index--) {
			this->compression_networks[c_index]->backprop(state_errors.back(),
														  TARGET_MAX_UPDATE);

			state_errors.pop_back();
			for (int sc_index = this->compress_num_scopes[c_index]-1; sc_index >= 0; sc_index--) {
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[c_index][sc_index]));
			}

			// state_errors[0][0] doesn't matter
			for (int sc_index = 1; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->compression_networks[c_index]->state_inputs[sc_index]->errors[st_index];
					this->compression_networks[c_index]->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
		}

		// copy observation if !update_existing_scope for now
		if (!update_existing_scope) {
			state_errors.pop_back();
		} else {
			this->state_network->backprop(state_errors.back(),
										  TARGET_MAX_UPDATE);
			// state_errors[0][0] doesn't matter
			for (int sc_index = 1; sc_index < (int)state_errors.size()-1; sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->state_network->state_inputs[sc_index]->errors[st_index];
					this->state_network->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
				state_errors.back()[st_index] = this->state_network->state_inputs.back()->errors[st_index];
				this->state_network->state_inputs.back()->errors[st_index] = 0.0;
			}
		}

		this->score_network->backprop_weights_with_no_error_signal(
			target_val,
			TARGET_MAX_UPDATE);
	}
}

void Node::backprop_zero_train(Node* original,
							   double& sum_error) {
	if (!this->just_score) {
		for (int c_index = (int)this->compression_networks.size()-1; c_index >= 0; c_index--) {
			vector<double> errors(this->compression_networks[c_index]->output->acti_vals.size());
			for (int o_index = 0; o_index < (int)this->compression_networks[c_index]->output->acti_vals.size(); o_index++) {
				errors[o_index] = original->compression_networks[c_index]->output->acti_vals[o_index]
					- this->compression_networks[c_index]->output->acti_vals[o_index];
				sum_error += abs(errors[o_index]);
			}
			this->compression_networks[c_index]->backprop_weights_with_no_error_signal(
				errors,
				TARGET_MAX_UPDATE);
		}

		if (update_existing_scope) {
			vector<double> errors(this->state_network->output->acti_vals.size());
			for (int o_index = 0; o_index < (int)this->state_network->output->acti_vals.size(); o_index++) {
				errors[o_index] = original->state_network->output->acti_vals[o_index]
					- this->state_network->output->acti_vals[o_index];
				sum_error += abs(errors[o_index]);
			}
			this->state_network->backprop_weights_with_no_error_signal(
				errors,
				TARGET_MAX_UPDATE);
		}
	}
}

void Node::activate_state(vector<vector<double>>& state_vals,
						  vector<bool>& scopes_on,
						  double observation) {
	if (this->just_score) {
		// do nothing
	} else {
		// has to be update_existing_scope
		vector<double> obs{observation};
		this->state_network->activate(state_vals,
									  scopes_on,
									  obs);
	}
}

void Node::backprop_zero_train_state(Node* original,
									 double& sum_error) {
	if (this->just_score) {
		// do nothing
	} else {
		// has to be update_existing_scope
		vector<double> errors(this->state_network->output->acti_vals.size());
		for (int o_index = 0; o_index < (int)this->state_network->output->acti_vals.size(); o_index++) {
			errors[o_index] = original->state_network->output->acti_vals[o_index]
				- this->state_network->output->acti_vals[o_index];
			sum_error += abs(errors[o_index]);
		}
		this->state_network->backprop_weights_with_no_error_signal(
			errors,
			TARGET_MAX_UPDATE);
	}
}

void Node::get_scope_sizes(vector<int>& scope_sizes) {
	if (just_score) {
		scope_sizes.erase(scope_sizes.begin()+1, scope_sizes.end());
	} else {
		if (!update_existing_scope) {
			scope_sizes.push_back(this->new_scope_size);
		}

		for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
			scope_sizes.pop_back();
		}
	}
}

void Node::save(ofstream& output_file) {
	ofstream network_save_file;
	network_save_file.open("saves/nns/" + this->id + "_score.txt");
	this->score_network->save(network_save_file);
	network_save_file.close();

	if (this->just_score) {
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

	if (!this->just_score && this->update_existing_scope) {
		ofstream network_save_file;
		network_save_file.open("saves/nns/" + this->id + "_state.txt");
		this->state_network->save(network_save_file);
		network_save_file.close();
	}

	output_file << this->compression_networks.size() << endl;
	for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
		output_file << this->compress_num_scopes[c_index] << endl;
		output_file << this->compress_sizes[c_index] << endl;
		for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
			output_file << this->compressed_scope_sizes[c_index][s_index] << endl;
		}

		ofstream network_save_file;
		network_save_file.open("saves/nns/" + this->id + "_post_" + to_string(c_index) + ".txt");
		this->compression_networks[c_index]->save(network_save_file);
		network_save_file.close();
	}
}
