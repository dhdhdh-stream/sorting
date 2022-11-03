#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"

using namespace std;

const double TARGET_MAX_UPDATE = 0.002;

BaseScope::BaseScope(int num_outputs) {
	this->type = SCOPE_TYPE_BASE;

	this->num_outputs = num_outputs;
}

BaseScope::~BaseScope() {
	// do nothing
}

Scope::Scope(vector<AbstractScope*> actions,
			 int num_inputs,
			 int num_outputs,
			 vector<vector<int>> input_sizes,
			 vector<vector<SmallNetwork*>> input_networks,
			 vector<int> scope_compressed_scope_sizes,
			 vector<int> scope_compressed_s_input_sizes,
			 vector<bool> is_simple_append,
			 vector<int> new_layer_sizes,
			 vector<Network*> obs_networks,
			 vector<vector<int>> outer_input_indexes,
			 vector<SmallNetwork*> score_networks,
			 vector<vector<int>> inner_input_sizes,
			 vector<vector<SmallNetwork*>> inner_input_networks,
			 vector<int> inner_compress_num_layers,
			 vector<int> inner_compress_new_sizes,
			 vector<Network*> inner_compression_networks,
			 vector<vector<int>> inner_compressed_scope_sizes,
			 vector<vector<int>> inner_compressed_s_input_sizes,
			 vector<int> end_compressed_scope_sizes,
			 vector<int> end_compressed_s_input_sizes) {
	this->type = SCOPE_TYPE_SCOPE;

	this->actions = actions;
	this->num_inputs = num_inputs;
	this->num_outputs = num_outputs;
	this->input_sizes = input_sizes;
	this->input_networks = input_networks;
	this->scope_compressed_scope_sizes = scope_compressed_scope_sizes;
	this->scope_compressed_s_input_sizes = scope_compressed_s_input_sizes;
	this->is_simple_append = is_simple_append;
	this->new_layer_sizes = new_layer_sizes;
	this->obs_networks = obs_networks;
	this->outer_input_indexes = outer_input_indexes;
	this->score_networks = score_networks;
	this->inner_input_sizes = inner_input_sizes;
	this->inner_input_networks = inner_input_networks;
	this->inner_compress_num_layers = inner_compress_num_layers;
	this->inner_compress_new_sizes = inner_compress_new_sizes;
	this->inner_compression_networks = inner_compression_networks;
	this->inner_compressed_scope_sizes = inner_compressed_scope_sizes;
	this->inner_compressed_s_input_sizes = inner_compressed_s_input_sizes;
	this->end_compressed_scope_sizes = end_compressed_scope_sizes;
	this->end_compressed_s_input_sizes = end_compressed_s_input_sizes;

	this->outputs.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		this->outputs.push_back(0.0);
	}
}

Scope::Scope(Scope* original) {
	this->type = SCOPE_TYPE_SCOPE;

	this->actions.reserve(original->actions.size());
	for (int a_index = 0; a_index < (int)original->actions.size(); a_index++) {
		if (original->actions[a_index]->type == SCOPE_TYPE_BASE) {
			this->actions.push_back(new BaseScope(((BaseScope*)original->actions[a_index])->num_outputs));
		} else {
			this->actions.push_back(new Scope((Scope*)original->actions[a_index]));
		}
	}

	this->num_inputs = original->num_inputs;
	this->num_outputs = original->num_outputs;

	this->input_sizes = original->input_sizes;
	this->input_networks.reserve(original->input_networks.size());
	for (int a_index = 0; a_index < (int)original->input_networks.size(); a_index++) {
		this->input_networks.push_back(vector<SmallNetwork*>(original->input_networks[a_index].size()));
		for (int i_index = 0; i_index < (int)original->input_networks[a_index].size(); i_index++) {
			this->input_networks[a_index][i_index] = new SmallNetwork(original->input_networks[a_index][i_index]);
		}
	}
	this->scope_compressed_scope_sizes = original->scope_compressed_scope_sizes;
	this->scope_compressed_s_input_sizes = original->scope_compressed_s_input_sizes;

	this->is_simple_append = original->is_simple_append;
	this->new_layer_sizes = original->new_layer_sizes;
	this->obs_networks.reserve(original->obs_networks.size());
	for (int a_index = 0; a_index < (int)original->obs_networks.size(); a_index++) {
		if (original->obs_networks[a_index] == NULL) {
			this->obs_networks.push_back(NULL);
		} else {
			this->obs_networks.push_back(new Network(original->obs_networks[a_index]));
		}
	}

	this->outer_input_indexes = original->outer_input_indexes;

	this->score_networks.reserve(original->score_networks.size());
	for (int a_index = 0; a_index < (int)original->score_networks.size(); a_index++) {
		if (original->score_networks[a_index] == NULL) {
			this->score_networks.push_back(NULL);
		} else {
			this->score_networks.push_back(new SmallNetwork(original->score_networks[a_index]));
		}
	}

	this->inner_input_sizes = original->inner_input_sizes;
	this->inner_input_networks.reserve(original->inner_input_networks.size());
	for (int a_index = 0; a_index < (int)original->inner_input_networks.size(); a_index++) {
		this->inner_input_networks.push_back(original->inner_input_networks[a_index]);
		for (int i_index = 0; i_index < (int)original->inner_input_networks[a_index].size(); i_index++) {
			this->inner_input_networks[a_index][i_index] = new SmallNetwork(original->inner_input_networks[a_index][i_index]);
		}
	}

	this->inner_compress_num_layers = original->inner_compress_num_layers;
	this->inner_compress_new_sizes = original->inner_compress_new_sizes;
	this->inner_compression_networks.reserve(original->inner_compression_networks.size());
	for (int a_index = 0; a_index < (int)original->inner_compression_networks.size(); a_index++) {
		if (original->inner_compression_networks[a_index] == NULL) {
			this->inner_compression_networks.push_back(NULL);
		} else {
			this->inner_compression_networks.push_back(new Network(original->inner_compression_networks[a_index]));
		}
	}
	this->inner_compressed_scope_sizes = original->inner_compressed_scope_sizes;
	this->inner_compressed_s_input_sizes = original->inner_compressed_s_input_sizes;

	this->end_compressed_scope_sizes = original->end_compressed_scope_sizes;
	this->end_compressed_s_input_sizes = original->end_compressed_s_input_sizes;

	this->outputs.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		this->outputs.push_back(0.0);
	}
}

