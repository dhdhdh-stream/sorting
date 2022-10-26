#include "node.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

const double TARGET_MAX_UPDATE = 0.002;

Node::Node(string id,
		   int obs_size,
		   int new_layer_size,
		   Network* obs_network,
		   vector<int> score_input_layer,
		   vector<int> score_input_sizes,
		   vector<Network*> score_input_networks,
		   Network* score_network,
		   int compress_num_layers,
		   int compress_new_size,
		   vector<int> input_layer,
		   vector<int> input_sizes,
		   vector<Network*> input_networks,
		   Network* compression_network,
		   vector<int> compressed_scope_sizes) {
	this->id = id;
	this->obs_size = obs_size;
	this->new_layer_size = new_layer_size;
	this->obs_network = obs_network;
	this->score_input_layer = score_input_layer;
	this->score_input_sizes = score_input_sizes;
	this->score_input_networks = score_input_networks;
	this->score_network = score_network;
	this->compress_num_layers = compress_num_layers;
	this->compress_new_size = compress_new_size;
	this->input_layer = input_layer;
	this->input_sizes = input_sizes;
	this->input_networks = input_networks;
	this->compression_network = compression_network;
	this->compressed_scope_sizes = compressed_scope_sizes;
}

Node::Node(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	boost::algorithm::trim(id_line);
	this->id = id_line;

	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	string new_layer_size_line;
	getline(input_file, new_layer_size_line);
	this->new_layer_size = stoi(new_layer_size_line);

	if (this->new_layer_size > 0) {
		ifstream obs_network_save_file;
		obs_network_save_file.open("saves/nns/" + this->id + "_obs.txt");
		this->obs_network = new Network(obs_network_save_file);
		obs_network_save_file.close();

		string num_score_input_line;
		getline(input_file, num_score_input_line);
		int num_score_input = stoi(num_score_input_line);
		for (int i_index = 0; i_index < num_score_input; i_index++) {
			string input_layer_line;
			getline(input_file, input_layer_line);
			this->score_input_layer.push_back(stoi(input_layer_line));

			string input_size_line;
			getline(input_file, input_size_line);
			this->score_input_sizes.push_back(stoi(input_size_line));

			ifstream score_input_network_save_file;
			score_input_network_save_file.open("saves/nns/" + this->id + "_score_input_" + to_string(i_index) + ".txt");
			this->score_input_networks.push_back(new Network(score_input_network_save_file));
			score_input_network_save_file.close();
		}

		ifstream score_network_save_file;
		score_network_save_file.open("saves/nns/" + this->id + "_score.txt");
		this->score_network = new Network(score_network_save_file);
		score_network_save_file.close();

		string compress_num_layers_line;
		getline(input_file, compress_num_layers_line);
		this->compress_num_layers = stoi(compress_num_layers_line);

		string compress_new_size_line;
		getline(input_file, compress_new_size_line);
		this->compress_new_size = stoi(compress_new_size_line);

		if (this->compress_num_layers > 0 && this->compress_new_size > 0) {
			string num_input_line;
			getline(input_file, num_input_line);
			int num_input = stoi(num_input_line);
			for (int i_index = 0; i_index < num_input; i_index++) {
				string input_layer_line;
				getline(input_file, input_layer_line);
				this->input_layer.push_back(stoi(input_layer_line));

				string input_size_line;
				getline(input_file, input_size_line);
				this->input_sizes.push_back(stoi(input_size_line));

				ifstream input_network_save_file;
				input_network_save_file.open("saves/nns/" + this->id + "_input_" + to_string(i_index) + ".txt");
				this->input_networks.push_back(new Network(input_network_save_file));
				input_network_save_file.close();
			}

			ifstream compression_network_save_file;
			compression_network_save_file.open("saves/nns/" + this->id + "_compress.txt");
			this->compression_network = new Network(compression_network_save_file);
			compression_network_save_file.close();
		} else {
			this->compression_network = NULL;
		}

		for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
			string scope_size_line;
			getline(input_file, scope_size_line);
			this->compressed_scope_sizes.push_back(stoi(scope_size_line));
		}
	} else {
		this->obs_network = NULL;
		this->score_network = NULL;
		this->compression_network = NULL;

		this->compress_num_layers = 0;
		this->compress_new_size = 0;
	}
}

