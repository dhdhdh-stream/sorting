#include "branch_experiment.h"

using namespace std;

void BranchExperiment::wrapup_transform() {
	Scope* new_scope = new Scope(solution->scopes.size(),
								 this->new_num_states,
								 this->new_state_initialized_locally,
								 this->new_state_family_ids,
								 this->new_state_default_class_ids,
								 false,
								 vector<int>(),
								 vector<StateNetwork*>(),
								 NULL,
								 NULL,
								 NULL,
								 NULL);
	solution->scopes.push_back(new_scope);

	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		int next_node_id;
		if (a_index == this->num_steps-1) {
			next_node_id = -1;
		} else {
			next_node_id = a_index+1;
		}
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			ActionNode* new_node = new ActionNode(new_scope,
												  a_index,
												  this->new_action_node_target_indexes[a_index],
												  this->new_action_node_state_networks[a_index],
												  this->step_score_networks[a_index],
												  next_node_id);
			new_scope->nodes.push_back(new_node);
		} else {
			vector<Scope*> new_sequence_scopes;
			for (int l_index = 0; l_index < (int)this->sequences[a_index]->scopes.size(); l_index++) {
				Scope* new_sequence_scope = new Scope(solution->scopes.size(),
													  this->sequences[a_index]->scopes[l_index]->num_states,
													  this->sequences[a_index]->scopes[l_index]->state_initialized_locally,
													  this->sequences[a_index]->scopes[l_index]->state_family_ids,
													  this->sequences[a_index]->scopes[l_index]->state_default_class_ids,
													  false,
													  vector<int>(),
													  vector<StateNetwork*>(),
													  NULL,
													  NULL,
													  NULL,
													  NULL);
				// can't be loop
				solution->scopes.push_back(new_sequence_scope);
				new_sequence_scopes.push_back(new_sequence_scope);
			}

			vector<int> new_starting_node_ids = this->sequences[a_index]->starting_node_ids;
			new_starting_node_ids[0] = 0;

			vector<int> new_input_indexes;
			vector<int> new_input_target_layers;
			vector<int> new_input_target_indexes;
			vector<bool> new_input_has_transform;
			vector<Transformation> new_input_transformations;
			for (int i_index = 0; i_index < (int)this->sequences[a_index]->input_init_types.size(); i_index++) {
				if (this->sequences[a_index]->input_index_translations[i_index] != -1) {
					new_input_indexes.push_back(this->sequences[a_index]->input_index_translations[i_index]);
					new_input_target_layers.push_back(this->sequences[a_index]->input_init_target_layers[i_index]);
					new_input_target_indexes.push_back(this->sequences[a_index]->input_init_target_indexes[i_index]);
					new_input_has_transform.push_back(false);
					new_input_transformations.push_back(Transformation());
				}
			}

			ScopeNode* new_outer_node = new ScopeNode(new_scope,
													  a_index,
													  new_sequence_scopes[0]->id,
													  new_starting_node_ids,
													  new_input_indexes,
													  new_input_target_layers,
													  new_input_target_indexes,
													  new_input_has_transform,
													  new_input_transformations,
													  this->sequence_scale_mods[a_index],
													  next_node_id);
			new_scope->nodes.push_back(new_outer_node);

			for (int l_index = 0; l_index < (int)this->sequences[a_index]->scopes.size(); l_index++) {
				for (int n_index = 0; n_index < (int)this->sequences[a_index]->node_ids[l_index].size()-1; n_index++) {
					AbstractNode* original_node = this->sequences[a_index]->scopes[l_index]->nodes[
						this->sequences[a_index]->node_ids[l_index][n_index]];
					if (original_node->type == NODE_TYPE_ACTION) {
						ActionNode* new_inner_node = new ActionNode((ActionNode*)original_node,
																	new_sequence_scopes[l_index],
																	n_index,
																	n_index+1);
						new_sequence_scopes[l_index]->nodes.push_back(new_inner_node);
					} else {
						ScopeNode* new_inner_node = new ScopeNode((ScopeNode*)original_node,
																  new_sequence_scopes[l_index],
																  n_index,
																  n_index+1);
						new_sequence_scopes[l_index]->nodes.push_back(new_inner_node);
					}
				}

				if (l_index == (int)this->sequences[a_index]->scopes.size()-1) {
					AbstractNode* original_node = this->sequences[a_index]->scopes.back()->nodes[
						this->sequences[a_index]->node_ids.back().back()];
					if (original_node->type == NODE_TYPE_ACTION) {
						ActionNode* new_inner_node = new ActionNode((ActionNode*)original_node,
																	new_sequence_scopes.back(),
																	(int)this->sequences[a_index]->node_ids.back().size()-1,
																	-1);
						new_sequence_scopes.back()->nodes.push_back(new_inner_node);
					} else {
						ScopeNode* new_inner_node = new ScopeNode((ScopeNode*)original_node,
																  new_sequence_scopes.back(),
																  (int)this->sequences[a_index]->node_ids.back().size()-1,
																  -1);
						new_sequence_scopes.back()->nodes.push_back(new_inner_node);
					}
				} else {
					ScopeNode* original_node = (ScopeNode*)this->sequences[a_index]->scopes[l_index]->nodes[
						this->sequences[a_index]->node_ids[l_index].back()];
					ScopeNode* new_ending_node = new ScopeNode(original_node,
															   new_sequence_scopes[l_index],
															   (int)this->sequences[a_index]->node_ids[l_index].size()-1,
															   -1);
					new_ending_node->inner_scope_id = new_sequence_scopes[l_index+1]->id;
					new_sequence_scopes[l_index]->nodes.push_back(new_ending_node);
				}
			}
		}
	}

	Scope* outer_scope = solution->scopes[this->scope_context.back()];
	ActionNode* original_action_node = (ActionNode*)outer_scope->nodes[this->node_context.back()];

	vector<int> new_exit_node_target_indexes;
	vector<ExitNetwork*> new_exit_node_networks;
	for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
		if (this->exit_networks[e_index] != NULL) {
			new_exit_node_target_indexes.push_back(e_index);
			new_exit_node_networks.push_back(this->exit_networks[e_index]);
		}
	}
	ExitNode* new_exit_node = new ExitNode(outer_scope,
										   (int)outer_scope->nodes.size(),
										   this->exit_depth,
										   this->exit_node_id,
										   new_exit_node_target_indexes,
										   new_exit_node_networks);
	outer_scope->nodes.push_back(new_exit_node);

	ScopeNode* new_scope_node = new ScopeNode(outer_scope,
											  (int)outer_scope->nodes.size(),
											  new_scope->id,
											  vector<int>{0},
											  this->last_layer_indexes,
											  vector<int>(this->last_layer_indexes.size(), 0),
											  this->last_layer_has_transform,
											  this->last_layer_transformations,
											  new Scale(),
											  new_exit_node->id);
	outer_scope->nodes.push_back(new_scope_node);

	vector<int> new_branch_node_context = this->node_context;
	new_branch_node_context.back() = -1;
	BranchNode* new_branch_node;
	if (this->branch_weight > 0.99) {
		delete this->starting_score_network;
		delete this->starting_misguess_network;
		delete this->starting_original_score_network;
		delete this->starting_original_misguess_network;

		new_branch_node = new BranchNode(outer_scope,
										 (int)outer_scope->nodes.size(),
										 this->scope_context,
										 new_branch_node_context,
										 true,
										 NULL,
										 NULL,
										 new_exit_node->id,
										 NULL,
										 NULL,
										 original_action_node->next_node_id,
										 1.0);
	} else {
		new_branch_node = new BranchNode(outer_scope,
										 (int)outer_scope->nodes.size(),
										 this->scope_context,
										 new_branch_node_context,
										 false,
										 this->starting_score_network,
										 this->starting_misguess_network,
										 new_exit_node->id,
										 this->starting_original_score_network,
										 this->starting_original_misguess_network,
										 original_action_node->next_node_id,
										 this->branch_weight);
	}
	outer_scope->nodes.push_back(new_branch_node);

	original_action_node->next_node_id = new_branch_node->id;

	for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
			it != this->score_networks.end(); it++) {
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			if (it->second[n_index] != NULL) {
				delete it->second[n_index];
			}
		}
	}
	delete this->seed_outer_context_history;

	this->state = BRANCH_EXPERIMENT_STATE_DONE;
}