Scope::Scope(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	boost::algorithm::trim(id_line);
	this->id = id_line;

	string num_actions_line;
	getline(input_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		string action_type_line;
		getline(input_file, action_type_line);
		int action_type = stoi(action_type_line);
		if (action_type == SCOPE_TYPE_BASE) {
			string num_outputs_line;
			getline(input_file, num_outputs_line);
			this->actions.push_back(new BaseScope(stoi(num_outputs_line)));
		} else {
			// action_type == SCOPE_TYPE_SCOPE
			string scope_id_line;
			getline(input_file, scope_id_line);
			boost::algorithm::trim(scope_id_line);
			string scope_id = scope_id_line;

			ifstream scope_save_file;
			scope_save_file.open("saves/" + scope_id + ".txt");
			this->actions.push_back(new Scope(scope_save_file));
			scope_save_file.close();
		}
	}

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	this->num_inputs = stoi(num_inputs_line);

	string num_outputs_line;
	getline(input_file, num_outputs_line);
	this->num_outputs = stoi(num_outputs_line);

	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->input_sizes.push_back(vector<int>());
		this->input_networks.push_back(vector<SmallNetwork*>());

		string num_input_networks_line;
		getline(input_file, num_input_networks_line);
		int num_input_networks = stoi(num_input_networks_line);
		for (int i_index = 0; i_index < num_input_networks; i_index++) {
			string input_size_line;
			getline(input_file, input_size_line);
			this->input_sizes[a_index].push_back(stoi(input_size_line));

			ifstream input_network_save_file;
			input_network_save_file.open("saves/nns/" + this->id + "_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
			this->input_networks[a_index].push_back(new SmallNetwork(input_network_save_file));
			input_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		string scope_compressed_scope_size_line;
		getline(input_file, scope_compressed_scope_size_line);
		this->scope_compressed_scope_sizes.push_back(stoi(scope_compressed_scope_size_line));
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		string scope_compressed_s_input_size_line;
		getline(input_file, scope_compressed_s_input_size_line);
		this->scope_compressed_s_input_sizes.push_back(stoi(scope_compressed_s_input_size_line));
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		string is_simple_append_line;
		getline(input_file, is_simple_append_line);
		this->is_simple_append.push_back(stoi(is_simple_append_line));
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		string new_layer_size_line;
		getline(input_file, new_layer_size_line);
		this->new_layer_sizes.push_back(stoi(new_layer_size_line));
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		if (this->new_layer_sizes[a_index] > 0) {
			ifstream obs_network_save_file;
			obs_network_save_file.open("saves/nns/" + this->id + "_obs_" + to_string(a_index) + ".txt");
			this->obs_networks.push_back(new Network(obs_network_save_file));
			obs_network_save_file.close();
		} else {
			this->obs_networks.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->outer_input_indexes.push_back(vector<int>());

		string num_outer_input_indexes_line;
		getline(input_file, num_outer_input_indexes_line);
		int num_outer_input_indexes = stoi(num_outer_input_indexes_line);
		for (int i_index = 0; i_index < num_outer_input_indexes; i_index++) {
			string outer_input_indexes_line;
			getline(input_file, outer_input_indexes_line);
			this->outer_input_indexes[a_index].push_back(stoi(outer_input_indexes_line));
		}
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE
				&& this->new_layer_sizes[a_index] > 0) {
			ifstream score_network_save_file;
			score_network_save_file.open("saves/nns/" + this->id + "_score_" + to_string(a_index) + ".txt");
			this->score_networks.push_back(new SmallNetwork(score_network_save_file));
			score_network_save_file.close();
		} else {
			this->score_networks.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->inner_input_sizes.push_back(vector<int>());
		this->inner_input_networks.push_back(vector<SmallNetwork*>());

		string num_inner_input_networks_line;
		getline(input_file, num_inner_input_networks_line);
		int num_inner_input_networks = stoi(num_inner_input_networks_line);
		for (int i_index = 0; i_index < num_inner_input_networks; i_index++) {
			string inner_input_size_line;
			getline(input_file, inner_input_size_line);
			this->inner_input_sizes[a_index].push_back(stoi(inner_input_size_line));

			ifstream inner_input_network_save_file;
			inner_input_network_save_file.open("saves/nns/" + this->id + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
			this->inner_input_networks[a_index].push_back(new SmallNetwork(inner_input_network_save_file));
			inner_input_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		string inner_compress_num_layers_line;
		getline(input_file, inner_compress_num_layers_line);
		this->inner_compress_num_layers.push_back(stoi(inner_compress_num_layers_line));
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		string inner_compress_new_size_line;
		getline(input_file, inner_compress_new_size_line);
		this->inner_compress_new_sizes.push_back(stoi(inner_compress_new_size_line));
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		if (this->inner_compress_new_sizes[a_index] > 0) {
			ifstream inner_compress_network_save_file;
			inner_compress_network_save_file.open("saves/nns/" + this->id + "_inner_compress_" + to_string(a_index) + ".txt");
			this->inner_compression_networks.push_back(new Network(inner_compress_network_save_file));
			inner_compress_network_save_file.close();
		} else {
			this->inner_compression_networks.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->inner_compressed_scope_sizes.push_back(vector<int>());
		this->inner_compressed_s_input_sizes.push_back(vector<int>());

		for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
			string inner_compressed_scope_size_line;
			getline(input_file, inner_compressed_scope_size_line);
			this->inner_compressed_scope_sizes[a_index].push_back(stoi(inner_compressed_scope_size_line));

			string inner_compressed_s_input_size_line;
			getline(input_file, inner_compressed_s_input_size_line);
			this->inner_compressed_s_input_sizes[a_index].push_back(stoi(inner_compressed_s_input_size_line));
		}
	}

	string num_end_compressed_line;
	getline(input_file, num_end_compressed_line);
	int num_end_compressed = stoi(num_end_compressed_line);
	for (int sc_index = 0; sc_index < num_end_compressed; sc_index++) {
		string end_compressed_scope_size_line;
		getline(input_file, end_compressed_scope_size_line);
		this->end_compressed_scope_sizes.push_back(stoi(end_compressed_scope_size_line));

		string end_compressed_s_input_size_line;
		getline(input_file, end_compressed_s_input_size_line);
		this->end_compressed_s_input_sizes.push_back(stoi(end_compressed_s_input_size_line));
	}
}

Scope::~Scope() {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		delete this->actions[a_index];
	}
	for (int a_index = 0; a_index < (int)this->input_networks.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
			delete this->input_networks[a_index][i_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->obs_networks.size(); a_index++) {
		if (this->obs_networks[a_index] != NULL) {
			delete this->obs_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->score_networks.size(); a_index++) {
		if (this->score_networks[a_index] != NULL) {
			delete this->score_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->inner_input_networks.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
			delete this->inner_input_networks[a_index][i_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->inner_compression_networks.size(); a_index++) {
		if (this->inner_compression_networks[a_index] != NULL) {
			delete this->inner_compression_networks[a_index];
		}
	}
	for (int a_index = 0; a_index < (int)this->zero_scopes.size(); a_index++) {
		if (this->zero_scopes[a_index] != NULL) {
			delete this->zero_scopes[a_index];
		}
	}
}

void Scope::activate(vector<vector<double>>& flat_vals,
					 vector<double> inputs,
					 double& predicted_score) {
	vector<vector<double>> local_state_vals;
	vector<vector<double>> local_s_input_vals;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->activate(flat_vals[0]);
				vector<double> new_scope(this->new_layer_sizes[a_index]);
				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
				}
				local_state_vals.push_back(new_scope);
				local_s_input_vals.push_back(vector<double>());

				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
					this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																		   local_s_input_vals[0]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				this->score_networks[a_index]->activate(local_state_vals.back(),
														local_s_input_vals.back());
				predicted_score += this->score_networks[a_index]->output->acti_vals[0];

				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						vector<double> compression_input;
						for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
							for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
								compression_input.push_back(local_state_vals[sc_index][st_index]);
							}
						}
						for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
							compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
						}
						this->inner_compression_networks[a_index]->activate(compression_input);
						vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
						for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
						}
						for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
						local_state_vals.push_back(new_scope);
						local_s_input_vals.push_back(vector<double>());
					} else {
						// obs used for score, then popped
						local_state_vals.pop_back();
						local_s_input_vals.pop_back();
					}
				}
			}
			flat_vals.erase(flat_vals.begin());
		} else {
			for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
				local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
			}

			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
				this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																 local_s_input_vals.back());
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			((Scope*)this->actions[a_index])->activate(flat_vals,
													   scope_input,
													   predicted_score);

			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
					local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				vector<double> compress_input;
				compress_input = local_state_vals.back();
				compress_input.insert(compress_input.end(),
					((Scope*)this->actions[a_index])->outputs.begin(),
					((Scope*)this->actions[a_index])->outputs.end());
				compress_input.insert(compress_input.end(),
					local_s_input_vals.back().begin(),
					local_s_input_vals.back().end());
				this->obs_networks[a_index]->activate(compress_input);
				vector<double> new_scope(this->new_layer_sizes[a_index]);
				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
				}
				local_state_vals.pop_back();
				local_state_vals.push_back(new_scope);
				local_s_input_vals.pop_back();
				local_s_input_vals.push_back(vector<double>());
			}
			// if this->new_layer_sizes[a_index] == 0 (i.e., compression_network is NULL), then no action needed
		}
	}

	int outputs_index = 0;
	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
			this->outputs[outputs_index] = local_state_vals[sc_index][st_index];
			outputs_index++;
		}
	}
}

void Scope::backprop(vector<double> input_errors,
					 vector<double>& output_errors,
					 double& predicted_score,
					 double target_val) {
	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	output_errors.reserve(this->num_inputs);
	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
		output_errors.push_back(0.0);
	}

	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

						local_state_errors.pop_back();
						local_s_input_errors.pop_back();
						int input_index = 0;
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
							for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
								local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}

							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
						for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
							local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
							input_index++;
						}
					} else {
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
					}
				}

				vector<double> score_errors{target_val - predicted_score};
				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
					local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
					this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
					local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
					this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
				}
				predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
						local_s_input_errors[1].pop_back();
					}
					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
					local_s_input_errors[0].pop_back();
				}

				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
				local_state_errors.pop_back();
				local_s_input_errors.pop_back();
			}
		} else {
			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
			vector<double> new_input_errors(scope_num_outputs);
			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
					local_state_errors.back().pop_back();
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
				local_state_errors.pop_back();
				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
					this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
				}
				local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
				local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
				int input_index = 0;
				for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
					local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
				for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
					local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
			}

			vector<double> new_output_errors;
			((Scope*)this->actions[a_index])->backprop(new_input_errors,
													   new_output_errors,
													   predicted_score,
													   target_val);

			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> input_errors(this->input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
					new_output_errors.pop_back();
				}
				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
					this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
				}
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
					local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
					this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
				}
			}

			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
				output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
				local_s_input_errors[0].pop_back();
			}
		}
	}
}

void Scope::backprop_errors_with_no_weight_change(vector<double> input_errors,
												  vector<double>& output_errors,
												  double& predicted_score,
												  double target_val) {
	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	output_errors.reserve(this->num_inputs);
	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
		output_errors.push_back(0.0);
	}

	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						this->inner_compression_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());

						local_state_errors.pop_back();
						local_s_input_errors.pop_back();
						int input_index = 0;
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
							for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
								local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}

							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
						for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
							local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
							input_index++;
						}
					} else {
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
					}
				}

				vector<double> score_errors{target_val - predicted_score};
				this->score_networks[a_index]->backprop_errors_with_no_weight_change(score_errors);
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
					local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
					this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
					local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
					this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
				}
				predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
						local_s_input_errors[1].pop_back();
					}
					this->inner_input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
					local_s_input_errors[0].pop_back();
				}

				// no backprop needed for obs_networks
				local_state_errors.pop_back();
				local_s_input_errors.pop_back();
			}
		} else {
			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
			vector<double> new_input_errors(scope_num_outputs);
			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
					local_state_errors.back().pop_back();
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());
				local_state_errors.pop_back();
				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
					this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
				}
				local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
				local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
				int input_index = 0;
				for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
					local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
				for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
					local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
			}

			vector<double> new_output_errors;
			((Scope*)this->actions[a_index])->backprop_errors_with_no_weight_change(
				new_input_errors,
				new_output_errors,
				predicted_score,
				target_val);

			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> input_errors(this->input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
					new_output_errors.pop_back();
				}
				this->input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(input_errors);
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
					this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
				}
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
					local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
					this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
				}
			}

			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
				output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
				local_s_input_errors[0].pop_back();
			}
		}
	}
}

void Scope::add_to_dictionary(vector<Scope*>& scope_dictionary) {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_SCOPE) {
			scope_dictionary.push_back(new Scope((Scope*)this->actions[a_index]));

			((Scope*)this->actions[a_index])->add_to_dictionary(scope_dictionary);
		}
	}
}

void Scope::backprop(vector<double> input_errors,
					 vector<double>& output_errors,
					 double predicted_score_error) {
	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	output_errors.reserve(this->num_inputs);
	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
		output_errors.push_back(0.0);
	}

	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

						local_state_errors.pop_back();
						local_s_input_errors.pop_back();
						int input_index = 0;
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
							for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
								local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}

							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
						for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
							local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
							input_index++;
						}
					} else {
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
					}
				}

				vector<double> score_errors{predicted_score_error};
				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
					local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
					this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
					local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
					this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
				}

				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
						local_s_input_errors[1].pop_back();
					}
					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
					local_s_input_errors[0].pop_back();
				}

				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
				local_state_errors.pop_back();
				local_s_input_errors.pop_back();
			}
		} else {
			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
			vector<double> new_input_errors(scope_num_outputs);
			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
					local_state_errors.back().pop_back();
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
				local_state_errors.pop_back();
				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
					this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
				}
				local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
				local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
				int input_index = 0;
				for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
					local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
				for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
					local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
			}

			vector<double> new_output_errors;
			((Scope*)this->actions[a_index])->backprop(new_input_errors,
													   new_output_errors,
													   predicted_score_error);

			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> input_errors(this->input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
					new_output_errors.pop_back();
				}
				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
					this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
				}
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
					local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
					this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
				}
			}

			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
				output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
				local_s_input_errors[0].pop_back();
			}
		}
	}
}

