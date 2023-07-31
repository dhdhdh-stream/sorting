#include "branch_experiment.h"

using namespace std;

void BranchExperiment::explore_transform() {
	Scope* outer_scope = solution->scopes[this->scope_context.back()];

	this->starting_score_network = new ScoreNetwork(outer_scope->num_states,
													NUM_NEW_STATES,
													20);
	this->starting_score_network->update_lasso_weights(2);
	this->starting_misguess_network = new ScoreNetwork(outer_scope->num_states,
													   NUM_NEW_STATES,
													   20);
	this->starting_misguess_network->update_lasso_weights(2);
	this->branch_weight = 0.5;

	this->step_state_networks = vector<vector<StateNetwork*>>(this->num_steps);
	this->step_score_networks = vector<ScoreNetwork*>(this->num_steps, NULL);
	this->sequence_scale_mods = vector<Scale*>(this->num_steps, NULL);
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			this->step_state_networks[a_index] = vector<StateNetwork*>(NUM_NEW_STATES);
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				this->step_state_networks[a_index][s_index] = new StateNetwork(0,
																			   NUM_NEW_STATES,
																			   0,
																			   20);
				this->step_state_networks[a_index][s_index]->update_lasso_weights(1);
			}
			this->step_score_networks[a_index] = new ScoreNetwork(0,
																  NUM_NEW_STATES,
																  20);
			this->step_score_networks[a_index]->update_lasso_weights(1);
		} else {
			for (int l_index = 0; l_index < (int)this->sequences[a_index]->scopes.size(); l_index++) {
				Scope* scope = this->sequences[a_index]->scopes[l_index];
				map<int, vector<vector<StateNetwork*>>>::iterator state_it = this->state_networks.find(scope->id);
				map<int, vector<ScoreNetwork*>>::iterator score_it = this->score_networks.find(scope->id);
				if (state_it == this->state_networks.end()) {
					this->state_networks[scope->id] = vector<vector<StateNetwork*>>(scope->nodes.size());
					this->score_networks[scope->id] = vector<ScoreNetwork*>(scope->nodes.size());
					this->scope_furthest_layer_seen_in[scope->id] = this->scope_context.size()+1;
					this->scope_steps_seen_in[scope->id] = vector<bool>(this->num_steps, false);

					// in sequence networks initialized in ActionNode
				}
				this->scope_steps_seen_in[scope->id][a_index] = true;
			}

			this->sequences[a_index]->step_state_networks = vector<vector<StateNetwork*>>(this->num_steps);
			for (int ia_index = 0; ia_index < a_index; ia_index++) {
				if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					this->sequences[a_index]->step_state_networks[ia_index] = vector<StateNetwork*>(this->sequences[a_index]->input_init_types.size());
					for (int i_index = 0; i_index < (int)this->sequences[a_index]->input_init_types.size(); i_index++) {
						this->sequences[a_index]->step_state_networks[ia_index][i_index] =
							new StateNetwork(0,
											 NUM_NEW_STATES,
											 1,
											 20);
						this->sequences[a_index]->step_state_networks[ia_index][i_index]->update_lasso_weights(1);
					}
				} else {
					for (int l_index = 0; l_index < (int)this->sequences[ia_index]->scopes.size(); l_index++) {
						Scope* scope = this->sequences[ia_index]->scopes[l_index];
						map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.find(scope->id);
						if (it == this->sequences[a_index]->state_networks.end()) {
							it = this->sequences[a_index]->state_networks.insert({scope->id, vector<vector<StateNetwork*>>(scope->nodes.size())}).first;
							this->sequences[a_index]->scope_furthest_layer_seen_in[scope->id] = this->scope_context.size()+1;
							for (int n_index = 0; n_index < this->sequences[ia_index]->node_ids[l_index].size(); n_index++) {
								int node_id = this->sequences[ia_index]->node_ids[l_index][n_index];
								if (this->sequences[ia_index]->scopes[l_index]->nodes[node_id]->type == NODE_TYPE_ACTION) {
									for (int i_index = 0; i_index < (int)this->sequences[a_index]->input_init_types.size(); i_index++) {
										it->second[node_id].push_back(
											new StateNetwork(this->sequences[ia_index]->num_states,
															 NUM_NEW_STATES,
															 1,
															 20));
										it->second[node_id].back()->update_lasso_weights(1);
									}
								}
							}
						}
					}
				}
			}

			this->sequence_scale_mods[a_index] = new Scale();
		}
	}

	Scope* exit_scope = solution->scopes[this->scope_context[this->scope_context.size() - (this->exit_depth+1)]];
	this->exit_networks = vector<ExitNetwork*>(exit_scope->num_states);
	vector<int> exit_context_sizes(this->exit_depth+1);
	for (int l_index = 0; l_index < this->exit_depth+1; l_index++) {
		Scope* scope = solution->scopes[this->scope_context[
			this->scope_context.size() - (this->exit_depth+1) + l_index]];
		exit_context_sizes[l_index] = scope->num_states;
	}
	for (int e_index = 0; e_index < exit_scope->num_states; e_index++) {
		this->exit_networks = new ExitNetwork(exit_context_sizes,
											  NUM_NEW_STATES,
											  20);
	}
	this->exit_network_impacts = vector<double>(exit_scope->num_states, 0.0);

	this->branch_average_score = 0.0;
	this->existing_average_score = 0.0;
	this->branch_average_misguess = 0.0;
	this->existing_average_misguess = 0.0;

	this->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
