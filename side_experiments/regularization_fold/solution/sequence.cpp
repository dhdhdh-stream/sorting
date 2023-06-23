#include "sequence.h"

using namespace std;



void Sequence::explore_activate(vector<double>& flat_vals,
								vector<double>& new_state_vals,
								vector<StateDefinition*>& new_state_types,
								RunHelper& run_helper) {
	for (int l_index = 0; l_index < (int)this->last_seen_init_new_state_index.size(); l_index++) {
		double last_seen_val = run_helper.last_seen_vals.find(this->last_seen_init_types)->second;
		// always exists for explore
		if (this->last_seen_init_transformations[l_index] != NULL) {
			last_seen_val = this->last_seen_init_transformations[l_index]->forward(last_seen_val);
		}

		new_state_vals[this->last_seen_init_new_state_index[l_index]] = last_seen_val;
	}

	vector<vector<double>> state_vals(this->scopes.size());
	vector<vector<StateDefinition*>> state_types(this->scopes.size());

	state_vals[0] = vector<double>(scopes[0]->num_states, 0.0);
	state_types[0] = vector<StateDefinition*>(scopes[0]->num_states, NULL);
	for (int i_index = 0; i_index < (int)this->input_indexes[0].size(); i_index++) {
		double val = new_state_vals[this->input_indexes[0][i_index]];
		if (this->input_transformations[0][i_index] != NULL) {
			val = this->input_transformations[0][i_index]->forward(val);
		}
		state_vals[0][this->input_target_indexes[0][i_index]] = val;

		if (this->input_types[0][i_index] == NULL) {
			state_types[0][this->input_target_indexes[0][i_index]] = new_state_types[this->input_indexes[0][i_index]];
		} else {
			state_types[0][this->input_target_indexes[0][i_index]] = this->input_types[0][i_index];
		}
	}

	if (this->starting_node_ids.size() > 0) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		vector<vector<double>> starting_state_vals;
		vector<vector<StateDefinition*>> starting_state_types;
		for (int l_index = 1; l_index < (int)this->scopes.size(); l_index++) {
			starting_state_vals.push_back(vector<double>(scopes[l_index]->num_states, 0.0));
			starting_state_types.push_back(vector<StateDefinition*>(scopes[l_index]->num_states, NULL));
			for (int s_index = 0; s_index < (int)this->input_indexes[l_index].size(); s_index++) {
				double val = new_state_vals[this->input_indexes[l_index][s_index]];
				if (this->input_transformations[l_index][s_index] != NULL) {
					val = this->input_transformations[l_index][s_index]->forward(val);
				}
				starting_state_vals[this->input_target_indexes[l_index][s_index]] = val;

				if (this->input_types[l_index][s_index] == NULL) {
					starting_state_types[this->input_target_indexes[l_index][s_index]] = new_state_types[this->input_indexes[l_index][s_index]];
				} else {
					starting_state_types[this->input_target_indexes[l_index][s_index]] = this->input_types[l_index][s_index];
				}
			}
		}

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		vector<ContextLayer> context;
		int exit_depth;
		int exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		scope_node->halfway_activate(starting_node_ids_copy,
									 starting_state_vals,
									 starting_state_types,
									 flat_vals,
									 state_vals,
									 state_types,
									 context,
									 exit_depth,
									 exit_node_id,
									 run_helper,
									 node_history);
		delete node_history;
	}

	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 0) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

				run_helper.scale_factor *= scope_node->scope_scale_mod->weight;

				state_vals[l_index+1] = vector<double>(scopes[l_index+1]->num_states, 0.0);
				state_types[l_index+1] = vector<StateDefinition*>(scopes[l_index+1]->num_states, NULL);
				for (int i_index = 0; i_index < (int)scope_node->inner_input_indexes.size(); i_index++) {
					double val = state_vals[l_index][scope_node->inner_input_indexes[i_index]];
					if (scope_node->inner_input_transformations[i_index] != NULL) {
						val = scope_node->inner_input_transformations[i_index]->forward(val);
					}
					state_vals[l_index+1][scope_node->inner_input_target_indexes[i_index]] = val;

					if (scope_node->inner_input_types[i_index] == NULL) {
						state_types[l_index+1] = state_types[l_index][scope_node->inner_input_indexes[i_index]];
					} else {
						state_types[l_index+1] = scope_node->inner_input_types[i_index];
					}
				}

				// no need to update last_seen_vals
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					action_node->activate(flat_vals,
										  state_vals[l_index],
										  state_types[l_index],
										  run_helper,
										  action_node_history);
					delete action_node_history;
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

					vector<ContextLayer> context;
					int exit_depth;
					int exit_node_id;

					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
					scope_node->activate(flat_vals,
										 state_vals[l_index],
										 state_types[l_index],
										 context,
										 exit_depth,
										 exit_node_id,
										 run_helper,
										 scope_node_history);
					delete scope_node_history;
				}
			}
		}
	}

	if (this->scopes.size() > 1) {
		// starting from second-to-last layer to second layer
		for (int l_index = (int)this->scopes.size()-2; l_index >= 0; l_index--) {
			ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];

			for (int i_index = 0; i_index < (int)scope_node->inner_input_indexes.size(); i_index++) {
				double val = state_vals[l_index+1][scope_node->inner_input_target_indexes[i_index]];
				if (scope_node->inner_input_transformations[i_index] != NULL) {
					val = scope_node->inner_input_transformations[i_index]->backward(val);
				}
				state_vals[l_index][scope_node->inner_input_indexes[i_index]] = val;
			}

			for (int s_index = 0; s_index < (int)state_vals[l_index+1].size(); s_index++) {
				if (state_types[l_index+1][s_index] != NULL) {
					run_helper.last_seen_vals[state_types[l_index+1][s_index]] = state_vals[l_index+1][s_index];
				}
			}

			run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
		}
	}

	for (int i_index = 0; i_index < (int)this->input_indexes[0].size(); i_index++) {
		double val = state_vals[0][this->input_target_indexes[0][i_index]];
		if (this->input_transformations[0][i_index] != NULL) {
			val = this->input_transformations[0][i_index]->backward(val);
		}
		new_state_vals[this->input_indexes[0][i_index]] = val;
	}
}