void Scope::backprop_errors_with_no_weight_change(vector<double> input_errors,
												  vector<double>& output_errors,
												  double predicted_score_error) {
	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	output_errors.reserve(this->num_inputs);
	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
		output_errors.push_back(0.0);
	}

	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			if (this->new_layer_sizes[a_index] > 0) {
				if (this->inner_compress_num_layers[a_index] > 0) {
					if (this->inner_compression_networks[a_index] != NULL) {
						this->inner_compression_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());

						local_state_errors.pop_back();
						local_s_input_errors.pop_back();
						int input_index = 0;
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
							for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
								local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}

							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
						for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
							local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
							input_index++;
						}
					} else {
						for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
							local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
							local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
						}
					}
				}

				vector<double> score_errors{predicted_score_error};
				this->score_networks[a_index]->backprop_errors_with_no_weight_change(score_errors);
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
					local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
					this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
				}
				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
					local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
					this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
				}

				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
						local_s_input_errors[1].pop_back();
					}
					this->inner_input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
					local_s_input_errors[0].pop_back();
				}

				// no backprop needed for obs_networks
				local_state_errors.pop_back();
				local_s_input_errors.pop_back();
			}
		} else {
			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
			vector<double> new_input_errors(scope_num_outputs);
			if (this->is_simple_append[a_index]) {
				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
					local_state_errors.back().pop_back();
				}
			} else if (this->new_layer_sizes[a_index] > 0) {
				this->obs_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());
				local_state_errors.pop_back();
				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
					this->obs_networks[a_index]->input->errors[
						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
				}
				local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
				local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
				int input_index = 0;
				for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
					local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
				for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
					local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
					this->obs_networks[a_index]->input->errors[input_index] = 0.0;
					input_index++;
				}
			}

			vector<double> new_output_errors;
			((Scope*)this->actions[a_index])->backprop_errors_with_no_weight_change(
				new_input_errors,
				new_output_errors,
				predicted_score_error);

			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> input_errors(this->input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
					new_output_errors.pop_back();
				}
				this->input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(input_errors);
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
					this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
				}
				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
					local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
					this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
				}
			}

			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
				output_errors[this->outer_input_indexes[a_index][i_index]] = local_s_input_errors[0].back();
				local_s_input_errors[0].pop_back();
			}
		}
	}
}

// void Scope::setup_zero_scopes() {
// 	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			this->zero_scopes.push_back(NULL);
// 		} else {
// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				Scope* new_scope = new Scope((Scope*)this->actions[a_index]);
// 				this->zero_scopes.push_back(new_scope);
// 			} else {
// 				this->zero_scopes.push_back(NULL);
// 			}

// 			((Scope*)this->actions[a_index])->setup_zero_scopes();
// 		}
// 	}
// }

// void Scope::zero_train_activate(vector<vector<double>>& flat_vals,
// 								vector<double> inputs,
// 								double& predicted_score) {
// 	vector<vector<double>> local_state_vals;
// 	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			if (this->new_layer_sizes[a_index] > 0) {
// 				this->obs_networks[a_index]->activate(flat_vals[0]);
// 				vector<double> new_scope(this->new_layer_sizes[a_index]);
// 				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 				}
// 				local_state_vals.push_back(new_scope);

// 				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 					local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 				}

// 				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
// 					this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0]);
// 					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 						local_state_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 					}
// 				}

// 				this->score_networks[a_index]->activate(local_state_vals.back());
// 				predicted_score += this->score_networks[a_index]->output->acti_vals[0];

// 				if (this->inner_compress_num_layers[a_index] > 0) {
// 					if (this->inner_compression_networks[a_index] != NULL) {
// 						vector<double> compression_input;
// 						for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
// 							for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 								compression_input.push_back(local_state_vals[sc_index][st_index]);
// 							}
// 						}
// 						this->inner_compression_networks[a_index]->activate(compression_input);
// 						vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
// 						for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
// 							new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
// 						}
// 						for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
// 							local_state_vals.pop_back();
// 						}
// 						local_state_vals.push_back(new_scope);
// 					} else {
// 						// obs used for score, then popped
// 						local_state_vals.pop_back();
// 					}
// 				}
// 			}
// 			flat_vals.erase(flat_vals.begin());
// 		} else {
// 			for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
// 				local_state_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
// 			}

// 			vector<double> scope_input;
// 			for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
// 				this->input_networks[a_index][i_index]->activate(local_state_vals.back());
// 				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 					scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
// 				}
// 			}

// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				vector<vector<double>> copy_flat_vals = flat_vals;
// 				double copy_predicted_score = predicted_score;	// though shouldn't matter
// 				vector<double> empty_scope_input(this->zero_scopes[a_index]->num_inputs, 0.0);
// 				this->zero_scopes[a_index]->activate(copy_flat_vals,
// 													 empty_scope_input,
// 													 copy_predicted_score);
// 			}

// 			((Scope*)this->actions[a_index])->zero_train_activate(flat_vals,
// 																  scope_input,
// 																  predicted_score);

// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				this->zero_scopes[a_index]->zero_train_backprop((Scope*)this->actions[a_index]);
// 			}

// 			if (this->is_simple_append[a_index]) {
// 				for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
// 					local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
// 				}
// 			} else if (this->new_layer_sizes[a_index] > 0) {
// 				vector<double> compress_input;
// 				compress_input = local_state_vals.back();
// 				compress_input.insert(compress_input.end(),
// 					((Scope*)this->actions[a_index])->outputs.begin(),
// 					((Scope*)this->actions[a_index])->outputs.end());
// 				this->obs_networks[a_index]->activate(compress_input);
// 				vector<double> new_scope(this->new_layer_sizes[a_index]);
// 				for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
// 					new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
// 				}
// 				local_state_vals.pop_back();
// 				local_state_vals.push_back(new_scope);
// 			}
// 		}
// 	}

// 	int outputs_index = 0;
// 	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
// 		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
// 			this->outputs[outputs_index] = local_state_vals[sc_index][st_index];
// 			outputs_index++;
// 		}
// 	}
// }

// void Scope::zero_train_backprop(Scope* original) {
// 	vector<vector<double>> local_state_errors;
// 	int input_errors_index = 0;
// 	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
// 		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
// 		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
// 			double error = original->outputs[input_errors_index] - this->outputs[input_errors_index];
// 			local_state_errors[sc_index][st_index] = error;
// 			input_errors_index++;
// 		}
// 	}

// 	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			if (this->new_layer_sizes[a_index] > 0) {
// 				if (this->inner_compression_networks[a_index] != NULL) {
// 					this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

// 					local_state_errors.pop_back();
// 					int input_index = 0;
// 					for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
// 						local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
// 						for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
// 							local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
// 							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
// 							input_index++;
// 						}
// 					}
// 				}

// 				vector<double> score_errors{original->score_networks[a_index]->output->acti_vals[0]
// 					- this->score_networks[a_index]->output->acti_vals[0]};
// 				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
// 				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->input->errors.size(); st_index++) {
// 					local_state_errors.back()[st_index] += this->score_networks[a_index]->input->errors[st_index];
// 					this->score_networks[a_index]->input->errors[st_index] = 0.0;
// 				}

// 				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
// 					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_state_errors[1].back();
// 						local_state_errors[1].pop_back();
// 					}
// 					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
// 					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->input->errors[in_index];
// 						this->inner_input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 					}
// 				}

// 				// ignore outer_input_indexes

// 				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 			}
// 		} else {
// 			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
// 			vector<double> new_input_errors(scope_num_outputs);
// 			if (this->is_simple_append[a_index]) {
// 				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
// 					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
// 					local_state_errors.back().pop_back();
// 				}
// 			} else if (this->new_layer_sizes[a_index] > 0) {
// 				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
// 					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
// 					this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
// 				}
// 				local_state_errors.push_back(vector<double>(
// 					this->obs_networks[a_index]->input->errors.size()-scope_num_outputs));
// 				for (int i_index = 0; i_index < (int)this->obs_networks[a_index]->input->errors.size()-scope_num_outputs; i_index++) {
// 					local_state_errors.back()[i_index] = this->obs_networks[a_index]->input->errors[i_index];
// 					this->obs_networks[a_index]->input->errors[i_index] = 0.0;
// 				}
// 			}

// 			vector<double> new_output_errors;
// 			((Scope*)this->actions[a_index])->zero_train_backprop(
// 				new_input_errors,
// 				new_output_errors,
// 				(Scope*)original->actions[a_index]);

// 			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 				vector<double> input_errors(this->input_sizes[a_index][i_index]);
// 				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
// 					new_output_errors.pop_back();
// 				}
// 				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
// 				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->input->errors[in_index];
// 					this->input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 				}
// 			}

// 			// ignore outer_input_indexes
// 		}
// 	}
// }

// void Scope::zero_train_backprop(vector<double> input_errors,
// 								std::vector<double>& output_errors,
// 								Scope* original) {
// 	vector<vector<double>> local_state_errors;
// 	int input_errors_index = 0;
// 	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
// 		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
// 		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
// 			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
// 			input_errors_index++;
// 		}
// 	}

// 	output_errors.reserve(this->num_inputs);
// 	for (int i_index = 0; i_index < this->num_inputs; i_index++) {
// 		output_errors.push_back(0.0);
// 	}

// 	for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
// 			if (this->new_layer_sizes[a_index] > 0) {
// 				if (this->inner_compression_networks[a_index] != NULL) {
// 					this->inner_compression_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);

// 					local_state_errors.pop_back();
// 					int input_index = 0;
// 					for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
// 						local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
// 						for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
// 							local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
// 							this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
// 							input_index++;
// 						}
// 					}
// 				}

// 				vector<double> score_errors{original->score_networks[a_index]->output->acti_vals[0]
// 					- this->score_networks[a_index]->output->acti_vals[0]};
// 				this->score_networks[a_index]->backprop(score_errors, TARGET_MAX_UPDATE);
// 				for (int st_index = 0; st_index < (int)this->score_networks[a_index]->input->errors.size(); st_index++) {
// 					local_state_errors.back()[st_index] += this->score_networks[a_index]->input->errors[st_index];
// 					this->score_networks[a_index]->input->errors[st_index] = 0.0;
// 				}

// 				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
// 					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_state_errors[1].back();
// 						local_state_errors[1].pop_back();
// 					}
// 					this->inner_input_networks[a_index][i_index]->backprop(inner_input_errors, TARGET_MAX_UPDATE);
// 					for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 						local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->input->errors[in_index];
// 						this->inner_input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 					}
// 				}

// 				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
// 					output_errors[this->outer_input_indexes[a_index][i_index]] = local_state_errors[0].back();
// 					local_state_errors[0].pop_back();
// 				}

// 				this->obs_networks[a_index]->backprop_weights_with_no_error_signal(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 			}
// 		} else {
// 			int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
// 			vector<double> new_input_errors(scope_num_outputs);
// 			if (this->is_simple_append[a_index]) {
// 				for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
// 					new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
// 					local_state_errors.back().pop_back();
// 				}
// 			} else if (this->new_layer_sizes[a_index] > 0) {
// 				this->obs_networks[a_index]->backprop(local_state_errors.back(), TARGET_MAX_UPDATE);
// 				local_state_errors.pop_back();
// 				for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
// 					new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
// 					this->obs_networks[a_index]->input->errors[
// 						this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
// 				}
// 				local_state_errors.push_back(vector<double>(
// 					this->obs_networks[a_index]->input->errors.size()-scope_num_outputs));
// 				for (int i_index = 0; i_index < (int)this->obs_networks[a_index]->input->errors.size()-scope_num_outputs; i_index++) {
// 					local_state_errors.back()[i_index] = this->obs_networks[a_index]->input->errors[i_index];
// 					this->obs_networks[a_index]->input->errors[i_index] = 0.0;
// 				}
// 			}