Node::Node(Node* original) {
	this->id = original->id;
	this->obs_size = original->obs_size;
	this->new_layer_size = original->new_layer_size;
	if (this->new_layer_size > 0) {
		this->obs_network = new Network(original->obs_network);
		for (int i_index = 0; i_index < (int)original->score_input_networks.size(); i_index++) {
			this->score_input_layer.push_back(original->score_input_layer[i_index]);
			this->score_input_sizes.push_back(original->score_input_sizes[i_index]);
			this->score_input_networks.push_back(new Network(original->score_input_networks[i_index]));
		}
		this->score_network = new Network(original->score_network);
		this->compress_num_layers = original->compress_num_layers;
		this->compress_new_size = original->compress_new_size;
		if (this->compress_num_layers > 0 && this->compress_new_size > 0) {
			for (int i_index = 0; i_index < (int)original->input_networks.size(); i_index++) {
				this->input_layer.push_back(original->input_layer[i_index]);
				this->input_sizes.push_back(original->input_sizes[i_index]);
				this->input_networks.push_back(new Network(original->input_networks[i_index]));
			}
			this->compression_network = new Network(original->compression_network);
		} else {
			this->compression_network = NULL;
		}
		for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
			this->compressed_scope_sizes.push_back(original->compressed_scope_sizes[s_index]);
		}
	} else {
		this->obs_network = NULL;
		this->score_network = NULL;
		this->compression_network = NULL;

		this->compress_num_layers = 0;
		this->compress_new_size = 0;
	}
}

Node::~Node() {
	if (this->new_layer_size > 0) {
		delete this->obs_network;

		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			delete this->score_input_networks[i_index];
		}
		delete this->score_network;

		if (this->compress_num_layers > 0 && this->compress_new_size > 0) {
			for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
				delete this->input_networks[i_index];
			}
			delete this->compression_network;
		}
	}
}

void Node::activate(vector<vector<double>>& state_vals,
					vector<double>& obs,
					double& predicted_score) {
	if (this->new_layer_size > 0) {
		this->obs_network->activate(obs);
		state_vals.push_back(vector<double>(this->new_layer_size));
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}

		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				state_vals[this->score_input_layer[i_index]+1].push_back(
					this->score_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->score_network->activate(state_vals.back());
		predicted_score += this->score_network->output->acti_vals[0];

		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
					this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						state_vals[this->input_layer[i_index]+1].push_back(
							this->input_networks[i_index]->output->acti_vals[s_index]);
					}
				}

				vector<double> compression_inputs;
				for (int l_index = (int)state_vals.size()-this->compress_num_layers; l_index < (int)state_vals.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
						compression_inputs.push_back(state_vals[l_index][st_index]);
					}
				}
				this->compression_network->activate(compression_inputs);

				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
				}
				state_vals.push_back(vector<double>(this->compress_new_size));

				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_network->output->acti_vals[st_index];
				}
			}
		}
	}
}

void Node::backprop(vector<vector<double>>& state_errors,
					double& predicted_score,
					double target_val) {
	if (this->new_layer_size > 0) {
		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
				}
			} else {
				this->compression_network->backprop(state_errors.back(),
													TARGET_MAX_UPDATE);

				state_errors.pop_back();
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
				}

				int input_index = 0;
				for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_errors[l_index].size(); st_index++) {
						state_errors[l_index][st_index] += this->compression_network->input->errors[input_index];
						this->compression_network->input->errors[input_index] = 0.0;
						input_index++;
					}
				}

				for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
					vector<double> input_errors(this->input_sizes[i_index]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						input_errors[this->input_sizes[i_index]-1-s_index] = state_errors[this->input_layer[i_index]+1].back();
						state_errors[this->input_layer[i_index]+1].pop_back();
					}
					this->input_networks[i_index]->backprop(input_errors,
															TARGET_MAX_UPDATE);
					for (int st_index = 0; st_index < (int)state_errors[this->input_layer[i_index]].size(); st_index++) {
						state_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->input->errors[st_index];
						this->input_networks[i_index]->input->errors[st_index] = 0.0;
					}
				}
			}
		}

		vector<double> score_errors{target_val - predicted_score};
		this->score_network->backprop(score_errors, TARGET_MAX_UPDATE);
		for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
			state_errors.back()[st_index] += this->score_network->input->errors[st_index];
			this->score_network->input->errors[st_index] = 0.0;
		}
		predicted_score -= this->score_network->output->acti_vals[0];

		for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> score_input_errors(this->score_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				score_input_errors[this->score_input_sizes[i_index]-1-s_index] = state_errors[this->score_input_layer[i_index]+1].back();
				state_errors[this->score_input_layer[i_index]+1].pop_back();
			}
			this->score_input_networks[i_index]->backprop(score_input_errors,
														  TARGET_MAX_UPDATE);
			for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
				state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->input->errors[st_index];
				this->score_input_networks[i_index]->input->errors[st_index] = 0.0;
			}
		}

		this->obs_network->backprop_weights_with_no_error_signal(state_errors.back(), TARGET_MAX_UPDATE);
		state_errors.pop_back();
	}
}

