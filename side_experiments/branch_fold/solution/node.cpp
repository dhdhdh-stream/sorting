#include "node.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

const double TARGET_MAX_UPDATE = 0.002;

Node::Node(string id,
		   bool is_scope,
		   SmallNetwork* action_input_network,
		   Scope* action,
		   int obs_size,
		   int new_layer_size,
		   Network* obs_network,
		   vector<int> score_input_layer,
		   vector<int> score_input_sizes,
		   vector<SmallNetwork*> score_input_networks,
		   SmallNetwork* score_network,
		   int compress_num_layers,
		   int compress_new_size,
		   vector<int> input_layer,
		   vector<int> input_sizes,
		   vector<SmallNetwork*> input_networks,
		   Network* compression_network,
		   vector<int> compressed_scope_sizes,
		   vector<int> compressed_s_input_sizes) {
	this->id = id;
	this->is_scope = is_scope;
	this->action_input_network = action_input_network;
	this->action = action;
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
	this->compressed_s_input_sizes = compressed_s_input_sizes;
}

Node::Node(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	boost::algorithm::trim(id_line);
	this->id = id_line;

	string is_scope_line;
	getline(input_file, is_scope_line);
	this->is_scope = stoi(is_scope_line);

	if (this->is_scope) {
		ifstream action_save_file;
		action_save_file.open("saves/" + this->id + "_scope.txt");
		this->action = new Scope(action_save_file);
		action_save_file.close();

		if (this->action->num_inputs > 0) {
			ifstream action_input_network_save_file;
			action_input_network_save_file.open("saves/nns/" + this->id + "_action_input.txt");
			this->action_input_network = new SmallNetwork(action_input_network_save_file);
			action_input_network_save_file.close();
		} else {
			this->action_input_network = NULL;
		}
	} else {
		this->action_input_network = NULL;
		this->action = NULL;
	}

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
			this->score_input_networks.push_back(new SmallNetwork(score_input_network_save_file));
			score_input_network_save_file.close();
		}

		ifstream score_network_save_file;
		score_network_save_file.open("saves/nns/" + this->id + "_score.txt");
		this->score_network = new SmallNetwork(score_network_save_file);
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
				this->input_networks.push_back(new SmallNetwork(input_network_save_file));
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
		for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
			string s_input_size_line;
			getline(input_file, s_input_size_line);
			this->compressed_s_input_sizes.push_back(stoi(s_input_size_line));
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
	if (this->is_scope) {
		if (this->action->num_inputs > 0) {
			delete this->action_input_network;
		}
		delete this->action;
	}

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
					vector<vector<double>>& s_input_vals,
					vector<double>& obs,
					double& predicted_score) {
	if (this->new_layer_size > 0) {
		this->obs_network->activate(obs);
		state_vals.push_back(vector<double>(this->new_layer_size));
		s_input_vals.push_back(vector<double>());
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}

		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]],
														  s_input_vals[this->score_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				s_input_vals[this->score_input_layer[i_index]+1].push_back(
					this->score_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->score_network->activate(state_vals.back(),
									  s_input_vals.back());
		predicted_score += this->score_network->output->acti_vals[0];

		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
					s_input_vals.pop_back();
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
					this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]],
															s_input_vals[this->input_layer[i_index]]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						s_input_vals[this->input_layer[i_index]+1].push_back(
							this->input_networks[i_index]->output->acti_vals[s_index]);
					}
				}

				vector<double> compression_inputs;
				for (int l_index = (int)state_vals.size()-this->compress_num_layers; l_index < (int)state_vals.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
						compression_inputs.push_back(state_vals[l_index][st_index]);
					}
				}
				for (int st_index = 0; st_index < (int)s_input_vals[(int)state_vals.size()-this->compress_num_layers].size(); st_index++) {
					compression_inputs.push_back(s_input_vals[(int)state_vals.size()-this->compress_num_layers][st_index]);
				}
				this->compression_network->activate(compression_inputs);

				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
					s_input_vals.pop_back();
				}
				state_vals.push_back(vector<double>(this->compress_new_size));
				// TODO: don't compress last layer of s_input_vals
				s_input_vals.push_back(vector<double>());

				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_network->output->acti_vals[st_index];
				}
			}
		}
	}
}