// 			vector<double> new_output_errors;
// 			((Scope*)this->actions[a_index])->zero_train_backprop(
// 				new_input_errors,
// 				new_output_errors,
// 				(Scope*)original->actions[a_index]);

// 			for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 				vector<double> input_errors(this->input_sizes[a_index][i_index]);
// 				for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
// 					input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
// 					new_output_errors.pop_back();
// 				}
// 				this->input_networks[a_index][i_index]->backprop(input_errors, TARGET_MAX_UPDATE);
// 				for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->input->errors.size(); in_index++) {
// 					local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->input->errors[in_index];
// 					this->input_networks[a_index][i_index]->input->errors[in_index] = 0.0;
// 				}
// 			}

// 			for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
// 				output_errors[this->outer_input_indexes[a_index][i_index]] = local_state_errors[0].back();
// 				local_state_errors[0].pop_back();
// 			}
// 		}
// 	}
// }

// void Scope::add_to_zero_train_dictionary(vector<Scope*>& scope_dictionary) {
// 	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
// 		if (this->actions[a_index]->type == SCOPE_TYPE_SCOPE) {
// 			if (((Scope*)this->actions[a_index])->num_inputs > 0) {
// 				scope_dictionary.push_back(new Scope(this->zero_scopes[a_index]));
// 			} else {
// 				scope_dictionary.push_back(new Scope((Scope*)this->actions[a_index]));
// 			}

// 			((Scope*)this->actions[a_index])->add_to_zero_train_dictionary(scope_dictionary);
// 		}
// 	}
// }

void Scope::branch_front(vector<vector<double>>& flat_vals,
						 vector<double> inputs,
						 vector<int> explore_path,
						 int explore_start_inclusive,
						 int explore_end_non_inclusive,
						 Network* front_network,
						 double target_val,
						 double target_max_update) {
	vector<vector<double>> local_state_vals;
	vector<vector<double>> local_s_input_vals;

	if (explore_path.size() == 0) {
		vector<double> new_input_vals = inputs;

		for (int a_index = 0; a_index < explore_start_inclusive; a_index++) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->activate(flat_vals[0]);
					vector<double> new_scope(this->new_layer_sizes[a_index]);
					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
					}
					local_state_vals.push_back(new_scope);
					local_s_input_vals.push_back(vector<double>());

					if (a_index == explore_start_inclusive-1) {
						for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
							new_input_vals.insert(new_input_vals.end(),
								local_state_vals[sc_index].begin(), local_state_vals[sc_index].end());
						}
					}

					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
						local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
					}

					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																			   local_s_input_vals[0]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
						}
					}

					// no need to activate score_networks

					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							vector<double> compression_input;
							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
									compression_input.push_back(local_state_vals[sc_index][st_index]);
								}
							}
							for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
								compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
							}
							this->inner_compression_networks[a_index]->activate(compression_input);
							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
							}
							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
								local_state_vals.pop_back();
								local_s_input_vals.pop_back();
							}
							local_state_vals.push_back(new_scope);
							local_s_input_vals.push_back(vector<double>());
						} else {
							// obs used for score, then popped
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
					}
				}
				flat_vals.erase(flat_vals.begin());
			} else {
				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				vector<double> scope_input;
				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
					this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																	 local_s_input_vals.back());
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				double empty_score = 0.0;
				((Scope*)this->actions[a_index])->activate(flat_vals,
														   scope_input,
														   empty_score);

				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
					}
				} else {
					if (a_index == explore_start_inclusive-1) {
						// add local_state_vals before compression
						new_input_vals.insert(new_input_vals.end(),
							local_state_vals.back().begin(), local_state_vals.back().end());
						new_input_vals.insert(new_input_vals.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
					}

					if (this->new_layer_sizes[a_index] > 0) {
						vector<double> compress_input;
						compress_input = local_state_vals.back();
						compress_input.insert(compress_input.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
						compress_input.insert(compress_input.end(),
							local_s_input_vals.back().begin(),
							local_s_input_vals.back().end());
						this->obs_networks[a_index]->activate(compress_input);
						vector<double> new_scope(this->new_layer_sizes[a_index]);
						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
						}
						local_state_vals.pop_back();
						local_state_vals.push_back(new_scope);
						local_s_input_vals.pop_back();
						local_s_input_vals.push_back(vector<double>());
					}
				}
			}
		}

		new_input_vals.insert(new_input_vals.end(),
			flat_vals[0].begin(), flat_vals[0].end());
		flat_vals.erase(flat_vals.begin());
		new_input_vals.insert(new_input_vals.end(),
			flat_vals[0].begin(), flat_vals[0].end());
		flat_vals.erase(flat_vals.begin());
		new_input_vals.insert(new_input_vals.end(),
			flat_vals[0].begin(), flat_vals[0].end());
		flat_vals.erase(flat_vals.begin());

		front_network->activate(new_input_vals);

		vector<double> errors;
		errors.push_back(target_val - front_network->output->acti_vals[0]);
		global_sum_error += abs(errors[0]);

		front_network->backprop(errors, target_max_update);
	} else {
		for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->activate(flat_vals[0]);
					vector<double> new_scope(this->new_layer_sizes[a_index]);
					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
					}
					local_state_vals.push_back(new_scope);
					local_s_input_vals.push_back(vector<double>());

					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
						local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
					}

					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																			   local_s_input_vals[0]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
						}
					}

					// no need to activate score_networks

					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							vector<double> compression_input;
							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
									compression_input.push_back(local_state_vals[sc_index][st_index]);
								}
							}
							for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
								compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
							}
							this->inner_compression_networks[a_index]->activate(compression_input);
							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
							}
							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
								local_state_vals.pop_back();
								local_s_input_vals.pop_back();
							}
							local_state_vals.push_back(new_scope);
							local_s_input_vals.push_back(vector<double>());
						} else {
							// obs used for score, then popped
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
					}
				}
				flat_vals.erase(flat_vals.begin());
			} else {
				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				vector<double> scope_input;
				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
					this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																	 local_s_input_vals.back());
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				if (explore_path[0] == a_index) {
					// don't use outer state for branch

					vector<int> trimmed_explore_path(explore_path.begin()+1, explore_path.end());
					((Scope*)this->actions[a_index])->branch_front(
						flat_vals,
						scope_input,
						trimmed_explore_path,
						explore_start_inclusive,
						explore_end_non_inclusive,
						front_network,
						target_val,
						target_max_update);

					// no need to process beyond branch
					return;
				} else {
					double empty_score = 0.0;
					((Scope*)this->actions[a_index])->activate(flat_vals,
															   scope_input,
															   empty_score);
				}

				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
					}
				} else {
					if (this->new_layer_sizes[a_index] > 0) {
						vector<double> compress_input;
						compress_input = local_state_vals.back();
						compress_input.insert(compress_input.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
						compress_input.insert(compress_input.end(),
							local_s_input_vals.back().begin(),
							local_s_input_vals.back().end());
						this->obs_networks[a_index]->activate(compress_input);
						vector<double> new_scope(this->new_layer_sizes[a_index]);
						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
						}
						local_state_vals.pop_back();
						local_state_vals.push_back(new_scope);
						local_s_input_vals.pop_back();
						local_s_input_vals.push_back(vector<double>());
					}
				}
			}
		}
	}

	// no need to set output
}