void Node::backprop_errors_with_no_weight_change(vector<vector<double>>& state_errors,
												 double& predicted_score,
												 double target_val) {
	if (this->new_layer_size > 0) {
		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
				}
			} else {
				this->compression_network->backprop_errors_with_no_weight_change(state_errors.back());

				state_errors.pop_back();
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
				}

				int input_index = 0;
				for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_errors[l_index].size(); st_index++) {
						state_errors[l_index][st_index] += this->compression_network->input->errors[input_index];
						this->compression_network->input->errors[input_index] = 0.0;
						input_index++;
					}
				}

				for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
					vector<double> input_errors(this->input_sizes[i_index]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						input_errors[this->input_sizes[i_index]-1-s_index] = state_errors[this->input_layer[i_index]+1].back();
						state_errors[this->input_layer[i_index]+1].pop_back();
					}
					this->input_networks[i_index]->backprop_errors_with_no_weight_change(input_errors);
					for (int st_index = 0; st_index < (int)state_errors[this->input_layer[i_index]].size(); st_index++) {
						state_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->input->errors[st_index];
						this->input_networks[i_index]->input->errors[st_index] = 0.0;
					}
				}
			}
		}

		vector<double> score_errors{target_val - predicted_score};
		this->score_network->backprop_errors_with_no_weight_change(score_errors);
		for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
			state_errors.back()[st_index] += this->score_network->input->errors[st_index];
			this->score_network->input->errors[st_index] = 0.0;
		}
		predicted_score -= this->score_network->output->acti_vals[0];

		for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> score_input_errors(this->score_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				score_input_errors[this->score_input_sizes[i_index]-1-s_index] = state_errors[this->score_input_layer[i_index]+1].back();
				state_errors[this->score_input_layer[i_index]+1].pop_back();
			}
			this->score_input_networks[i_index]->backprop_errors_with_no_weight_change(score_input_errors);
			for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
				state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->input->errors[st_index];
				this->score_input_networks[i_index]->input->errors[st_index] = 0.0;
			}
		}

		// no need to backprop obs_network
		state_errors.pop_back();
	}
}

void Node::get_scope_sizes(vector<int>& scope_sizes) {
	if (this->new_layer_size > 0) {
		scope_sizes.push_back(this->new_layer_size);

		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			scope_sizes[this->score_input_layer[i_index]+1] += this->score_input_sizes[i_index];
		}

		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					scope_sizes.pop_back();
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
					scope_sizes[this->input_layer[i_index]+1] += this->input_sizes[i_index];
				}

				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					scope_sizes.pop_back();
				}
				scope_sizes.push_back(this->compress_new_size);
			}
		}
	}
}

void Node::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->obs_size << endl;
	output_file << this->new_layer_size << endl;

	if (this->new_layer_size > 0) {
		ofstream obs_network_save_file;
		obs_network_save_file.open("saves/nns/" + this->id + "_obs.txt");
		this->obs_network->save(obs_network_save_file);
		obs_network_save_file.close();

		output_file << this->score_input_networks.size() << endl;
		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			output_file << this->score_input_layer[i_index] << endl;
			output_file << this->score_input_sizes[i_index] << endl;

			ofstream score_input_network_save_file;
			score_input_network_save_file.open("saves/nns/" + this->id + "_score_input_" + to_string(i_index) + ".txt");
			this->score_input_networks[i_index]->save(score_input_network_save_file);
			score_input_network_save_file.close();
		}

		ofstream score_network_save_file;
		score_network_save_file.open("saves/nns/" + this->id + "_score.txt");
		this->score_network->save(score_network_save_file);
		score_network_save_file.close();

		output_file << this->compress_num_layers << endl;
		output_file << this->compress_new_size << endl;

		if (this->compress_num_layers > 0 && this->compress_new_size > 0) {
			output_file << this->input_networks.size() << endl;
			for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
				output_file << this->input_layer[i_index] << endl;
				output_file << this->input_sizes[i_index] << endl;

				ofstream input_network_save_file;
				input_network_save_file.open("saves/nns/" + this->id + "_input_" + to_string(i_index) + ".txt");
				this->input_networks[i_index]->save(input_network_save_file);
				input_network_save_file.close();
			}

			ofstream compression_network_save_file;
			compression_network_save_file.open("saves/nns/" + this->id + "_compress.txt");
			this->compression_network->save(compression_network_save_file);
			compression_network_save_file.close();
		}

		for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
			output_file << this->compressed_scope_sizes[s_index] << endl;
		}
	}
}
