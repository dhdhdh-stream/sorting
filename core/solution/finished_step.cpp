#include "finished_step.h"

#include <iostream>

using namespace std;

FinishedStep::FinishedStep(AbstractScope* scope,
						   SubFoldNetwork* inner_input_network,
						   int new_layer_size,
						   Network* obs_network,
						   SubFoldNetwork* score_network,
						   int compress_num_layers,
						   int compress_new_size,
						   CompressNetwork* compress_network,
						   vector<int> compressed_scope_sizes,
						   vector<int> compressed_s_input_sizes,
						   vector<int> input_layer,
						   vector<int> input_sizes,
						   vector<SmallNetwork*> input_networks) {
	this->scope_type = scope->type;
	this->scope = scope;
	this->inner_input_network = inner_input_network;
	this->new_layer_size = new_layer_size;
	this->obs_network = obs_network;
	this->score_network = score_network;
	this->compress_num_layers = compress_num_layers;
	this->compress_new_size = compress_new_size;
	this->compress_network = compress_network;
	this->compressed_scope_sizes = compressed_scope_sizes;
	this->compressed_s_input_sizes = compressed_s_input_sizes;
	this->input_layer = input_layer;
	this->input_sizes = input_sizes;
	this->input_networks = input_networks;
}

FinishedStep::~FinishedStep() {
	// do nothing
}

void FinishedStep::activate(Problem& problem,
							vector<vector<double>>& state_vals,
							vector<vector<double>>& s_input_vals,
							double& predicted_score) {
	vector<double> obs_input;
	if (this->action_type == SCOPE_TYPE_ACTION) {
		problem.perform_action(((BaseActionScope*)this->scope)->action);

		obs_input.push_back(problem.get_observation());
	} else {
		// this->action_type == SCOPE_TYPE_SCOPE
		Scope* scope_scope = (Scope*)this->scope;

		vector<double> scope_input;
		if (scope_scope->num_inputs > 0) {
			for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
				this->inner_input_input_networks[i_index]->activate(state_vals[this->inner_input_input_layer[i_index]],
																	s_input_vals[this->inner_input_input_layer[i_index]]);
				for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
					s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
						this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
				}
			}

			this->inner_input_network->activate(state_vals.back(),
												s_input_vals.back());
			scope_input.reserve(scope_scope->num_inputs);
			for (int i_index = 0; i_index < scope_scope->num_inputs; i_index++) {
				scope_input.push_back(this->inner_input_network->output->acti_vals[i_index]);
			}
		}
		vector<double> scope_output;
		double scope_predicted_score = 0.0;
		scope_scope->activate(problem,
							  scope_input,
							  scope_output,
							  scope_predicted_score);
		obs_input = scope_output;
		obs_input.push_back(scope_predicted_score);
	}

	if (this->new_layer_size) {
		this->obs_network->activate(obs_input);
		state_vals.push_back(vector<double>(this->new_layer_size));
		s_input_vals.push_back(vector<double>());
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}

		for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
			this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]],
													s_input_vals[this->input_layer[i_index]]);
			for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
				s_input_vals[this->input_layer[i_index]+1].push_back(
					this->input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		vector<double> back_state_inputs;
		for (int l_index = (int)state_vals.size()-this->compress_num_layers; l_index < (int)state_vals.size(); l_index++) {
			for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
				back_inputs.push_back(state_vals[l_index][st_index]);
			}
		}

		this->score_network->activate_small(back_state_inputs,
											s_input_vals[state_vals.size()-this->compress_num_layers]);
		predicted_score += this->score_network->output->acti_vals[0];

		if (this->compress_size > 0) {
			int compress_new_size = (int)back_state_inputs.size() - this->compress_size;
			if (compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
					s_input_vals.pop_back();
				}
			} else {
				this->compress_network->activate_small(back_state_inputs,
													   s_input_vals[state_vals.size()-this->compress_num_layers]);
				for (int s_index = 0; s_index < this->compress_num_layers-1; s_index++) {
					state_vals.pop_back();
					s_input_vals.pop_back();
				}
				state_vals.pop_back();
				state_vals.push_back(vector<double>(compress_new_size));
				// don't pop last layer of s_input_vals
				for (int st_index = 0; st_index < compress_new_size; st_index++) {
					state_vals.back()[st_index] = this->compress_network->output->acti_vals[st_index];
				}
			}
		}
	}
}