void Sequence::experiment_outer_activate_helper(vector<vector<double>>& input_vals,
												int context_index,
												RunHelper& run_helper,
												ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	map<int, vector<vector<vector<StateNetwork*>>>>::iterator it = this->state_networks.find(scope_id);
	if (it == this->state_networks.end()) {
		it = this->state_networks.insert({scope_id, vector<vector<vector<StateNetwork*>>>()}).first;
	}

	int size_diff = (int)scope_history->scope->nodes.size() - (int)it->second.size();
	it->second.insert(it->second.end(), size_diff, vector<vector<StateNetwork*>>());

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				int node_id = scope_history->node_histories[i_index][h_index]->node->id;
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];

				if (it->second[node_id].size() == 0) {
					it->second[node_id] = vector<vector<StateNetwork*>>(this->input_init_types.size());
					for (int l_index = 0; l_index < (int)this->input_init_types.size(); l_index++) {
						it->second[node_id][l_index] = vector<StateNetwork*>(this->input_init_types[l_index].size());
						for (int i_index = 0; i_index < (int)this->input_init_types[l_index].size(); i_index++) {
							it->second[node_id][l_index][i_index] = new StateNetwork(scope_history->scope->num_states,
																					 NUM_NEW_STATES,
																					 20);
						}
					}
				}

				// new_state_vals_snapshots already set

				action_node_history->experiment_sequence_index.push_back(this->step_index);
				action_node_history->input_vals_snapshots.push_back(input_vals);
				action_node_history->input_state_network_histories.push_back(vector<vector<StateNetworkHistory*>>(this->input_init_types.size()));
				for (int l_index = 0; l_index < (int)this->input_init_types.size(); l_index++) {
					action_node_history->input_state_network_histories.back()[l_index] = vector<StateNetworkHistory*>(this->input_init_types[l_index].size(), NULL);
					for (int i_index = 0; i_index < (int)this->input_init_types[l_index].size(); i_index++) {
						if (run_helper.can_zero && rand()%5 == 0) {
							// do nothing
						} else {
							StateNetwork* network = it->second[node_id][l_index][i_index];
							StateNetworkHistory* network_history = new StateNetworkHistory(network);
							network->activate(action_node_history->obs_snapshot,
											  action_node_history->starting_state_vals_snapshot,
											  action_node_history->starting_new_state_vals_snapshot,
											  input_vals[l_index][i_index],
											  network_history);
							action_node_history->input_state_network_histories.back()[l_index][i_index] = network_history;
							input_vals[l_index][i_index] += network->output->acti_vals[0];
						}
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
					// do nothing
				} else {
					experiment_outer_activate_helper(input_vals,
													 context_index,
													 run_helper,
													 scope_node_history->inner_scope_history);
				}
			}
		}
	}
}

