#include "scope_path.h"

using namespace std;

ScopePath::ScopePath(int num_input_states,
					 int num_local_states,
					 int sequence_length,
					 vector<bool> is_inner_scope,
					 vector<Scope*> scopes,
					 vector<vector<bool>> inner_input_is_local,
					 vector<vector<int>> inner_input_indexes,
					 vector<vector<bool>> state_network_target_is_local,
					 vector<vector<int>> state_network_target_indexes,
					 vector<vector<Network*>> state_networks,
					 vector<bool> has_score_network,
					 vector<Network*> score_networks) {
	this->num_input_states = num_input_states;
	this->num_local_states = num_local_states;
	this->sequence_length = sequence_length;
	this->is_inner_scope = is_inner_scope;
	this->scopes = scopes;
	this->inner_input_is_local = inner_input_is_local;
	this->inner_input_indexes = inner_input_indexes;
	this->state_network_target_is_local = state_network_target_is_local;
	this->state_network_target_indexes = state_network_target_indexes;
	this->state_networks = state_networks;
	this->has_score_network = has_score_network;
	this->score_networks = score_networks;
}

ScopePath::~ScopePath() {
	// TODO: switch to using scope dictionary
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->scopes[a_index] != NULL) {
			delete this->scopes[a_index];
		}

		for (int s_index = 0; s_index < (int)this->state_networks[a_index].size(); s_index++) {
			delete this->state_networks[a_index][s_index];
		}

		if (this->score_networks[a_index] != NULL) {
			delete this->score_networks[a_index];
		}
	}
}

void ScopePath::activate(vector<double>& input_vals,
						 vector<vector<double>>& flat_vals,
						 double& predicted_score) {
	vector<double> local_state_vals(this->num_local_states, 0.0);

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->is_inner_scope[a_index]) {
			vector<double> scope_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_is_local[a_index].size(); i_index++) {
				if (this->inner_input_is_local[a_index][i_index]) {
					scope_input_vals.push_back(local_state_vals[this->inner_input_indexes[a_index][i_index]]);
				} else {
					scope_input_vals.push_back(input_vals[this->inner_input_indexes[a_index][i_index]]);
				}
			}
			for (int i_index = (int)this->inner_input_is_local[a_index].size(); i_index < this->scopes[a_index]->num_input_states; i_index++) {
				scope_input_vals.push_back(0.0);
			}

			this->scopes[a_index]->activate(scope_input_vals,
											flat_vals,
											predicted_score);

			for (int i_index = 0; i_index < (int)this->inner_input_is_local[a_index].size(); i_index++) {
				if (this->inner_input_is_local[a_index][i_index]) {
					local_state_vals[this->inner_input_indexes[a_index][i_index]] += scope_input_vals[i_index];
				} else {
					input_vals[this->inner_input_indexes[a_index][i_index]] += scope_input_vals[i_index];
				}
			}
		} else {
			vector<double> obs = *flat_vals.begin();

			for (int s_index = 0; s_index < (int)this->state_networks[a_index].size(); s_index++) {
				this->state_networks[a_index][s_index]->scope_activate(obs,
																	   input_vals,
																	   local_state_vals);
				if (this->state_network_target_is_local[a_index][s_index]) {
					local_state_vals[this->state_network_target_indexes[a_index][s_index]] += 
						this->state_networks[a_index][s_index]->output->acti_vals[0];
				} else {
					input_vals[this->state_network_target_indexes[a_index][s_index]] +=
						this->state_networks[a_index][s_index]->output->acti_vals[0];
				}
			}

			flat_vals.erase(flat_vals.begin());
		}

		if (this->has_score_network[a_index]) {
			this->score_networks[a_index]->scope_activate(input_vals,
														  local_state_vals);
			predicted_score += this->score_networks[a_index]->output->acti_vals[0];
		}
	}
}