void Node::activate(vector<vector<double>>& state_vals,
					vector<vector<double>>& s_input_vals,
					vector<vector<double>>& inner_flat_vals,
					double& predicted_score) {
	vector<double> action_input;
	if (this->action->num_inputs > 0) {
		this->action_input_network->activate(state_vals.back(),
											 s_input_vals.back());
		action_input.reserve(this->action->num_inputs);
		for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
			action_input.push_back(this->action_input_network->output->acti_vals[i_index]);
		}
	}
	this->action->activate(inner_flat_vals,
						   action_input,
						   predicted_score);

	if (this->new_layer_size > 0) {
		this->obs_network->activate(this->action->outputs);
		state_vals.push_back(vector<double>(this->new_layer_size));
		s_input_vals.push_back(vector<double>());
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}

		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]],
														  s_input_vals[this->score_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				s_input_vals[this->score_input_layer[i_index]+1].push_back(
					this->score_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->score_network->activate(state_vals.back(),
									  s_input_vals.back());
		predicted_score += this->score_network->output->acti_vals[0];

		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
					s_input_vals.pop_back();
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
					this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]],
															s_input_vals[this->input_layer[i_index]]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						s_input_vals[this->input_layer[i_index]+1].push_back(
							this->input_networks[i_index]->output->acti_vals[s_index]);
					}
				}

				vector<double> compression_inputs;
				for (int l_index = (int)state_vals.size()-this->compress_num_layers; l_index < (int)state_vals.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
						compression_inputs.push_back(state_vals[l_index][st_index]);
					}
				}
				for (int st_index = 0; st_index < (int)s_input_vals[(int)state_vals.size()-this->compress_num_layers].size(); st_index++) {
					compression_inputs.push_back(s_input_vals[(int)state_vals.size()-this->compress_num_layers][st_index]);
				}
				this->compression_network->activate(compression_inputs);

				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
					s_input_vals.pop_back();
				}
				state_vals.push_back(vector<double>(this->compress_new_size));
				s_input_vals.push_back(vector<double>());

				for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
					state_vals.back()[st_index] = this->compression_network->output->acti_vals[st_index];
				}
			}
		}
	}
}

