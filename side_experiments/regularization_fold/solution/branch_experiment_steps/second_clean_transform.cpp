#include "branch_experiment.h"

using namespace std;

void BranchExperiment::second_clean_transform() {
	this->new_num_states = 0;
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			this->sequences[a_index]->input_index_translations = vector<int>(this->sequences[a_index]->input_init_types.size(), -1);
		}
	}

	for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
		if (this->new_state_furthest_layer_needed_in[s_index] < this->scope_context.size()+2) {
			FamilyDefinition* new_family = new FamilyDefinition();
			new_family->id = solution->families.size();
			solution->families.push_back(new_family);
			ClassDefinition* new_class = new ClassDefinition(new_family->id);
			new_class->id = solution->classes.size();
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

			int starting_scope_id;
			if (this->new_state_furthest_layer_needed_in[s_index] == 0) {
				this->new_num_states++;
				this->new_state_initialized_locally.push_back(false);
				this->new_state_family_ids.push_back(new_family->id);
				this->new_state_default_class_ids.push_back(-1);
				this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
				this->last_layer_target_indexes.push_back(this->new_num_states-1);
				this->last_layer_has_transform.push_back(false);
				this->last_layer_transformations.push_back(Transformation());

				starting_scope_id = 0;

				solution->scopes[0]->add_state(true,
											   new_family->id,
											   new_class->id);

				// special case last layer, as was skipped due to from ActionNode
				this->scope_additions_needed[s_index].push_back(this->scope_context.back());
			} else if (this->new_state_furthest_layer_needed_in[s_index] == this->scope_context.size()+1) {
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
				this->last_layer_has_transform.push_back(false);
				this->last_layer_transformations.push_back(Transformation());

				starting_scope_id = this->scope_context[this->new_state_furthest_layer_needed_in[s_index]-1];

				solution->scopes[starting_scope_id]->add_state(true,
															   new_family->id,
															   new_class->id);

				// special case last layer, as was skipped due to from ActionNode
				this->scope_additions_needed[s_index].push_back(this->scope_context.back());
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
				}
			}

			for (set<pair<int, int>::iterator it = this->scope_node_additions_needed[s_index].begin();
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

			for (int a_index = 0; a_index < this->num_steps; a_index++) {
				if (this->new_state_steps_needed_in[s_index][a_index]) {
					Scope* inner_scope = this->sequences[a_index]->scopes[0];
					this->sequences[a_index]->input_init_types.push_back(SEQUENCE_INPUT_INIT_NEW_STATE);
					this->sequences[a_index]->input_init_target_layers.push_back(0);
					this->sequences[a_index]->input_init_target_indexes.push_back(inner_scope->num_states-1);

					this->sequences[a_index]->input_index_translations.push_back(this->new_num_states-1);
				}
			}
		}
	}

	// networks
	this->new_action_node_target_indexes = vector<vector<int>>(this->num_steps);
	this->new_action_node_state_networks = vector<vector<StateNetwork*>>(this->num_steps);

	for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
			it != this->state_networks.end(); it++) {
		Scope* scope = solution->scopes[it->first];

		int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
		int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			if (it->second[n_index].size() > 0) {
				ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
				for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
					if (it->second[n_index][s_index] != NULL) {
						it->second[n_index][s_index]->finalize_new_state(
							num_new_states,
							scope->num_states);
						action_node->target_indexes.push_back(scope->num_states - num_new_states + s_index);
						action_node->state_networks.push_back(it->second[n_index][s_index]);
					}
				}
			}
		}
	}

	for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
			it != this->score_networks.end(); it++) {
		Scope* scope = solution->scopes[it->first];
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			if (it->second[n_index] != NULL) {
				it->second[n_index]->finalize_new_state(scope->num_states);
			}
		}
	}

	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				if (this->step_state_networks[a_index][s_index] != NULL) {
					this->step_state_networks[a_index][s_index]->finalize_new_state(
						this->layer_num_new_states.back(),
						this->layer_num_new_states.back());
					this->new_action_node_target_indexes[a_index].push_back(s_index);
					this->new_action_node_state_networks[a_index].push_back(this->step_state_networks[a_index][s_index]);
				}
			}

			this->step_score_networks[a_index]->finalize_new_state(this->layer_num_new_states.back());
		}
	}

	this->starting_score_network->finalize_new_state(this->layer_num_new_states[this->scope_context.size()]);
	this->starting_misguess_network->finalize_new_state(this->layer_num_new_states[this->scope_context.size()]);

	for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
		if (this->exit_networks[e_index] != NULL) {
			this->exit_networks[e_index]->finalize_new_state(this->exit_depth,
															 this->layer_num_new_states[this->scope_context.size()],
															 this->scope_context.back()->num_states);
		}
	}

	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			// note: this->sequences[a_index]->input_init_types.size() currently does not match number of networks (i.e., initial input_init_types.size())
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
					it != this->sequences[a_index]->state_networks.end(); it++) {
				Scope* scope = solution->scopes[it->first];

				int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first);
				int num_new_states = this->layer_num_new_states[furthest_layer_seen_in];

				for (int n_index = 0; n_index < it->second.size(); n_index++) {
					for (int i_index = 0; i_index < (int)it->second[n_index].size(); i_index++) {
						if (it->second[n_index][i_index] != NULL) {
							it->second[n_index][i_index]->finalize_new_state(num_new_states,
																			 scope->num_states);
							// add to node below
						}
					}
				}
			}

			for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
				if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
					for (int i_index = 0; i_index < (int)this->sequences[a_index]->step_state_networks[ia_index].size(); i_index++) {
						if (this->sequences[a_index]->step_state_networks[ia_index][i_index] != NULL) {
							this->sequences[a_index]->step_state_networks[ia_index][i_index]->finalize_new_state(
								this->layer_num_new_states.back(),
								this->layer_num_new_states.back());
						}
					}
				}
			}
		}
	}

	// new inputs
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			int input_size = this->sequences[a_index]->input_init_types.size();
			for (int i_index = 0; i_index < input_size; i_index++) {
				bool new_input_needed = false;

				Scope* curr_scope = this->sequences[a_index]->scopes[0];
				for (int l_index = 0; l_index < this->sequences[a_index]->input_init_target_layers[i_index]; l_index++) {
					ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->sequences[a_index]->starting_node_ids[l_index]];
					curr_scope = solution->scopes[scope_node->inner_scope_id];
				}
				FamilyDefinition* new_family = solution->families[curr_scope->state_family_ids[this->sequences[a_index]->input_init_target_indexes[i_index]]];

				int starting_scope_id;
				if (this->sequences[a_index]->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL
						&& !this->sequences[a_index]->input_is_new_class[i_index]) {
					new_input_needed = true;

					/**
					 * - can add to scopes that already process original
					 *   - will be added as separate, so will be no conflict
					 *     - the 2 will just share the same family
					 */

					if (this->sequences[a_index]->input_init_local_scope_depths[i_index] == 0) {
						this->new_num_states++;
						this->new_state_initialized_locally.push_back(false);
						this->new_state_family_ids.push_back(new_family->id);
						this->new_state_default_class_ids.push_back(-1);
						this->last_layer_indexes.push_back(this->sequences[a_index]->input_init_local_input_indexes[i_index]);
						this->last_layer_target_indexes.push_back(this->new_num_states-1);
						this->last_layer_has_transform.push_back(this->sequences[a_index]->input_has_transform[i_index]);
						this->last_layer_transformations.push_back(this->sequences[a_index]->input_transformations[i_index]);

						this->sequences[a_index]->input_index_translations[i_index] = this->new_num_states-1;
					} else {
						this->new_num_states++;
						this->new_state_initialized_locally.push_back(false);
						this->new_state_family_ids.push_back(new_family->id);
						this->new_state_default_class_ids.push_back(-1);
						this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
						this->last_layer_target_indexes.push_back(this->new_num_states-1);
						this->last_layer_has_transform.push_back(false);
						this->last_layer_transformations.push_back(Transformation());

						this->sequences[a_index]->input_index_translations[i_index] = this->new_num_states-1;

						int continue_scope_id = this->scope_context[this->scope_context.size()-1 - this->sequences[a_index]->input_init_local_scope_depths[i_index]];
						Scope* continue_scope = solution->scopes[continue_scope_id];
						int continue_node_id = this->node_context[this->scope_context.size()-1 - this->sequences[a_index]->input_init_local_scope_depths[i_index]];
						ScopeNode* continue_scope_node = (ScopeNode*)continue_scope->nodes[continue_node_id];
						Scope* inner_scope = solution->scopes[continue_scope_node->inner_scope_id];

						continue_scope_node->input_indexes.push_back(this->sequences[a_index]->input_init_local_input_indexes[i_index]);
						continue_scope_node->input_target_layers.push_back(0);
						continue_scope_node->input_target_indexes.push_back(inner_scope->num_states);	// new state not added yet
						continue_scope_node->input_has_transform.push_back(this->sequences[a_index]->input_has_transform[i_index]);
						continue_scope_node->input_transformations.push_back(this->sequences[a_index]->input_transformations[i_index]);

						// special case last layer, as was skipped due to from ActionNode
						this->sequences[a_index]->scope_additions_needed[i_index].insert(this->scope_context.back());
					}

					starting_scope_id = -1;	// simply set to -1 for below
				} else if (this->sequences[a_index]->input_init_types[i_index] != SEQUENCE_INPUT_INIT_NEW_STATE) {
					if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index] < this->scope_context.size()+2) {
						new_input_needed = true;

						ClassDefinition* new_class = new ClassDefinition(new_family->id);
						new_class->id = solution->classes.size();
						solution->classes.push_back(new_class);

						for (int cc_index = 0; cc_index < (int)this->corr_calc_scope_depths.size(); cc_index++) {
							double pcc = this->sequences[a_index]->corr_calc_covariances[cc_index][i_index]
								/ this->corr_calc_variances[cc_index]
								/ this->sequences[a_index]->corr_calc_new_variances[cc_index][i_index];
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
									existing_family->similar_family_ids.push_back(new_family->id);
									existing_family->transformations.push_back(this->sequences[a_index]->new_transformations[cc_index][i_index]);
									existing_family->pcc.push_back(pcc);

									new_family->similar_family_ids.push_back(existing_family->id);
									new_family->transformations.push_back(Transformation(this->sequences[a_index]->new_transformations[cc_index][i_index]));
									new_family->pcc.push_back(pcc);
								} else {
									if (abs(pcc) > abs(new_family->pcc[new_existing_index])) {
										int existing_existing_index = -1;
										for (int f_index = 0; f_index < (int)existing_family->similar_family_ids.size(); f_index++) {
											if (existing_family->similar_family_ids[f_index] == new_family->id) {
												existing_existing_index = f_index;
												break;
											}
										}
										existing_family->transformations[existing_existing_index] = this->sequences[a_index]->new_transformations[cc_index][i_index];
										existing_family->pcc[existing_existing_index] = pcc;

										new_family->transformations[new_existing_index] = Transformation(this->sequences[a_index]->new_transformations[cc_index][i_index]);
										new_family->pcc[new_existing_index] = pcc;
									}
								}
							}
						}

						for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
							double pcc = this->sequences[a_index]->corr_calc_new_covariances[s_index][i_index]
								/ this->sequences[a_index]->corr_calc_state_variances[s_index]
								/ this->sequences[a_index]->corr_calc_input_variances[s_index][i_index];
							if (abs(pcc) > 0.6) {
								/**
								 * - safe to simply fetch from new_state_family_ids
								 *   - if abs(pcc) > 0.6, then has to be added in sequence at s_index
								 */
								FamilyDefinition* new_state_family = solution->families[this->new_state_family_ids[s_index]];

								// check in case added in previous step
								int new_existing_index = -1;
								for (int f_index = 0; f_index < (int)new_family->similar_family_ids.size(); f_index++) {
									if (new_family->similar_family_ids[f_index] == new_state_family->id) {
										new_existing_index = f_index;
										break;
									}
								}

								if (new_existing_index == -1) {
									new_state_family->similar_family_ids.push_back(new_family->id);
									new_state_family->transformations.push_back(this->sequences[a_index]->new_new_transformations[s_index][i_index]);
									new_state_family->pcc.push_back(pcc);

									new_family->similar_family_ids.push_back(new_state_family->id);
									new_family->transformations.push_back(Transformation(this->sequences[a_index]->new_new_transformations[s_index][i_index]));
									new_family->pcc.push_back(pcc);
								} else {
									if (abs(pcc) > abs(new_family->pcc[new_existing_index])) {
										int new_state_existing_index = -1;
										for (int f_index = 0; f_index < (int)new_state_family->similar_family_ids.size(); f_index++) {
											if (new_state_family->similar_family_ids[f_index] == new_family->id) {
												new_state_existing_index = f_index;
												break;
											}
										}
									}
									new_state_family->transformations[new_state_existing_index] = this->sequences[a_index]->new_new_transformations[s_index][i_index];
									new_state_family->pcc[new_state_existing_index] = pcc;

									new_family->transformations[new_existing_index] = Transformation(this->sequences[a_index]->new_new_transformations[s_index][i_index]);
									new_family->pcc[new_existing_index] = pcc;
								}
							}
						}

						if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index] == 0) {
							this->new_num_states++;
							this->new_state_initialized_locally.push_back(false);
							this->new_state_family_id.push_back(new_family->id);
							this->new_state_default_class_ids.push_back(-1);
							this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
							this->last_layer_target_indexes.push_back(this->new_num_states-1);
							this->last_layer_has_transform.push_back(false);
							this->last_layer_transformations.push_back(Transformation());

							this->sequences[a_index]->input_index_translations[i_index] = this->new_num_states-1;

							starting_scope_id = 0;

							solution->scopes[0]->add_state(true,
														   new_family->id,
														   new_class->id);

							// special case last layer, as was skipped due to from ActionNode
							this->sequences[a_index]->scope_additions_needed[i_index].push_back(this->scope_context.back());
						} else if (this->sequences[a_index]->input_furthest_layer_needed_in[i_index] == this->scope_context.size()+1) {
							this->new_num_states++;
							this->new_state_initialized_locally.push_back(true);
							this->new_state_family_ids.push_back(new_family->id);
							this->new_state_default_class_ids.push_back(new_class->id);

							this->sequences[a_index]->input_index_translations[i_index] = this->new_num_states-1;

							starting_scope_id = -1;
						} else {
							this->new_num_states++;
							this->new_state_initialized_locally.push_back(false);
							this->new_state_family_ids.push_back(new_family->id);
							this->new_state_default_class_ids.push_back(-1);
							this->last_layer_indexes.push_back(solution->scopes[this->scope_context.back()]->num_states);	// new state not added yet
							this->last_layer_target_indexes.push_back(this->new_num_states-1);
							this->last_layer_has_transform.push_back(false);
							this->last_layer_transformations.push_back(Transformation());

							this->sequences[a_index]->input_index_translations[i_index] = this->new_num_states-1;

							starting_scope_id = this->scope_context[this->sequences[a_index]->input_furthest_layer_needed_in[i_index]-1];

							solution->scopes[starting_scope_id]->add_state(true,
																		   new_family->id,
																		   new_class->id);

							// special case last layer, as was skipped due to from ActionNode
							this->sequences[a_index]->scope_additions_needed[i_index].push_back(this->scope_context.back());
						}
					}
				}

				if (new_input_needed) {
					for (set<pair<int, int>>::iterator it = this->sequences[a_index]->scope_node_additions_needed[i_index].begin();
							it != this->sequences[a_index]->scope_node_additions_needed[i_index].end(); it++) {
						this->sequences[a_index]->scope_additions_needed[i_index].insert((*it).first);
					}

					for (set<int>::iterator it = this->sequences[a_index]->scope_additions_needed[i_index].begin();
							it != this->sequences[a_index]->scope_additions_needed[i_index].end(); it++) {
						if (*it != starting_scope_id) {
							solution->scopes[*it]->add_state(false,
															 new_family->id,
															 -1);
						}
					}

					for (set<pair<int, int>>::iterator it = this->sequences[a_index]->scope_node_additions_needed[i_index].begin();
							it != this->sequences[a_index]->scope_node_additions_needed[i_index].end(); it++) {
						Scope* outer_scope = solution->scopes[(*it).first];
						ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[(*it).second];
						Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
						scope_node->input_indexes.push_back(outer_scope->num_states-1);
						scope_node->input_target_layers.push_back(0);
						scope_node->input_target_indexes.push_back(inner_scope->num_states-1);
						scope_node->input_has_transform.push_back(false);
						scope_node->input_transformations.push_back(Transformation());
					}

					for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
						if (this->sequences[a_index]->input_steps_needed_in[i_index][ia_index]) {
							Scope* inner_scope = this->sequences[ia_index]->scopes[0];
							this->sequences[ia_index]->input_init_types.push_back(SEQUENCE_INPUT_INIT_NEW_STATE);
							this->sequences[ia_index]->input_init_target_layers.push_back(0);
							this->sequences[ia_index]->input_init_target_indexes.push_back(inner_scope->num_states-1);

							this->sequences[ia_index]->input_index_translations.push_back(this->new_num_states-1);
						}
					}

					for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->sequences[a_index]->state_networks.begin();
							it != this->sequences[a_index]->state_networks.end(); it++) {
						Scope* scope = solution->scopes[it->first];
						for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
							if (it->second[n_index].size() > 0) {
								if (it->second[n_index][i_index] != NULL) {
									ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
									it->second[n_index][i_index]->finalize_new_input(scope->num_states-1);
									action_node->target_indexes.push_back(scope->num_states-1);
									action_node->state_networks.push_back(it->second[n_index][i_index]);
								}
							}
						}
					}

					for (int ia_index = 0; ia_index < this->num_steps; ia_index++) {
						if (this->step_types[ia_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
							if (this->sequences[a_index]->step_state_networks[ia_index][i_index] != NULL) {
								this->sequences[a_index]->step_state_networks[ia_index][i_index]->finalize_new_input(this->new_num_states-1);
								this->new_action_node_target_indexes[a_index].push_back(this->new_num_states-1);
								this->new_action_node_state_networks[a_index].push_back(this->sequences[a_index]->step_state_networks[ia_index][i_index]);
							}
						}
					}

					this->starting_score_network->add_state();
					this->starting_misguess_network->add_state();
				}
			}
		}
	}

	this->starting_original_score_network = new ScoreNetwork(this->starting_score_network->state_size,
															 0,
															 20);
	this->starting_original_misguess_network = new ScoreNetwork(this->starting_misguess_network->state_size,
																0,
																20);

	this->state = BRANCH_EXPERIMENT_STATE_WRAPUP;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