void Sequence::experiment_experiment_activate_helper(vector<vector<double>>& input_vals,
													 BranchExperimentHistory* branch_experiment_history,
													 RunHelper& run_helper,
													 SequenceHistory* history) {
	BranchExperiment* experiment = branch_experiment_history->branch_experiment;

	history->step_input_vals_snapshots = vector<vector<vector<double>>>(this->step_index);
	history->step_state_network_histories = vector<vector<vector<StateNetworkHistory*>>>(this->step_index);
	for (int a_index = 0; a_index < this->step_index; a_index++) {
		if (experiment->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			history->step_input_vals_snapshots[a_index] = input_vals;
			history->step_state_network_histories[a_index] = vector<vector<double>>(this->input_init_types.size());
			for (int l_index = 0; l_index < (int)this->input_init_types.size(); l_index++) {
				history->step_state_network_histories[a_index][l_index] = vector<double>(this->input_init_types[l_index].size(), NULL);
				for (int i_index = 0; i_index < (int)this->input_init_types[l_index].size(); i_index++) {
					if (run_helper.can_zero && rand()%5 == 0) {
						// do nothing
					} else {
						StateNetwork* network = this->step_state_networks[a_index][l_index][i_index];
						StateNetworkHistory* network_history = new StateNetworkHistory(network);
						network->activate(branch_experiment_history->step_obs_snapshots[a_index],
										  branch_experiment_history->step_starting_new_state_vals_snapshots[a_index],
										  input_vals[a_index][l_index],
										  network_history);
						history->step_state_network_histories[a_index][l_index][i_index] = network_history;
						input_vals[l_index][i_index] += network->output->acti_vals[0];
					}
				}
			}
		} else {
			SequenceHistory* sequence_history = branch_experiment_history->sequence_histories[a_index];
			for (int s_index = 0; s_index < (int)sequence_history->node_histories.size(); s_index++) {
				for (int n_index = 0; n_index < (int)sequence_history->node_histories[s_index].size(); n_index++) {
					if (sequence_history->node_histories[s_index][n_index]->node->type == NODE_TYPE_ACTION) {
						int node_id = sequence_history->node_histories[s_index][n_index]->node->id;
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)sequence_history->node_histories[s_index][n_index];

						history->sequence_input_vals_snapshots[a_index][s_index][n_index] = input_vals;
						history->sequence_state_network_histories[a_index][s_index][n_index] = vector<vector<double>>(this->input_init_types.size());
						for (int l_index = 0; l_index < (int)this->input_init_types.size(); l_index++) {
							history->sequence_state_network_histories[a_index][s_index][n_index][l_index] = vector<StateNetworkHistory*>(this->input_init_types[l_index].size(), NULL);
							for (int i_index = 0; i_index < (int)this->input_init_types[l_index].size(); i_index++) {
								if (run_helper.can_zero && rand()%5 == 0) {
									// do nothing
								} else {
									StateNetwork* network = this->sequence_state_networks[a_index][s_index][n_index][l_index][i_index];
									StateNetworkHistory* network_history = new StateNetworkHistory(network);
									network->activate(action_node_history->obs_snapshot,
													  action_node_history->starting_state_vals_snapshot,
													  action_node_history->starting_new_state_vals_snapshot,
													  input_vals[l_index][i_index],
													  network_history);
									history->sequence_state_network_histories[a_index][s_index][n_index][l_index][i_index] = network_history;
									input_vals[l_index][i_index] += network->output->acti_vals[0];
								}
							}
						}
					} else {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)sequence_history->node_histories[s_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

						experiment_outer_activate_helper(input_vals,
														 run_helper.experiment->scope_context.size()+1,
														 run_helper,
														 scope_node_history->inner_scope_history);
					}
				}
			}
		}
	}
}

