#include "node.h"

#include <iostream>

using namespace std;

const double TARGET_MAX_UPDATE = 0.001;

void Node::activate(vector<vector<double>>& state_vals,
					vector<double>& obs) {
	if (this->new_layer_size > 0) {
		this->obs_network->activate(obs);
		state_vals.push_back(vector<double>(this->new_layer_size));
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}
	}

	for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
		this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]]);
		for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
			state_vals[this->score_input_layer[i_index]+1].push_back(
				this->score_input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	// don't necessary need to always activate score_network? (but should always add input)
	this->score_network->activate(state_vals.back());

	if (this->compress_size > 0) {
		for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
			this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]]);
			for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
				state_vals[this->input_layer[i_index]+1].push_back(
					this->input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		vector<double> compression_inputs;
		for (int l_index = state_vals.size()-this->compress_num_layers; l_index < state_vals.size(); l_index++) {
			for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
				compression_inputs.push_back(state_vals[l_index][st_index]);
			}
		}
		this->compression_network->activate(compression_inputs);

		int sum_scope_sizes = 0;
		for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
			sum_scope_sizes += (int)state_vals.back().size();
			state_vals.pop_back();
			scopes_on.pop_back();
		}
		state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_size));

		for (int st_index = 0; st_index < (int)state_vals.back(); st_index++) {
			state_vals.back()[st_index] = this->compression_network->output->acti_vals[st_index];
		}
	}
}

void Node::backprop(double target_val,
					vector<vector<double>>& state_errors) {
	if (this->compress_size > 0) {
		this->compression_network->backprop(state_errors.back(),
											TARGET_MAX_UPDATE);

		state_errors.pop_back();
		for (int sc_index = this->compress_num_scopes-1; sc_index >= 0; sc_index--) {
			state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
		}

		int input_index = 0;
		for (int l_index = state_errors.size()-this->compress_num_layers; l_index < state_errors.size(); l_index++) {
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

	vector<double> score_errors;
	score_errors.push_back(target_val - this->score_network->output->acti_vals[0]);
	this->score_network->backprop(score_errors, TARGET_MAX_UPDATE);
	for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
		state_errors.back()[st_index] = this->score_network->input->errors[st_index];
		this->score_network->input->errors[st_index] = 0.0;
	}

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

	if (this->new_layer_size > 0) {
		this->obs_network->backprop(state_errors.back());
		state_errors.pop_back();
	}
}