void Node::backprop(vector<vector<double>>& state_errors,
					vector<vector<double>>& s_input_errors,
					double& predicted_score,
					double target_val) {
	if (this->new_layer_size > 0) {
		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
					s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[sc_index], 0.0));
				}
			} else {
				this->compression_network->backprop(state_errors.back(),
													TARGET_MAX_UPDATE);

				state_errors.pop_back();
				s_input_errors.pop_back();
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
					s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[sc_index], 0.0));
				}

				int input_index = 0;
				for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_errors[l_index].size(); st_index++) {
						state_errors[l_index][st_index] += this->compression_network->input->errors[input_index];
						this->compression_network->input->errors[input_index] = 0.0;
						input_index++;
					}
				}
				for (int st_index = 0; st_index < (int)s_input_errors[state_errors.size()-this->compress_num_layers].size(); st_index++) {
					s_input_errors[state_errors.size()-this->compress_num_layers][st_index] += this->compression_network->input->errors[input_index];
					this->compression_network->input->errors[input_index] = 0.0;
					input_index++;
				}

				for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
					vector<double> input_errors(this->input_sizes[i_index]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						input_errors[this->input_sizes[i_index]-1-s_index] = s_input_errors[this->input_layer[i_index]+1].back();
						s_input_errors[this->input_layer[i_index]+1].pop_back();
					}
					this->input_networks[i_index]->backprop(input_errors,
															TARGET_MAX_UPDATE);
					for (int st_index = 0; st_index < (int)state_errors[this->input_layer[i_index]].size(); st_index++) {
						state_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->state_input->errors[st_index];
						this->input_networks[i_index]->state_input->errors[st_index] = 0.0;
					}
					for (int st_index = 0; st_index < (int)s_input_errors[this->input_layer[i_index]].size(); st_index++) {
						s_input_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->s_input_input->errors[st_index];
						this->input_networks[i_index]->s_input_input->errors[st_index] = 0.0;
					}
				}
			}
		}

		vector<double> score_errors{target_val - predicted_score};
		this->score_network->backprop(score_errors, TARGET_MAX_UPDATE);
		for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
			state_errors.back()[st_index] += this->score_network->state_input->errors[st_index];
			this->score_network->state_input->errors[st_index] = 0.0;
		}
		for (int st_index = 0; st_index < (int)s_input_errors.back().size(); st_index++) {
			s_input_errors.back()[st_index] += this->score_network->s_input_input->errors[st_index];
			this->score_network->s_input_input->errors[st_index] = 0.0;
		}
		predicted_score -= this->score_network->output->acti_vals[0];

		for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> score_input_errors(this->score_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				score_input_errors[this->score_input_sizes[i_index]-1-s_index] = s_input_errors[this->score_input_layer[i_index]+1].back();
				s_input_errors[this->score_input_layer[i_index]+1].pop_back();
			}
			this->score_input_networks[i_index]->backprop(score_input_errors,
														  TARGET_MAX_UPDATE);
			for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
				state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->state_input->errors[st_index];
				this->score_input_networks[i_index]->state_input->errors[st_index] = 0.0;
			}
			for (int st_index = 0; st_index < (int)s_input_errors[this->score_input_layer[i_index]].size(); st_index++) {
				s_input_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->s_input_input->errors[st_index];
				this->score_input_networks[i_index]->s_input_input->errors[st_index] = 0.0;
			}
		}

		this->obs_network->backprop(state_errors.back(), TARGET_MAX_UPDATE);
		state_errors.pop_back();
		s_input_errors.pop_back();

		if (this->is_scope) {
			vector<double> action_input_errors(this->action->num_outputs);
			for (int o_index = 0; o_index < this->action->num_outputs; o_index++) {
				action_input_errors[o_index] = this->obs_network->input->errors[o_index];
				this->obs_network->input->errors[o_index] = 0.0;
			}
			vector<double> action_output_errors;
			this->action->backprop(action_input_errors,
								   action_output_errors,
								   predicted_score,
								   target_val);

			if (this->action->num_inputs > 0) {
				this->action_input_network->backprop(action_output_errors, TARGET_MAX_UPDATE);
				for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
					state_errors.back()[st_index] += this->action_input_network->state_input->errors[st_index];
					this->action_input_network->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)s_input_errors.back().size(); st_index++) {
					s_input_errors.back()[st_index] += this->action_input_network->s_input_input->errors[st_index];
					this->action_input_network->s_input_input->errors[st_index] = 0.0;
				}
			}
		}
	}
}