void Sequence::experiment_activate(vector<double>& flat_vals,
								   RunHelper& run_helper,
								   BranchExperimentHistory* branch_experiment_history,
								   SequenceHistory* history) {
	vector<vector<double>> input_vals(this->input_init_types.size());
	for (int l_index = 0; l_index < (int)this->input_init_types.size(); l_index++) {
		input_vals[l_index] = vector<double>(this->input_init_types[l_index].size(), 0.0)
	}

	int context_size_diff = (int)context.size() - this->context_index;



	for (int l_index = 0; l_index < (int)this->last_seen_init_new_state_index.size(); l_index++) {
		map<StateDefinition*, double>::iterator it = run_helper.last_seen_vals.find(this->last_seen_init_types);
		if (it != run_helper.last_seen_vals.end()) {
			double last_seen_val = it->second;
			if (this->last_seen_init_transformations[l_index] != NULL) {
				last_seen_val = this->last_seen_init_transformations[l_index]->forward(last_seen_val);
			}

			double last_seen_scale = (1000000.0-this->state_iter)/1000000.0;

			run_helper.new_state_vals[this->last_seen_init_new_state_index[l_index]] += last_seen_scale*last_seen_val;
		}
	}

	vector<vector<double>> state_vals(this->scopes.size());
	vector<vector<StateDefinition*>> state_types(this->scopes.size());

	state_vals[0] = vector<double>(scopes[0]->num_states, 0.0);
	state_types[0] = vector<StateDefinition*>(scopes[0]->num_states, NULL);
	for (int i_index = 0; i_index < (int)this->input_indexes[0].size(); i_index++) {
		double val = run_helper.new_state_vals[this->input_indexes[0][i_index]];
		if (this->input_transformations[0][i_index] != NULL) {
			val = this->input_transformations[0][i_index]->forward(val);
		}
		state_vals[0][this->input_target_indexes[0][i_index]] = val;

		if (this->input_types[0][i_index] == NULL) {
			state_types[0][this->input_target_indexes[0][i_index]] = run_helper.new_state_types[this->input_indexes[0][i_index]];
		} else {
			state_types[0][this->input_target_indexes[0][i_index]] = this->input_types[0][i_index];
		}
	}

	history->node_histories = vector<vector<AbstractNetworkHistory*>>(this->scopes.size());

	if (this->starting_node_ids.size() > 0) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		vector<vector<double>> starting_state_vals;
		vector<vector<StateDefinition*>> starting_state_types;
		for (int l_index = 1; l_index < (int)this->scopes.size(); l_index++) {
			starting_state_vals.push_back(vector<double>(scopes[l_index]->num_states, 0.0));
			starting_state_types.push_back(vector<StateDefinition*>(scopes[l_index]->num_states, NULL));
			for (int s_index = 0; s_index < (int)this->input_indexes[l_index].size(); s_index++) {
				double val = new_state_vals[this->input_indexes[l_index][s_index]];
				if (this->input_transformations[l_index][s_index] != NULL) {
					val = this->input_transformations[l_index][s_index]->forward(val);
				}
				starting_state_vals[this->input_target_indexes[l_index][s_index]] = val;

				if (this->input_types[l_index][s_index] == NULL) {
					starting_state_types[this->input_target_indexes[l_index][s_index]] = new_state_types[this->input_indexes[l_index][s_index]];
				} else {
					starting_state_types[this->input_target_indexes[l_index][s_index]] = this->input_types[l_index][s_index];
				}
			}
		}

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		vector<ContextLayer> context;
		int exit_depth;
		int exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids_copy,
									 starting_state_vals,
									 starting_state_types,
									 flat_vals,
									 state_vals,
									 state_types,
									 context,
									 exit_depth,
									 exit_node_id,
									 run_helper,
									 node_history);
	}

	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 0) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

				run_helper.scale_factor *= scope_node->scope_scale_mod->weight;

				state_vals[l_index+1] = vector<double>(scopes[l_index+1]->num_states, 0.0);
				state_types[l_index+1] = vector<StateDefinition*>(scopes[l_index+1]->num_states, NULL);
				for (int i_index = 0; i_index < (int)scope_node->inner_input_indexes.size(); i_index++) {
					double val = state_vals[l_index][scope_node->inner_input_indexes[i_index]];
					if (scope_node->inner_input_transformations[i_index] != NULL) {
						val = scope_node->inner_input_transformations[i_index]->forward(val);
					}
					state_vals[l_index+1][scope_node->inner_input_target_indexes[i_index]] = val;

					if (scope_node->inner_input_types[i_index] == NULL) {
						state_types[l_index+1] = state_types[l_index][scope_node->inner_input_indexes[i_index]];
					} else {
						state_types[l_index+1] = scope_node->inner_input_types[i_index];
					}
				}

				// no need to update last_seen_vals
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					history->node_histories[l_index].push_back(action_node_history);
					action_node->activate(flat_vals,
										  state_vals[l_index],
										  state_types[l_index],
										  run_helper,
										  action_node_history);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

					vector<ContextLayer> context;
					int exit_depth;
					int exit_node_id;

					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
					history->node_histories[l_index].push_back(scope_node_history);
					scope_node->activate(flat_vals,
										 state_vals[l_index],
										 state_types[l_index],
										 context,
										 exit_depth,
										 exit_node_id,
										 run_helper,
										 scope_node_history);
				}
			}
		}
	}

	if (this->scopes.size() > 1) {
		// starting from second-to-last layer to second layer
		for (int l_index = (int)this->scopes.size()-2; l_index >= 0; l_index--) {
			ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];

			for (int i_index = 0; i_index < (int)scope_node->inner_input_indexes.size(); i_index++) {
				double val = state_vals[l_index+1][scope_node->inner_input_target_indexes[i_index]];
				if (scope_node->inner_input_transformations[i_index] != NULL) {
					val = scope_node->inner_input_transformations[i_index]->backward(val);
				}
				state_vals[l_index][scope_node->inner_input_indexes[i_index]] = val;
			}

			for (int s_index = 0; s_index < (int)state_vals[l_index+1].size(); s_index++) {
				if (state_types[l_index+1][s_index] != NULL) {
					run_helper.last_seen_vals[state_types[l_index+1][s_index]] = state_vals[l_index+1][s_index];
				}
			}

			run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
		}
	}

	for (int i_index = 0; i_index < (int)this->input_indexes[0].size(); i_index++) {
		double val = state_vals[0][this->input_target_indexes[0][i_index]];
		if (this->input_transformations[0][i_index] != NULL) {
			val = this->input_transformations[0][i_index]->backward(val);
		}
		run_helper.new_state_vals[this->input_indexes[0][i_index]] = val;
	}
}