void Scope::branch_combine_activate(vector<vector<double>>& flat_vals,
									vector<double> inputs,
									vector<int> explore_path,
									int explore_start_inclusive,
									int explore_end_non_inclusive,
									Network* front_network,
									Network* back_network,
									double& front_predicted_score,
									double& back_predicted_score) {
	vector<vector<double>> local_state_vals;
	vector<vector<double>> local_s_input_vals;

	if (explore_path.size() == 0) {
		vector<double> new_input_vals = inputs;

		for (int a_index = 0; a_index < explore_start_inclusive; a_index++) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->activate(flat_vals[0]);
					vector<double> new_scope(this->new_layer_sizes[a_index]);
					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
					}
					local_state_vals.push_back(new_scope);
					local_s_input_vals.push_back(vector<double>());

					if (a_index == explore_start_inclusive-1) {
						for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
							new_input_vals.insert(new_input_vals.end(),
								local_state_vals[sc_index].begin(), local_state_vals[sc_index].end());
						}
					}

					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
						local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
					}

					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																			   local_s_input_vals[0]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
						}
					}

					// no need to activate score_networks

					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							vector<double> compression_input;
							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
									compression_input.push_back(local_state_vals[sc_index][st_index]);
								}
							}
							for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
								compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
							}
							this->inner_compression_networks[a_index]->activate(compression_input);
							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
							}
							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
								local_state_vals.pop_back();
								local_s_input_vals.pop_back();
							}
							local_state_vals.push_back(new_scope);
							local_s_input_vals.push_back(vector<double>());
						} else {
							// obs used for score, then popped
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
					}
				}
				flat_vals.erase(flat_vals.begin());
			} else {
				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				vector<double> scope_input;
				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
					this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																	 local_s_input_vals.back());
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				double empty_score = 0.0;
				((Scope*)this->actions[a_index])->activate(flat_vals,
														   scope_input,
														   empty_score);

				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
					}
				} else {
					if (a_index == explore_start_inclusive-1) {
						// add local_state_vals before compression
						new_input_vals.insert(new_input_vals.end(),
							local_state_vals.back().begin(), local_state_vals.back().end());
						new_input_vals.insert(new_input_vals.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
					}

					if (this->new_layer_sizes[a_index] > 0) {
						vector<double> compress_input;
						compress_input = local_state_vals.back();
						compress_input.insert(compress_input.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
						compress_input.insert(compress_input.end(),
							local_s_input_vals.back().begin(),
							local_s_input_vals.back().end());
						this->obs_networks[a_index]->activate(compress_input);
						vector<double> new_scope(this->new_layer_sizes[a_index]);
						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
						}
						local_state_vals.pop_back();
						local_state_vals.push_back(new_scope);
						local_s_input_vals.pop_back();
						local_s_input_vals.push_back(vector<double>());
					}
				}
			}
		}

		new_input_vals.insert(new_input_vals.end(),
			flat_vals[0].begin(), flat_vals[0].end());
		flat_vals.erase(flat_vals.begin());
		new_input_vals.insert(new_input_vals.end(),
			flat_vals[0].begin(), flat_vals[0].end());
		flat_vals.erase(flat_vals.begin());
		new_input_vals.insert(new_input_vals.end(),
			flat_vals[0].begin(), flat_vals[0].end());
		flat_vals.erase(flat_vals.begin());

		front_network->activate(new_input_vals);
		front_predicted_score = front_network->output->acti_vals[0];
		back_predicted_score = 0.0;

		back_network->activate(new_input_vals);

		local_state_vals.pop_back();
		local_state_vals.push_back(vector<double>(1));
		local_state_vals.back()[0] = back_network->output->acti_vals[0];

		for (int a_index = explore_end_non_inclusive; a_index < (int)this->actions.size(); a_index++) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->activate(flat_vals[0]);
					vector<double> new_scope(this->new_layer_sizes[a_index]);
					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
					}
					local_state_vals.push_back(new_scope);
					local_s_input_vals.push_back(vector<double>());

					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
						local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
					}

					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																			   local_s_input_vals[0]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
						}
					}

					this->score_networks[a_index]->activate(local_state_vals.back(),
															local_s_input_vals.back());
					back_predicted_score += this->score_networks[a_index]->output->acti_vals[0];

					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							vector<double> compression_input;
							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
									compression_input.push_back(local_state_vals[sc_index][st_index]);
								}
							}
							for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
								compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
							}
							this->inner_compression_networks[a_index]->activate(compression_input);
							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
							}
							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
								local_state_vals.pop_back();
								local_s_input_vals.pop_back();
							}
							local_state_vals.push_back(new_scope);
							local_s_input_vals.push_back(vector<double>());
						} else {
							// obs used for score, then popped
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
					}
				}
				flat_vals.erase(flat_vals.begin());
			} else {
				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				vector<double> scope_input;
				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
					this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																	 local_s_input_vals.back());
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				((Scope*)this->actions[a_index])->activate(flat_vals,
														   scope_input,
														   back_predicted_score);

				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
					}
				} else {
					if (this->new_layer_sizes[a_index] > 0) {
						vector<double> compress_input;
						compress_input = local_state_vals.back();
						compress_input.insert(compress_input.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
						compress_input.insert(compress_input.end(),
							local_s_input_vals.back().begin(),
							local_s_input_vals.back().end());
						this->obs_networks[a_index]->activate(compress_input);
						vector<double> new_scope(this->new_layer_sizes[a_index]);
						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
						}
						local_state_vals.pop_back();
						local_state_vals.push_back(new_scope);
						local_s_input_vals.pop_back();
						local_s_input_vals.push_back(vector<double>());
					}
				}
			}
		}
	} else {
		for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->activate(flat_vals[0]);
					vector<double> new_scope(this->new_layer_sizes[a_index]);
					for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
						new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
					}
					local_state_vals.push_back(new_scope);
					local_s_input_vals.push_back(vector<double>());

					for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
						local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
					}

					for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
						this->inner_input_networks[a_index][i_index]->activate(local_state_vals[0],
																			   local_s_input_vals[0]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							local_s_input_vals[1].push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
						}
					}

					// predicted_score will be reset, so no need to special case
					this->score_networks[a_index]->activate(local_state_vals.back(),
															local_s_input_vals.back());
					back_predicted_score += this->score_networks[a_index]->output->acti_vals[0];

					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							vector<double> compression_input;
							for (int sc_index = (int)local_state_vals.size()-this->inner_compress_num_layers[a_index]; sc_index < (int)local_state_vals.size(); sc_index++) {
								for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
									compression_input.push_back(local_state_vals[sc_index][st_index]);
								}
							}
							for (int st_index = 0; st_index < (int)local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]].size(); st_index++) {
								compression_input.push_back(local_s_input_vals[local_state_vals.size()-this->inner_compress_num_layers[a_index]][st_index]);
							}
							this->inner_compression_networks[a_index]->activate(compression_input);
							vector<double> new_scope(this->inner_compress_new_sizes[a_index]);
							for (int s_index = 0; s_index < this->inner_compress_new_sizes[a_index]; s_index++) {
								new_scope[s_index] = this->inner_compression_networks[a_index]->output->acti_vals[s_index];
							}
							for (int c_index = 0; c_index < this->inner_compress_num_layers[a_index]; c_index++) {
								local_state_vals.pop_back();
								local_s_input_vals.pop_back();
							}
							local_state_vals.push_back(new_scope);
							local_s_input_vals.push_back(vector<double>());
						} else {
							// obs used for score, then popped
							local_state_vals.pop_back();
							local_s_input_vals.pop_back();
						}
					}
				}
				flat_vals.erase(flat_vals.begin());
			} else {
				for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
					local_s_input_vals[0].push_back(inputs[this->outer_input_indexes[a_index][i_index]]);
				}

				vector<double> scope_input;
				for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
					this->input_networks[a_index][i_index]->activate(local_state_vals.back(),
																	 local_s_input_vals.back());
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
				}

				if (explore_path[0] == a_index) {
					// don't use outer state for branch

					vector<int> trimmed_explore_path(explore_path.begin()+1, explore_path.end());
					((Scope*)this->actions[a_index])->branch_combine_activate(
						flat_vals,
						scope_input,
						trimmed_explore_path,
						explore_start_inclusive,
						explore_end_non_inclusive,
						front_network,
						back_network,
						front_predicted_score,
						back_predicted_score);
				} else {
					((Scope*)this->actions[a_index])->activate(flat_vals,
															   scope_input,
															   back_predicted_score);
				}

				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < ((Scope*)this->actions[a_index])->num_outputs; o_index++) {
						local_state_vals.back().push_back(((Scope*)this->actions[a_index])->outputs[o_index]);
					}
				} else {
					if (this->new_layer_sizes[a_index] > 0) {
						vector<double> compress_input;
						compress_input = local_state_vals.back();
						compress_input.insert(compress_input.end(),
							((Scope*)this->actions[a_index])->outputs.begin(),
							((Scope*)this->actions[a_index])->outputs.end());
						compress_input.insert(compress_input.end(),
							local_s_input_vals.back().begin(),
							local_s_input_vals.back().end());
						this->obs_networks[a_index]->activate(compress_input);
						vector<double> new_scope(this->new_layer_sizes[a_index]);
						for (int s_index = 0; s_index < this->new_layer_sizes[a_index]; s_index++) {
							new_scope[s_index] = this->obs_networks[a_index]->output->acti_vals[s_index];
						}
						local_state_vals.pop_back();
						local_state_vals.push_back(new_scope);
						local_s_input_vals.pop_back();
						local_s_input_vals.push_back(vector<double>());
					}
				}
			}
		}
	}

	int outputs_index = 0;
	for (int sc_index = 0; sc_index < (int)local_state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)local_state_vals[sc_index].size(); st_index++) {
			this->outputs[outputs_index] = local_state_vals[sc_index][st_index];
			outputs_index++;
		}
	}
}

void Scope::branch_back_backprop(vector<double> input_errors,
								 double& predicted_score,
								 double target_val,
								 vector<int> explore_path,
								 int explore_start_inclusive,
								 int explore_end_non_inclusive,
								 Network* back_network,
								 double target_max_update) {
	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	if (explore_path.size() == 0) {
		for (int a_index = (int)this->actions.size()-1; a_index >= explore_end_non_inclusive; a_index--) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							this->inner_compression_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());

							local_state_errors.pop_back();
							local_s_input_errors.pop_back();
							int input_index = 0;
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
								for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
									local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
									this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
									input_index++;
								}

								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
							for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
								local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}
						} else {
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
						}
					}

					vector<double> score_errors{target_val - predicted_score};
					this->score_networks[a_index]->backprop_errors_with_no_weight_change(score_errors);

					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
						local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
						this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
					}
					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
						local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
						this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
					}
					predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

					for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
						vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
							local_s_input_errors[1].pop_back();
						}
						this->inner_input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
							local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
						}
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
							local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
						}
					}

					for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
						// no need to set output_errors
						local_s_input_errors[0].pop_back();
					}

					// no backprop needed for obs_networks
					local_state_errors.pop_back();
					local_s_input_errors.pop_back();
				}
			} else {
				int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
				vector<double> new_input_errors(scope_num_outputs);
				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
						new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
						local_state_errors.back().pop_back();
					}
				} else if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());
					local_state_errors.pop_back();
					for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
						new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
						this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
					}
					local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
					local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
					int input_index = 0;
					for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
						local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
					for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
						local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
				}

				vector<double> new_output_errors;
				((Scope*)this->actions[a_index])->backprop_errors_with_no_weight_change(
					new_input_errors,
					new_output_errors,
					predicted_score,
					target_val);

				for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> input_errors(this->input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
						new_output_errors.pop_back();
					}
					this->input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(input_errors);
					for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
						this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					// no need to set output_errors
					local_s_input_errors[0].pop_back();
				}
			}
		}

		vector<double> back_errors;
		back_errors.push_back(local_state_errors.back()[0]);
		back_network->backprop(back_errors, target_max_update);
	} else {
		for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							this->inner_compression_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());

							local_state_errors.pop_back();
							local_s_input_errors.pop_back();
							int input_index = 0;
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
								for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
									local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
									this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
									input_index++;
								}

								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
							for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
								local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}
						} else {
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
						}
					}

					vector<double> score_errors{target_val - predicted_score};
					this->score_networks[a_index]->backprop_errors_with_no_weight_change(score_errors);

					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
						local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
						this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
					}
					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
						local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
						this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
					}
					predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

					for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
						vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
							local_s_input_errors[1].pop_back();
						}
						this->inner_input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
							local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
						}
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
							local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
						}
					}

					for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
						// no need to set output_errors
						local_s_input_errors[0].pop_back();
					}

					// no backprop needed for obs_networks
					local_state_errors.pop_back();
					local_s_input_errors.pop_back();
				}
			} else {
				int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
				vector<double> new_input_errors(scope_num_outputs);
				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
						new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
						local_state_errors.back().pop_back();
					}
				} else if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());
					local_state_errors.pop_back();
					for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
						new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
						this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
					}
					local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
					local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
					int input_index = 0;
					for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
						local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
					for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
						local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
				}

				if (explore_path[0] == a_index) {
					vector<int> trimmed_explore_path(explore_path.begin()+1, explore_path.end());
					((Scope*)this->actions[a_index])->branch_back_backprop(
						new_input_errors,
						predicted_score,
						target_val,
						trimmed_explore_path,
						explore_start_inclusive,
						explore_end_non_inclusive,
						back_network,
						target_max_update);

					// no need to process beyond branch
					return;
				} else {
					vector<double> new_output_errors;
					((Scope*)this->actions[a_index])->backprop_errors_with_no_weight_change(
						new_input_errors,
						new_output_errors,
						predicted_score,
						target_val);

					for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
						vector<double> input_errors(this->input_sizes[a_index][i_index]);
						for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
							input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
							new_output_errors.pop_back();
						}
						this->input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(input_errors);
						for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
							local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
							this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
						}
						for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
							local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
							this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
						}
					}

					for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
						// no need to set output_errors
						local_s_input_errors[0].pop_back();
					}
				}
			}
		}
	}
}