void Node::backprop_errors_with_no_weight_change(vector<vector<double>>& state_errors,
												 vector<vector<double>>& s_input_errors,
												 double& predicted_score,
												 double target_val) {
	if (this->new_layer_size > 0) {
		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
					s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[sc_index], 0.0));
				}
			} else {
				this->compression_network->backprop_errors_with_no_weight_change(state_errors.back());

				state_errors.pop_back();
				s_input_errors.pop_back();
				for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
					s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[sc_index], 0.0));
				}

				int input_index = 0;
				for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_errors[l_index].size(); st_index++) {
						state_errors[l_index][st_index] += this->compression_network->input->errors[input_index];
						this->compression_network->input->errors[input_index] = 0.0;
						input_index++;
					}
				}
				for (int st_index = 0; st_index < (int)s_input_errors[state_errors.size()-this->compress_num_layers].size(); st_index++) {
					s_input_errors[state_errors.size()-this->compress_num_layers][st_index] += this->compression_network->input->errors[input_index];
					this->compression_network->input->errors[input_index] = 0.0;
					input_index++;
				}

				for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
					vector<double> input_errors(this->input_sizes[i_index]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						input_errors[this->input_sizes[i_index]-1-s_index] = s_input_errors[this->input_layer[i_index]+1].back();
						s_input_errors[this->input_layer[i_index]+1].pop_back();
					}
					this->input_networks[i_index]->backprop_errors_with_no_weight_change(input_errors);
					for (int st_index = 0; st_index < (int)state_errors[this->input_layer[i_index]].size(); st_index++) {
						state_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->state_input->errors[st_index];
						this->input_networks[i_index]->state_input->errors[st_index] = 0.0;
					}
					for (int st_index = 0; st_index < (int)s_input_errors[this->input_layer[i_index]].size(); st_index++) {
						s_input_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->s_input_input->errors[st_index];
						this->input_networks[i_index]->s_input_input->errors[st_index] = 0.0;
					}
				}
			}
		}

		vector<double> score_errors{target_val - predicted_score};
		this->score_network->backprop_errors_with_no_weight_change(score_errors);
		for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
			state_errors.back()[st_index] += this->score_network->state_input->errors[st_index];
			this->score_network->state_input->errors[st_index] = 0.0;
		}
		for (int st_index = 0; st_index < (int)s_input_errors.back().size(); st_index++) {
			s_input_errors.back()[st_index] += this->score_network->s_input_input->errors[st_index];
			this->score_network->s_input_input->errors[st_index] = 0.0;
		}
		predicted_score -= this->score_network->output->acti_vals[0];

		for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> score_input_errors(this->score_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
				score_input_errors[this->score_input_sizes[i_index]-1-s_index] = s_input_errors[this->score_input_layer[i_index]+1].back();
				s_input_errors[this->score_input_layer[i_index]+1].pop_back();
			}
			this->score_input_networks[i_index]->backprop_errors_with_no_weight_change(score_input_errors);
			for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
				state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->state_input->errors[st_index];
				this->score_input_networks[i_index]->state_input->errors[st_index] = 0.0;
			}
			for (int st_index = 0; st_index < (int)s_input_errors[this->score_input_layer[i_index]].size(); st_index++) {
				s_input_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->s_input_input->errors[st_index];
				this->score_input_networks[i_index]->s_input_input->errors[st_index] = 0.0;
			}
		}

		this->obs_network->backprop_errors_with_no_weight_change(state_errors.back());
		state_errors.pop_back();
		s_input_errors.pop_back();

		if (this->is_scope) {
			vector<double> action_input_errors(this->action->num_outputs);
			for (int o_index = 0; o_index < this->action->num_outputs; o_index++) {
				action_input_errors[o_index] = this->obs_network->input->errors[o_index];
				this->obs_network->input->errors[o_index] = 0.0;
			}
			vector<double> action_output_errors;
			this->action->backprop_errors_with_no_weight_change(
				action_input_errors,
				action_output_errors,
				predicted_score,
				target_val);

			if (this->action->num_inputs > 0) {
				this->action_input_network->backprop_errors_with_no_weight_change(action_output_errors);
				for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
					state_errors.back()[st_index] += this->action_input_network->state_input->errors[st_index];
					this->action_input_network->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)s_input_errors.back().size(); st_index++) {
					s_input_errors.back()[st_index] += this->action_input_network->s_input_input->errors[st_index];
					this->action_input_network->s_input_input->errors[st_index] = 0.0;
				}
			}
		}
	}
}

void Node::get_scope_sizes(vector<int>& scope_sizes,
						   vector<int>& s_input_sizes) {
	if (this->new_layer_size > 0) {
		scope_sizes.push_back(this->new_layer_size);
		s_input_sizes.push_back(0);

		for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
			s_input_sizes[this->score_input_layer[i_index]+1] += this->score_input_sizes[i_index];
		}

		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					scope_sizes.pop_back();
					s_input_sizes.pop_back();
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
					s_input_sizes[this->input_layer[i_index]+1] += this->input_sizes[i_index];
				}

				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					scope_sizes.pop_back();
					s_input_sizes.pop_back();
				}
				scope_sizes.push_back(this->compress_new_size);
				s_input_sizes.push_back(0);
			}
		}
	}
}

void Node::save(ofstream& output_file) {
	output_file << this->id << endl;

	if (this->is_scope) {
		output_file << 1 << endl;

		ofstream action_save_file;
		action_save_file.open("saves/" + this->id + "_scope.txt");
		this->action->save(action_save_file);
		action_save_file.close();

		if (this->action->num_inputs > 0) {
			ofstream action_input_network_save_file;
			action_input_network_save_file.open("saves/nns/" + this->id + "_action_input.txt");
			this->action_input_network->save(action_input_network_save_file);
			action_input_network_save_file.close();
		}
	} else {
		output_file << 0 << endl;
	}

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
		for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
			output_file << this->compressed_s_input_sizes[s_index] << endl;
		}
	}
}
