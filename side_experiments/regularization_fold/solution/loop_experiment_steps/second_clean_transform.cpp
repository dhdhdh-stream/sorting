#include "loop_experiment.h"

#include "action_node.h"
#include "class_definition.h"
#include "exit_network.h"
#include "family_definition.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "sequence.h"
#include "state_network.h"

using namespace std;

void LoopExperiment::second_clean_transform() {
	for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
			it != this->score_networks.end(); it++) {
		int updated_state_size = solution->scopes[it->first]->num_states;
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			if (it->second[n_index] != NULL) {
				while (it->second[n_index]->state_size < updated_state_size) {
					it->second[n_index]->add_state();
				}
			}
		}
	}

	this->new_num_states = 0;

	for (int i_index = 0; i_index < (int)this->sequence->input_types.size(); i_index++) {
		int continue_scope_id = this->scope_context[this->scope_context.size()-1 - this->sequence->input_local_scope_depths[i_index]];
		Scope* continue_scope = solution->scopes[continue_scope_id];
		int new_family_id = continue_scope->state_family_ids[this->sequence->input_local_input_indexes[i_index]];

		int starting_scope_id;
		if (!this->sequence->input_is_new_class[i_index]) {
			if (this->sequence->input_local_scope_depths[i_index] == 0) {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family_id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(this->sequence->input_local_input_indexes[i_index]);
				this->last_layer_target_indexes.push_back(this->new_num_states-1);
			} else {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family_id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
				this->last_layer_target_indexes.push_back(this->new_num_states-1);

				int continue_node_id = this->node_context[this->scope_context.size()-1 - this->sequence->input_local_scope_depths[i_index]];
				ScopeNode* continue_scope_node = (ScopeNode*)continue_scope->nodes[continue_node_id];
				Scope* inner_scope = solution->scopes[continue_scope_node->inner_scope_id];

				continue_scope_node->input_indexes.push_back(this->sequence->input_local_input_indexes[i_index]);
				continue_scope_node->input_target_layers.push_back(0);
				continue_scope_node->input_target_indexes.push_back(inner_scope->num_states);	// new state not added yet
				continue_scope_node->input_has_transform.push_back(false);
				continue_scope_node->input_transformations.push_back(Transformation());

				// special case last layer, as was skipped due to from ActionNode
				this->sequence->scope_additions_needed[i_index].insert(this->scope_context.back());
			}

			starting_scope_id = -1;	// simply set to -1 for below
		} else {
			ClassDefinition* new_class = new ClassDefinition((int)solution->classes.size(),
															 new_family_id);
			solution->classes.push_back(new_class);

			FamilyDefinition* new_family = solution->families[new_family_id];
			for (int cc_index = 0; cc_index < (int)this->corr_calc_scope_depths.size(); cc_index++) {
				double pcc = this->sequence->corr_calc_covariances[cc_index][i_index]
					/ this->corr_calc_variances[cc_index]
					/ this->sequence->corr_calc_new_variances[cc_index][i_index];
				if (abs(pcc) > 0.6) {
					Scope* scope = solution->scopes[this->scope_context.size()-1 - this->corr_calc_scope_depths[cc_index]];
					FamilyDefinition* existing_family = solution->families[scope->state_family_ids[this->corr_calc_input_indexes[cc_index]]];

					int new_existing_index = -1;
					for (int f_index = 0; f_index < (int)new_family->similar_family_ids.size(); f_index++) {
						if (new_family->similar_family_ids[f_index] == existing_family->id) {
							new_existing_index = f_index;
							break;
						}
					}

					if (new_existing_index == -1) {
						existing_family->similar_family_ids.push_back(new_family_id);
						existing_family->transformations.push_back(this->sequence->new_transformations[cc_index][i_index].transformation);
						existing_family->pcc.push_back(pcc);

						new_family->similar_family_ids.push_back(existing_family->id);
						new_family->transformations.push_back(this->sequence->new_transformations[cc_index][i_index].reverse());
						new_family->pcc.push_back(pcc);
					} else {
						if (abs(pcc) > abs(new_family->pcc[new_existing_index])) {
							int existing_existing_index = -1;
							for (int f_index = 0; f_index < (int)existing_family->similar_family_ids.size(); f_index++) {
								if (existing_family->similar_family_ids[f_index] == new_family_id) {
									existing_existing_index = f_index;
									break;
								}
							}
							existing_family->transformations[existing_existing_index] = this->sequence->new_transformations[cc_index][i_index].transformation;
							existing_family->pcc[existing_existing_index] = pcc;

							new_family->transformations[new_existing_index] = this->sequence->new_transformations[cc_index][i_index].reverse();
							new_family->pcc[new_existing_index] = pcc;
						}
					}
				}
			}

			if (this->sequence->input_furthest_layer_needed_in[i_index] == 0) {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family_id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
				this->last_layer_target_indexes.push_back(this->new_num_states-1);

				starting_scope_id = 0;

				solution->scopes[0]->add_state(true,
											   new_family_id,
											   new_class->id);

				// special case last layer, as was skipped due to from ActionNode
				this->sequence->scope_additions_needed[i_index].insert(this->scope_context.back());
			} else if (this->sequence->input_furthest_layer_needed_in[i_index] >= (int)this->scope_context.size()+1) {
				// can't actually be scope_context.size()+1, but can be scope_context.size()+2
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(true);
				this->new_state_family_ids.push_back(new_family_id);
				this->new_state_default_class_ids.push_back(new_class->id);

				starting_scope_id = -1;
			} else {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family_id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
				this->last_layer_target_indexes.push_back(this->new_num_states-1);

				starting_scope_id = this->scope_context[this->sequence->input_furthest_layer_needed_in[i_index]-1];

				solution->scopes[starting_scope_id]->add_state(true,
															   new_family_id,
															   new_class->id);

				// special case last layer, as was skipped due to from ActionNode
				this->sequence->scope_additions_needed[i_index].insert(this->scope_context.back());
			}
		}

		for (set<pair<int, int>>::iterator it = this->sequence->scope_node_additions_needed[i_index].begin();
				it != this->sequence->scope_node_additions_needed[i_index].end(); it++) {
			this->sequence->scope_additions_needed[i_index].insert((*it).first);
		}

		for (set<int>::iterator it = this->sequence->scope_additions_needed[i_index].begin();
				it != this->sequence->scope_additions_needed[i_index].end(); it++) {
			if (*it != starting_scope_id) {
				solution->scopes[*it]->add_state(false,
												 new_family_id,
												 -1);
			}
		}

		for (set<pair<int, int>>::iterator it = this->sequence->scope_node_additions_needed[i_index].begin();
				it != this->sequence->scope_node_additions_needed[i_index].end(); it++) {
			Scope* outer_scope = solution->scopes[(*it).first];
			ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[(*it).second];
			Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
			scope_node->input_indexes.push_back(outer_scope->num_states-1);
			scope_node->input_target_layers.push_back(0);
			scope_node->input_target_indexes.push_back(inner_scope->num_states-1);
			scope_node->input_has_transform.push_back(false);
			scope_node->input_transformations.push_back(Transformation());
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
				it != this->sequence->state_networks.end(); it++) {
			Scope* scope = solution->scopes[it->first];
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				if (it->second[n_index].size() > 0) {
					if (it->second[n_index][i_index] != NULL) {
						ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
						it->second[n_index][i_index]->finalize_new_input(scope->num_states-1);
						// new state finalized below
						action_node->target_indexes.push_back(scope->num_states-1);
						action_node->state_networks.push_back(it->second[n_index][i_index]);
					}
				}
			}
		}
	}

	for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
		if (this->new_state_furthest_layer_needed_in[s_index] < (int)this->scope_context.size()+2) {
			FamilyDefinition* new_family = new FamilyDefinition((int)solution->families.size());
			solution->families.push_back(new_family);
			ClassDefinition* new_class = new ClassDefinition((int)solution->classes.size(),
															 new_family->id);
			solution->classes.push_back(new_class);

			for (int cc_index = 0; cc_index < (int)this->corr_calc_scope_depths.size(); cc_index++) {
				double pcc = this->corr_calc_covariances[cc_index][s_index]
					/ this->corr_calc_variances[cc_index]
					/ this->corr_calc_new_variances[cc_index][s_index];
				if (abs(pcc) > 0.6) {
					Scope* scope = solution->scopes[this->scope_context.size()-1 - this->corr_calc_scope_depths[cc_index]];
					FamilyDefinition* existing_family = solution->families[scope->state_family_ids[this->corr_calc_input_indexes[cc_index]]];

					existing_family->similar_family_ids.push_back(new_family->id);
					existing_family->transformations.push_back(this->new_transformations[cc_index][s_index].transformation);
					existing_family->pcc.push_back(pcc);

					new_family->similar_family_ids.push_back(existing_family->id);
					new_family->transformations.push_back(Transformation(this->new_transformations[cc_index][s_index].transformation));
					new_family->pcc.push_back(pcc);
				}
			}

			for (int i_index = 0; i_index < (int)this->sequence->input_types.size(); i_index++) {
				double pcc = this->sequence->corr_calc_new_covariances[s_index][i_index]
					/ this->sequence->corr_calc_state_variances[s_index]
					/ this->sequence->corr_calc_input_variances[s_index][i_index];
				if (abs(pcc) > 0.6) {
					int input_family_id = this->new_state_family_ids[i_index];
					FamilyDefinition* input_family = solution->families[input_family_id];

					// check in case added in previous step
					int new_input_index = -1;
					for (int f_index = 0; f_index < (int)input_family->similar_family_ids.size(); f_index++) {
						if (input_family->similar_family_ids[f_index] == new_family->id) {
							new_input_index = f_index;
							break;
						}
					}

					if (new_input_index == -1) {
						new_family->similar_family_ids.push_back(input_family_id);
						new_family->transformations.push_back(this->sequence->new_new_transformations[s_index][i_index].transformation);
						new_family->pcc.push_back(pcc);

						input_family->similar_family_ids.push_back(new_family->id);
						input_family->transformations.push_back(this->sequence->new_new_transformations[s_index][i_index].reverse());
						input_family->pcc.push_back(pcc);
					} else {
						if (abs(pcc) > abs(input_family->pcc[new_input_index])) {
							int new_state_index = -1;
							for (int f_index = 0; f_index < (int)new_family->similar_family_ids.size(); f_index++) {
								if (new_family->similar_family_ids[f_index] == input_family_id) {
									new_state_index = f_index;
									break;
								}
							}
							new_family->transformations[new_state_index] = this->sequence->new_new_transformations[s_index][i_index].transformation;
							new_family->pcc[new_state_index] = pcc;

							input_family->transformations[new_input_index] = this->sequence->new_new_transformations[s_index][i_index].reverse();
							input_family->pcc[new_input_index] = pcc;
						}
					}
				}
			}

			int starting_scope_id;
			if (this->new_state_furthest_layer_needed_in[s_index] == 0) {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family->id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
				this->last_layer_target_indexes.push_back(this->new_num_states-1);

				starting_scope_id = 0;

				solution->scopes[0]->add_state(true,
											   new_family->id,
											   new_class->id);

				// special case last layer, as was skipped due to from ActionNode
				this->scope_additions_needed[s_index].insert(this->scope_context.back());
			} else if (this->new_state_furthest_layer_needed_in[s_index] == (int)this->scope_context.size()+1) {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(true);
				this->new_state_family_ids.push_back(new_family->id);
				this->new_state_default_class_ids.push_back(new_class->id);

				starting_scope_id = -1;
			} else {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family->id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
				this->last_layer_target_indexes.push_back(this->new_num_states-1);

				starting_scope_id = this->scope_context[this->new_state_furthest_layer_needed_in[s_index]-1];

				solution->scopes[starting_scope_id]->add_state(true,
															   new_family->id,
															   new_class->id);

				// special case last layer, as was skipped due to from ActionNode
				this->scope_additions_needed[s_index].insert(this->scope_context.back());
			}

			for (set<pair<int, int>>::iterator it = this->scope_node_additions_needed[s_index].begin();
					it != this->scope_node_additions_needed[s_index].end(); it++) {
				this->scope_additions_needed[s_index].insert((*it).first);
			}

			for (set<int>::iterator it = this->scope_additions_needed[s_index].begin();
					it != this->scope_additions_needed[s_index].end(); it++) {
				if (*it != starting_scope_id) {
					solution->scopes[*it]->add_state(false,
													 new_family->id,
													 -1);
					// check scope_additions_needed to determine if score network should add input?
				}
			}

			for (set<pair<int, int>>::iterator it = this->scope_node_additions_needed[s_index].begin();
					it != this->scope_node_additions_needed[s_index].end(); it++) {
				Scope* outer_scope = solution->scopes[(*it).first];
				ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[(*it).second];
				Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
				scope_node->input_indexes.push_back(outer_scope->num_states-1);
				scope_node->input_target_layers.push_back(0);
				scope_node->input_target_indexes.push_back(inner_scope->num_states-1);
				scope_node->input_has_transform.push_back(false);
				scope_node->input_transformations.push_back(Transformation());
			}

			if (this->new_state_steps_needed_in[s_index][0]) {
				Scope* inner_scope = this->sequence->scopes[0];
				this->sequence->input_types.push_back(SEQUENCE_INPUT_TYPE_NONE);
				this->sequence->input_target_layers.push_back(0);
				this->sequence->input_target_indexes.push_back(inner_scope->num_states-1);
			}

			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
					it != this->state_networks.end(); it++) {
				Scope* scope = solution->scopes[it->first];
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						for (int is_index = 0; is_index < NUM_NEW_STATES; is_index++) {
							if (it->second[n_index][is_index] != NULL) {
								it->second[n_index][is_index]->finalize_new_state(
									s_index,
									scope->num_states-1);
							}
						}

						if (it->second[n_index][s_index] != NULL) {
							ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
							action_node->target_indexes.push_back(scope->num_states-1);
							action_node->state_networks.push_back(it->second[n_index][s_index]);
						}
					}
				}
			}

			for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
					it != this->score_networks.end(); it++) {
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index] != NULL) {
						it->second[n_index]->finalize_new_state(s_index);
					}
				}
			}

			this->continue_score_network->finalize_new_state(s_index);
			this->continue_misguess_network->finalize_new_state(s_index);
			this->halt_score_network->finalize_new_state(s_index);
			this->halt_misguess_network->finalize_new_state(s_index);

			for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
				if (this->exit_networks[e_index] != NULL) {
					this->exit_networks[e_index]->finalize_new_state(0,
																	 s_index,
																	 solution->scopes[this->scope_context.back()]->num_states-1);
				}
			}

			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequence->state_networks.begin();
					it != this->sequence->state_networks.end(); it++) {
				Scope* scope = solution->scopes[it->first];
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					for (int i_index = 0; i_index < (int)it->second[n_index].size(); i_index++) {
						if (it->second[n_index][i_index] != NULL) {
							it->second[n_index][i_index]->finalize_new_state(
								s_index,
								scope->num_states-1);
							// already added to node above
						}
					}
				}
			}
		}
	}

	for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
			it != this->score_networks.end(); it++) {
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			if (it->second[n_index] != NULL) {
				it->second[n_index]->cleanup_new_state();
			}
		}
	}

	this->continue_score_network->cleanup_new_state();
	this->continue_misguess_network->cleanup_new_state();
	this->halt_score_network->cleanup_new_state();
	this->halt_misguess_network->cleanup_new_state();

	this->state = EXPERIMENT_STATE_WRAPUP;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