void Scope::branch_combine_backprop(vector<double> input_errors,
									double& predicted_score,
									double target_val,
									vector<int> explore_path,
									int explore_start_inclusive,
									int explore_end_non_inclusive,
									Network* front_network,
									double front_error,
									Network* back_network,
									double target_max_update) {
	vector<vector<double>> local_state_errors;
	int input_errors_index = 0;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		local_state_errors.push_back(vector<double>(this->end_compressed_scope_sizes[sc_index]));
		for (int st_index = 0; st_index < (int)this->end_compressed_scope_sizes[sc_index]; st_index++) {
			local_state_errors[sc_index][st_index] = input_errors[input_errors_index];
			input_errors_index++;
		}
	}
	vector<vector<double>> local_s_input_errors;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_s_input_sizes.size(); sc_index++) {
		local_s_input_errors.push_back(vector<double>(this->end_compressed_s_input_sizes[sc_index], 0.0));
	}

	if (explore_path.size() == 0) {
		for (int a_index = (int)this->actions.size()-1; a_index >= explore_end_non_inclusive; a_index--) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							this->inner_compression_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());

							local_state_errors.pop_back();
							local_s_input_errors.pop_back();
							int input_index = 0;
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
								for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
									local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
									this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
									input_index++;
								}

								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
							for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
								local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}
						} else {
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
						}
					}

					vector<double> score_errors{target_val - predicted_score};
					this->score_networks[a_index]->backprop_errors_with_no_weight_change(score_errors);

					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
						local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
						this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
					}
					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
						local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
						this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
					}
					predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

					for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
						vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
							local_s_input_errors[1].pop_back();
						}
						this->inner_input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
							local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
						}
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
							local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
						}
					}

					for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
						// no need to set output_errors
						local_s_input_errors[0].pop_back();
					}

					// no backprop needed for obs_networks
					local_state_errors.pop_back();
					local_s_input_errors.pop_back();
				}
			} else {
				int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
				vector<double> new_input_errors(scope_num_outputs);
				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
						new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
						local_state_errors.back().pop_back();
					}
				} else if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());
					local_state_errors.pop_back();
					for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
						new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
						this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
					}
					local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
					local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
					int input_index = 0;
					for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
						local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
					for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
						local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
				}

				vector<double> new_output_errors;
				((Scope*)this->actions[a_index])->backprop_errors_with_no_weight_change(
					new_input_errors,
					new_output_errors,
					predicted_score,
					target_val);

				for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> input_errors(this->input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
						input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
						new_output_errors.pop_back();
					}
					this->input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(input_errors);
					for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
						local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
						this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
					}
					for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
						local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
						this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
					}
				}

				for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
					// no need to set output_errors
					local_s_input_errors[0].pop_back();
				}
			}
		}

		vector<double> back_errors;
		back_errors.push_back(local_state_errors.back()[0]);
		back_network->backprop(back_errors, target_max_update);

		vector<double> front_errors{front_error};
		front_network->backprop(front_errors, target_max_update);
	} else {
		for (int a_index = (int)this->actions.size()-1; a_index >= 0; a_index--) {
			if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
				if (this->new_layer_sizes[a_index] > 0) {
					if (this->inner_compress_num_layers[a_index] > 0) {
						if (this->inner_compression_networks[a_index] != NULL) {
							this->inner_compression_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());

							local_state_errors.pop_back();
							local_s_input_errors.pop_back();
							int input_index = 0;
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index]));
								for (int st_index = 0; st_index < this->inner_compressed_scope_sizes[a_index][sc_index]; st_index++) {
									local_state_errors[sc_index][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
									this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
									input_index++;
								}

								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
							for (int st_index = 0; st_index < this->inner_compressed_s_input_sizes[a_index][0]; st_index++) {
								local_s_input_errors[0][st_index] = this->inner_compression_networks[a_index]->input->errors[input_index];
								this->inner_compression_networks[a_index]->input->errors[input_index] = 0.0;
								input_index++;
							}
						} else {
							for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
								local_state_errors.push_back(vector<double>(this->inner_compressed_scope_sizes[a_index][sc_index], 0.0));
								local_s_input_errors.push_back(vector<double>(this->inner_compressed_s_input_sizes[a_index][sc_index], 0.0));
							}
						}
					}

					vector<double> score_errors{target_val - predicted_score};
					this->score_networks[a_index]->backprop_errors_with_no_weight_change(score_errors);

					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->state_input->errors.size(); st_index++) {
						local_state_errors.back()[st_index] += this->score_networks[a_index]->state_input->errors[st_index];
						this->score_networks[a_index]->state_input->errors[st_index] = 0.0;
					}
					for (int st_index = 0; st_index < (int)this->score_networks[a_index]->s_input_input->errors.size(); st_index++) {
						local_s_input_errors.back()[st_index] += this->score_networks[a_index]->s_input_input->errors[st_index];
						this->score_networks[a_index]->s_input_input->errors[st_index] = 0.0;
					}
					predicted_score -= this->score_networks[a_index]->output->acti_vals[0];

					for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
						vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
						for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
							inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = local_s_input_errors[1].back();
							local_s_input_errors[1].pop_back();
						}
						this->inner_input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
							local_state_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->state_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
						}
						for (int in_index = 0; in_index < (int)this->inner_input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
							local_s_input_errors[0][in_index] += this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index];
							this->inner_input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
						}
					}

					for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
						// no need to set output_errors
						local_s_input_errors[0].pop_back();
					}

					// no backprop needed for obs_networks
					local_state_errors.pop_back();
					local_s_input_errors.pop_back();
				}
			} else {
				int scope_num_outputs = ((Scope*)this->actions[a_index])->num_outputs;
				vector<double> new_input_errors(scope_num_outputs);
				if (this->is_simple_append[a_index]) {
					for (int o_index = 0; o_index < scope_num_outputs; o_index++) {
						new_input_errors[scope_num_outputs-1-o_index] = local_state_errors.back().back();
						local_state_errors.back().pop_back();
					}
				} else if (this->new_layer_sizes[a_index] > 0) {
					this->obs_networks[a_index]->backprop_errors_with_no_weight_change(local_state_errors.back());
					local_state_errors.pop_back();
					for (int i_index = 0; i_index < scope_num_outputs; i_index++) {
						new_input_errors[i_index] = this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index];
						this->obs_networks[a_index]->input->errors[
							this->obs_networks[a_index]->input->errors.size()-scope_num_outputs+i_index] = 0.0;
					}
					local_state_errors.push_back(vector<double>(this->scope_compressed_scope_sizes[a_index]));
					local_s_input_errors.push_back(vector<double>(this->scope_compressed_s_input_sizes[a_index]));
					int input_index = 0;
					for (int st_index = 0; st_index < this->scope_compressed_scope_sizes[a_index]; st_index++) {
						local_state_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
					for (int st_index = 0; st_index < this->scope_compressed_s_input_sizes[a_index]; st_index++) {
						local_s_input_errors.back()[st_index] = this->obs_networks[a_index]->input->errors[input_index];
						this->obs_networks[a_index]->input->errors[input_index] = 0.0;
						input_index++;
					}
				}

				if (explore_path[0] == a_index) {
					vector<int> trimmed_explore_path(explore_path.begin()+1, explore_path.end());
					((Scope*)this->actions[a_index])->branch_combine_backprop(
						new_input_errors,
						predicted_score,
						target_val,
						trimmed_explore_path,
						explore_start_inclusive,
						explore_end_non_inclusive,
						front_network,
						front_error,
						back_network,
						target_max_update);

					// no need to process beyond branch
					return;
				} else {
					vector<double> new_output_errors;
					((Scope*)this->actions[a_index])->backprop_errors_with_no_weight_change(
						new_input_errors,
						new_output_errors,
						predicted_score,
						target_val);

					for (int i_index = (int)this->input_networks[a_index].size()-1; i_index >= 0; i_index--) {
						vector<double> input_errors(this->input_sizes[a_index][i_index]);
						for (int s_index = 0; s_index < this->input_sizes[a_index][i_index]; s_index++) {
							input_errors[this->input_sizes[a_index][i_index]-1-s_index] = new_output_errors.back();
							new_output_errors.pop_back();
						}
						this->input_networks[a_index][i_index]->backprop_errors_with_no_weight_change(input_errors);
						for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->state_input->errors.size(); in_index++) {
							local_state_errors[0][in_index] += this->input_networks[a_index][i_index]->state_input->errors[in_index];
							this->input_networks[a_index][i_index]->state_input->errors[in_index] = 0.0;
						}
						for (int in_index = 0; in_index < (int)this->input_networks[a_index][i_index]->s_input_input->errors.size(); in_index++) {
							local_s_input_errors[0][in_index] += this->input_networks[a_index][i_index]->s_input_input->errors[in_index];
							this->input_networks[a_index][i_index]->s_input_input->errors[in_index] = 0.0;
						}
					}

					for (int i_index = (int)this->outer_input_indexes[a_index].size()-1; i_index >= 0; i_index--) {
						// no need to set output_errors
						local_s_input_errors[0].pop_back();
					}
				}
			}
		}
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->actions.size() << endl;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		output_file << this->actions[a_index]->type << endl;
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE) {
			BaseScope* base_scope_action = (BaseScope*)this->actions[a_index];
			output_file << base_scope_action->num_outputs << endl;
		} else {
			// this->actions[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_action = (Scope*)this->actions[a_index];
			output_file << scope_action->id << endl;

			ofstream scope_save_file;
			scope_save_file.open("saves/" + scope_action->id + ".txt");
			scope_action->save(scope_save_file);
			scope_save_file.close();
		}
	}

	output_file << this->num_inputs << endl;
	output_file << this->num_outputs << endl;

	for (int a_index = 0; a_index < (int)this->input_networks.size(); a_index++) {
		output_file << this->input_networks[a_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->input_networks[a_index].size(); i_index++) {
			output_file << this->input_sizes[a_index][i_index] << endl;

			ofstream input_network_save_file;
			input_network_save_file.open("saves/nns/" + this->id + "_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
			this->input_networks[a_index][i_index]->save(input_network_save_file);
			input_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < (int)this->scope_compressed_scope_sizes.size(); a_index++) {
		output_file << this->scope_compressed_scope_sizes[a_index] << endl;
	}

	for (int a_index = 0; a_index < (int)this->scope_compressed_s_input_sizes.size(); a_index++) {
		output_file << this->scope_compressed_s_input_sizes[a_index] << endl;
	}

	for (int a_index = 0; a_index < (int)this->is_simple_append.size(); a_index++) {
		if (this->is_simple_append[a_index]) {
			output_file << 1 << endl;
		} else {
			output_file << 0 << endl;
		}
	}

	for (int a_index = 0; a_index < (int)this->new_layer_sizes.size(); a_index++) {
		output_file << this->new_layer_sizes[a_index] << endl;
	}

	for (int a_index = 0; a_index < (int)this->obs_networks.size(); a_index++) {
		if (this->new_layer_sizes[a_index] > 0) {
			ofstream obs_network_save_file;
			obs_network_save_file.open("saves/nns/" + this->id + "_obs_" + to_string(a_index) + ".txt");
			this->obs_networks[a_index]->save(obs_network_save_file);
			obs_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < (int)this->outer_input_indexes.size(); a_index++) {
		output_file << this->outer_input_indexes[a_index].size() << endl;
		for (int i_index = 0; i_index < (int)this->outer_input_indexes[a_index].size(); i_index++) {
			output_file << this->outer_input_indexes[a_index][i_index] << endl;
		}
	}

	for (int a_index = 0; a_index < (int)this->score_networks.size(); a_index++) {
		if (this->actions[a_index]->type == SCOPE_TYPE_BASE
				&& this->new_layer_sizes[a_index] > 0) {
			ofstream score_network_save_file;
			score_network_save_file.open("saves/nns/" + this->id + "_score_" + to_string(a_index) + ".txt");
			this->score_networks[a_index]->save(score_network_save_file);
			score_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < (int)this->inner_input_networks.size(); a_index++) {
		output_file << this->inner_input_networks[a_index].size() << endl;

		for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
			output_file << this->inner_input_sizes[a_index][i_index] << endl;

			ofstream inner_input_network_save_file;
			inner_input_network_save_file.open("saves/nns/" + this->id + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
			this->inner_input_networks[a_index][i_index]->save(inner_input_network_save_file);
			inner_input_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < (int)this->inner_compress_num_layers.size(); a_index++) {
		output_file << this->inner_compress_num_layers[a_index] << endl;
	}

	for (int a_index = 0; a_index < (int)this->inner_compress_new_sizes.size(); a_index++) {
		output_file << this->inner_compress_new_sizes[a_index] << endl;
	}

	for (int a_index = 0; a_index < (int)this->inner_compression_networks.size(); a_index++) {
		if (this->inner_compress_new_sizes[a_index] > 0) {
			ofstream inner_compress_network_save_file;
			inner_compress_network_save_file.open("saves/nns/" + this->id + "_inner_compress_" + to_string(a_index) + ".txt");
			this->inner_compression_networks[a_index]->save(inner_compress_network_save_file);
			inner_compress_network_save_file.close();
		}
	}

	for (int a_index = 0; a_index < (int)this->inner_compressed_scope_sizes.size(); a_index++) {
		for (int sc_index = 0; sc_index < this->inner_compress_num_layers[a_index]; sc_index++) {
			output_file << this->inner_compressed_scope_sizes[a_index][sc_index] << endl;
			output_file << this->inner_compressed_s_input_sizes[a_index][sc_index] << endl;
		}
	}

	output_file << this->end_compressed_scope_sizes.size() << endl;
	for (int sc_index = 0; sc_index < (int)this->end_compressed_scope_sizes.size(); sc_index++) {
		output_file << this->end_compressed_scope_sizes[sc_index] << endl;
		output_file << this->end_compressed_s_input_sizes[sc_index] << endl;
	}
}

Scope* construct_scope_helper(vector<Node*> nodes,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<SmallNetwork*>& outer_input_networks,
							  int& compress_num_layers,
							  int& compress_new_size,
							  Network*& compression_network,
							  vector<int>& compressed_scope_sizes,
							  vector<int>& compressed_s_input_sizes) {
	vector<int> scope_starts;
	vector<int> scope_ends;

	vector<int> process_scope_stack;
	int num_scopes = 0;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		int starting_num_scopes = num_scopes;
		if (nodes[n_index]->new_layer_size > 0) {
			num_scopes++;
			if (nodes[n_index]->compress_num_layers > 0) {
				num_scopes -= nodes[n_index]->compress_num_layers;
				if (nodes[n_index]->compress_new_size > 0) {
					num_scopes++;
				}
			}
		}
		int ending_num_scopes = num_scopes;

		if (ending_num_scopes > starting_num_scopes) {
			process_scope_stack.push_back(n_index);
		} else if (ending_num_scopes < starting_num_scopes) {
			int num_diff = starting_num_scopes - ending_num_scopes;
			for (int d_index = 0; d_index < num_diff; d_index++) {
				if (process_scope_stack.size() > 0) {
					scope_starts.push_back(process_scope_stack.back());
					scope_ends.push_back(n_index);
					process_scope_stack.pop_back();
				}
				// else end of scope
			}
		}
	}
	// num_scopes may be > 0, but ignore

	if (scope_starts.back() == 0 && scope_ends.back() == (int)nodes.size()-1) {
		// outer scope is curr scope
		scope_starts.pop_back();
		scope_ends.pop_back();
	}

	if (scope_starts.size() > 0) {
		int s_index = (int)scope_starts.size()-1;
		while (true) {
			if (s_index == 0) {
				break;
			}

			if (scope_starts[s_index] <= scope_starts[s_index-1]
					&& scope_ends[s_index] >= scope_ends[s_index-1]) {
				scope_starts.erase(scope_starts.begin()+s_index-1);
				scope_ends.erase(scope_ends.begin()+s_index-1);
			}
			s_index--;
		}
	}

	vector<AbstractScope*> actions;

	vector<vector<int>> input_sizes;
	vector<vector<SmallNetwork*>> input_networks;
	vector<int> scope_compressed_scope_sizes;
	vector<int> scope_compressed_s_input_sizes;

	vector<bool> is_simple_append;
	vector<int> new_layer_sizes;
	vector<Network*> obs_networks;

	vector<vector<int>> outer_input_indexes;

	vector<SmallNetwork*> score_networks;

	vector<vector<int>> inner_input_sizes;
	vector<vector<SmallNetwork*>> inner_input_networks;

	vector<int> inner_compress_num_layers;
	vector<int> inner_compress_new_sizes;
	vector<Network*> inner_compression_networks;
	vector<vector<int>> inner_compressed_scope_sizes;
	vector<vector<int>> inner_compressed_s_input_sizes;

	vector<int> end_compressed_scope_sizes;
	vector<int> end_compressed_s_input_sizes;

	int n_index = 0;
	int outer_input_index = 0;
	compress_num_layers = 0;
	compress_new_size = 0;
	compression_network = NULL;
	int num_outputs = 0;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		while (true) {
			if (n_index < scope_starts[s_index]) {
				actions.push_back(new BaseScope(nodes[n_index]->obs_size));

				// push empty
				input_sizes.push_back(vector<int>());
				input_networks.push_back(vector<SmallNetwork*>());
				scope_compressed_scope_sizes.push_back(0);
				scope_compressed_s_input_sizes.push_back(0);

				is_simple_append.push_back(false);
				new_layer_sizes.push_back(nodes[n_index]->new_layer_size);
				if (nodes[n_index]->obs_network == NULL) {
					obs_networks.push_back(NULL);
				} else {
					obs_networks.push_back(new Network(nodes[n_index]->obs_network));
				}

				outer_input_indexes.push_back(vector<int>());
				inner_input_sizes.push_back(vector<int>());
				inner_input_networks.push_back(vector<SmallNetwork*>());
				for (int i_index = 0; i_index < (int)nodes[n_index]->score_input_networks.size(); i_index++) {
					if (nodes[n_index]->score_input_layer[i_index] - curr_layer == 0) {
						inner_input_sizes.back().push_back(nodes[n_index]->score_input_sizes[i_index]);
						inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));
					} else {
						outer_input_layer.push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
						outer_input_sizes.push_back(nodes[n_index]->score_input_sizes[i_index]);
						outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));

						if (nodes[n_index]->score_input_layer[i_index] - curr_layer == -1) {
							for (int is_index = 0; is_index < nodes[n_index]->score_input_sizes[i_index]; is_index++) {
								outer_input_indexes.back().push_back(outer_input_index);
								outer_input_index++;
							}
						}
					}
				}
				for (int i_index = 0; i_index < (int)nodes[n_index]->input_networks.size(); i_index++) {
					if (nodes[n_index]->input_layer[i_index] - curr_layer == 0) {
						inner_input_sizes.back().push_back(nodes[n_index]->input_sizes[i_index]);
						inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));
					} else {
						outer_input_layer.push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
						outer_input_sizes.push_back(nodes[n_index]->input_sizes[i_index]);
						outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));

						if (nodes[n_index]->input_layer[i_index] - curr_layer == -1) {
							for (int is_index = 0; is_index < nodes[n_index]->input_sizes[i_index]; is_index++) {
								outer_input_indexes.back().push_back(outer_input_index);
								outer_input_index++;
							}
						}
					}
				}

				if (nodes[n_index]->score_network == NULL) {
					score_networks.push_back(NULL);
				} else {
					score_networks.push_back(new SmallNetwork(nodes[n_index]->score_network));
				}

				inner_compress_num_layers.push_back(nodes[n_index]->compress_num_layers);
				inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
				if (nodes[n_index]->compression_network == NULL) {
					inner_compression_networks.push_back(NULL);
				} else {
					inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
				}
				inner_compressed_scope_sizes.push_back(nodes[n_index]->compressed_scope_sizes);
				inner_compressed_s_input_sizes.push_back(nodes[n_index]->compressed_s_input_sizes);

				n_index++;
			} else {
				vector<Node*> subscope(nodes.begin()+scope_starts[s_index],
					nodes.begin()+scope_ends[s_index]+1);	// inclusive end

				vector<int> new_outer_input_layer;
				vector<int> new_outer_input_sizes;
				vector<SmallNetwork*> new_outer_input_networks;
				int new_compress_num_layers;
				int new_compress_num_size;
				Network* new_compression_network;
				vector<int> new_compressed_scope_sizes;
				vector<int> new_compressed_s_input_sizes;
				Scope* new_scope = construct_scope_helper(subscope,
														  curr_layer+1,
														  new_outer_input_layer,
														  new_outer_input_sizes,
														  new_outer_input_networks,
														  new_compress_num_layers,
														  new_compress_num_size,
														  new_compression_network,
														  new_compressed_scope_sizes,
														  new_compressed_s_input_sizes);
				actions.push_back(new_scope);

				if (scope_ends[s_index] == (int)nodes.size()-1) {
					num_outputs = new_scope->num_outputs+new_compressed_scope_sizes.back();
					end_compressed_scope_sizes.insert(end_compressed_scope_sizes.begin(),
						new_scope->num_outputs+new_compressed_scope_sizes.back());
					scope_compressed_scope_sizes.push_back(new_compressed_scope_sizes.back());
					new_compressed_scope_sizes.pop_back();
					end_compressed_s_input_sizes.insert(end_compressed_s_input_sizes.begin(),
						new_compressed_s_input_sizes.back());
					scope_compressed_s_input_sizes.push_back(new_compressed_s_input_sizes.back());
					new_compressed_s_input_sizes.pop_back();

					if (new_compress_num_layers == 1) {
						// don't need to update for parent

						is_simple_append.push_back(false);
						new_layer_sizes.push_back(new_compress_num_size);
						if (new_compression_network == NULL) {
							obs_networks.push_back(NULL);
						} else {
							obs_networks.push_back(new Network(new_compression_network));
						}
					} else {
						compress_num_layers = new_compress_num_layers-1;
						compress_new_size = new_compress_num_size;
						compression_network = new_compression_network;
						compressed_scope_sizes = new_compressed_scope_sizes;
						compressed_s_input_sizes = new_compressed_s_input_sizes;

						is_simple_append.push_back(true);
						new_layer_sizes.push_back(0);
						obs_networks.push_back(NULL);
					}
				} else {
					is_simple_append.push_back(false);
					new_layer_sizes.push_back(new_compress_num_size);
					if (new_compression_network == NULL) {
						obs_networks.push_back(NULL);

						scope_compressed_scope_sizes.push_back(0);
						scope_compressed_s_input_sizes.push_back(0);
					} else {
						obs_networks.push_back(new Network(new_compression_network));

						scope_compressed_scope_sizes.push_back(new_compressed_scope_sizes.back());
						new_compressed_scope_sizes.pop_back();
						scope_compressed_s_input_sizes.push_back(new_compressed_s_input_sizes.back());
						new_compressed_s_input_sizes.pop_back();
					}
				}

				input_sizes.push_back(vector<int>());
				input_networks.push_back(vector<SmallNetwork*>());
				outer_input_indexes.push_back(vector<int>());
				for (int i_index = 0; i_index < (int)new_outer_input_networks.size(); i_index++) {
					// new_outer_input_layer[i_index] < 0
					if (new_outer_input_layer[i_index] == -1) {
						input_sizes.back().push_back(new_outer_input_sizes[i_index]);
						input_networks.back().push_back(new_outer_input_networks[i_index]);
					} else {
						outer_input_layer.push_back(new_outer_input_layer[i_index] + 1);
						outer_input_sizes.push_back(new_outer_input_sizes[i_index]);
						outer_input_networks.push_back(new_outer_input_networks[i_index]);

						if (new_outer_input_layer[i_index] + 1 == -1) {
							for (int is_index = 0; is_index < new_outer_input_sizes[i_index]; is_index++) {
								outer_input_indexes.back().push_back(outer_input_index);
								outer_input_index++;
							}
						}
					}
				}

				score_networks.push_back(NULL);

				inner_input_sizes.push_back(vector<int>());
				inner_input_networks.push_back(vector<SmallNetwork*>());

				inner_compress_num_layers.push_back(0);
				inner_compress_new_sizes.push_back(0);
				inner_compression_networks.push_back(NULL);
				inner_compressed_scope_sizes.push_back(vector<int>());
				inner_compressed_s_input_sizes.push_back(vector<int>());

				n_index = scope_ends[s_index]+1;

				break;
			}
		}
	}
	while (true) {
		if (n_index >= (int)nodes.size()) {
			break;
		}

		actions.push_back(new BaseScope(nodes[n_index]->obs_size));

		// push empty
		input_sizes.push_back(vector<int>());
		input_networks.push_back(vector<SmallNetwork*>());
		scope_compressed_scope_sizes.push_back(0);
		scope_compressed_s_input_sizes.push_back(0);

		is_simple_append.push_back(false);
		new_layer_sizes.push_back(nodes[n_index]->new_layer_size);
		if (nodes[n_index]->obs_network == NULL) {
			obs_networks.push_back(NULL);
		} else {
			obs_networks.push_back(new Network(nodes[n_index]->obs_network));
		}

		outer_input_indexes.push_back(vector<int>());
		inner_input_sizes.push_back(vector<int>());
		inner_input_networks.push_back(vector<SmallNetwork*>());
		for (int i_index = 0; i_index < (int)nodes[n_index]->score_input_networks.size(); i_index++) {
			if (nodes[n_index]->score_input_layer[i_index] - curr_layer == 0) {
				inner_input_sizes.back().push_back(nodes[n_index]->score_input_sizes[i_index]);
				inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));
			} else {
				outer_input_layer.push_back(nodes[n_index]->score_input_layer[i_index] - curr_layer);
				outer_input_sizes.push_back(nodes[n_index]->score_input_sizes[i_index]);
				outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->score_input_networks[i_index]));

				if (nodes[n_index]->score_input_layer[i_index] - curr_layer == -1) {
					for (int is_index = 0; is_index < nodes[n_index]->score_input_sizes[i_index]; is_index++) {
						outer_input_indexes.back().push_back(outer_input_index);
						outer_input_index++;
					}
				}
			}
		}
		for (int i_index = 0; i_index < (int)nodes[n_index]->input_networks.size(); i_index++) {
			if (nodes[n_index]->input_layer[i_index] - curr_layer == 0) {
				inner_input_sizes.back().push_back(nodes[n_index]->input_sizes[i_index]);
				inner_input_networks.back().push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));
			} else {
				outer_input_layer.push_back(nodes[n_index]->input_layer[i_index] - curr_layer);
				outer_input_sizes.push_back(nodes[n_index]->input_sizes[i_index]);
				outer_input_networks.push_back(new SmallNetwork(nodes[n_index]->input_networks[i_index]));

				if (nodes[n_index]->input_layer[i_index] - curr_layer == -1) {
					for (int is_index = 0; is_index < nodes[n_index]->input_sizes[i_index]; is_index++) {
						outer_input_indexes.back().push_back(outer_input_index);
						outer_input_index++;
					}
				}
			}
		}

		if (nodes[n_index]->score_network == NULL) {
			score_networks.push_back(NULL);
		} else {
			score_networks.push_back(new SmallNetwork(nodes[n_index]->score_network));
		}

		if (n_index == (int)nodes.size()-1) {
			if (nodes[n_index]->compress_num_layers == 0
					|| nodes[n_index]->compress_num_layers == 1
					|| (nodes[n_index]->compress_num_layers == 2 && nodes[n_index]->compress_new_size > 0)) {
				// edge case
				inner_compress_num_layers.push_back(nodes[n_index]->compress_num_layers);
				inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
				// nodes[n_index]->compress_new_size > 0
				if (nodes[n_index]->compression_network == NULL) {
					inner_compression_networks.push_back(NULL);
				} else {
					inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
				}
				inner_compressed_scope_sizes.push_back(nodes[n_index]->compressed_scope_sizes);
				inner_compressed_s_input_sizes.push_back(nodes[n_index]->compressed_s_input_sizes);
			} else {
				// output last 2 layers, and pass the rest
				compress_num_layers = nodes[n_index]->compress_num_layers-2;
				compress_new_size = nodes[n_index]->compress_new_size;
				compression_network = nodes[n_index]->compression_network;
				compressed_scope_sizes = nodes[n_index]->compressed_scope_sizes;
				compressed_s_input_sizes = nodes[n_index]->compressed_s_input_sizes;

				num_outputs += compressed_scope_sizes.back();
				end_compressed_scope_sizes.insert(end_compressed_scope_sizes.begin(), compressed_scope_sizes.back());
				compressed_scope_sizes.pop_back();
				end_compressed_s_input_sizes.insert(end_compressed_s_input_sizes.begin(), compressed_s_input_sizes.back());
				compressed_s_input_sizes.pop_back();
				num_outputs += compressed_scope_sizes.back();
				end_compressed_scope_sizes.insert(end_compressed_scope_sizes.begin(), compressed_scope_sizes.back());
				compressed_scope_sizes.pop_back();
				end_compressed_s_input_sizes.insert(end_compressed_s_input_sizes.begin(), compressed_s_input_sizes.back());
				compressed_s_input_sizes.pop_back();

				inner_compress_num_layers.push_back(0);
				inner_compress_new_sizes.push_back(0);
				inner_compression_networks.push_back(NULL);
				inner_compressed_scope_sizes.push_back(vector<int>());
				inner_compressed_s_input_sizes.push_back(vector<int>());
			}
		} else {
			inner_compress_num_layers.push_back(nodes[n_index]->compress_num_layers);
			inner_compress_new_sizes.push_back(nodes[n_index]->compress_new_size);
			if (nodes[n_index]->compression_network == NULL) {
				inner_compression_networks.push_back(NULL);
			} else {
				inner_compression_networks.push_back(new Network(nodes[n_index]->compression_network));
			}
			inner_compressed_scope_sizes.push_back(nodes[n_index]->compressed_scope_sizes);
			inner_compressed_s_input_sizes.push_back(nodes[n_index]->compressed_s_input_sizes);
		}

		n_index++;
	}

	Scope* scope = new Scope(actions,
							 outer_input_index,
							 num_outputs,
							 input_sizes,
							 input_networks,
							 scope_compressed_scope_sizes,
							 scope_compressed_s_input_sizes,
							 is_simple_append,
							 new_layer_sizes,
							 obs_networks,
							 outer_input_indexes,
							 score_networks,
							 inner_input_sizes,
							 inner_input_networks,
							 inner_compress_num_layers,
							 inner_compress_new_sizes,
							 inner_compression_networks,
							 inner_compressed_scope_sizes,
							 inner_compressed_s_input_sizes,
							 end_compressed_scope_sizes,
							 end_compressed_s_input_sizes);

	return scope;
}

Scope* construct_scope(vector<Node*> nodes) {
	// will be empty
	vector<int> outer_input_layer;
	vector<int> outer_input_sizes;
	vector<SmallNetwork*> outer_input_networks;
	int compress_num_layers;
	int compress_num_size;
	Network* compression_network;
	vector<int> compressed_scope_sizes;
	vector<int> compressed_s_input_sizes;

	Scope* scope = construct_scope_helper(nodes,
										  -1,	// start at -1 because outer scope has no state
										  outer_input_layer,
										  outer_input_sizes,
										  outer_input_networks,
										  compress_num_layers,
										  compress_num_size,
										  compression_network,
										  compressed_scope_sizes,
										  compressed_s_input_sizes);

	return scope;
}
