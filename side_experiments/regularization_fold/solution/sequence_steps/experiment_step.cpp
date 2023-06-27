#include "sequence.h"

using namespace std;

void Sequence::experiment_outer_activate_helper(vector<double>& input_vals,
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
					it->second[node_id] = vector<StateNetwork*>(this->input_init_types.size());
					for (int ii_index = 0; ii_index < (int)this->input_init_types.size(); ii_index++) {
						it->second[node_id][ii_index] = new StateNetwork(scope_history->scope->num_states,
																		 NUM_NEW_STATES,
																		 1,
																		 20);
					}
				}

				// new_state_vals_snapshots already set

				action_node_history->experiment_sequence_step_indexes.push_back(this->step_index);
				action_node_history->input_vals_snapshots.push_back(input_vals);
				action_node_history->input_state_network_histories.push_back(vector<StateNetworkHistory*>(this->input_init_types.size(), NULL));
				for (int ii_index = 0; ii_index < (int)this->input_init_types.size(); ii_index++) {
					if (run_helper.can_zero && rand()%5 == 0) {
						// do nothing
					} else {
						StateNetwork* network = it->second[node_id][ii_index];
						StateNetworkHistory* network_history = new StateNetworkHistory(network);
						network->activate(action_node_history->obs_snapshot,
										  action_node_history->starting_state_vals_snapshot,
										  action_node_history->starting_new_state_vals_snapshot,
										  input_vals[ii_index],
										  network_history);
						action_node_history->input_state_network_histories.back()[ii_index] = network_history;
						input_vals[ii_index] += network->output->acti_vals[0];
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

void Sequence::experiment_experiment_activate_helper(vector<double>& input_vals,
													 BranchExperimentHistory* branch_experiment_history,
													 RunHelper& run_helper,
													 SequenceHistory* history) {
	BranchExperiment* experiment = branch_experiment_history->branch_experiment;

	history->step_input_vals_snapshots = vector<vector<double>>(this->step_index);
	history->step_state_network_histories = vector<vector<StateNetworkHistory*>>(this->step_index);
	history->sequence_input_vals_snapshots = vector<vector<vector<vector<double>>>>(this->step_index);
	history->sequence_state_network_histories = vector<vector<vector<vector<StateNetworkHistory*>>>>(this->step_index);
	for (int a_index = 0; a_index < this->step_index; a_index++) {
		if (experiment->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			history->step_input_vals_snapshots[a_index] = input_vals;
			history->step_state_network_histories[a_index] = vector<double>(this->input_init_types.size());
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (run_helper.can_zero && rand()%5 == 0) {
					// do nothing
				} else {
					StateNetwork* network = this->step_state_networks[a_index][i_index];
					StateNetworkHistory* network_history = new StateNetworkHistory(network);
					network->activate(branch_experiment_history->step_obs_snapshots[a_index],
									  branch_experiment_history->step_starting_new_state_vals_snapshots[a_index],
									  input_vals[i_index],
									  network_history);
					history->step_state_network_histories[a_index][i_index] = network_history;
					input_vals[i_index] += network->output->acti_vals[0];
				}
			}
		} else {
			SequenceHistory* sequence_history = branch_experiment_history->sequence_histories[a_index];

			history->sequence_input_vals_snapshots[a_index] = vector<vector<vector<double>>>(sequence_history->node_histories.size());
			history->sequence_state_network_histories[a_index] = vector<vector<vector<StateNetworkHistory*>>>(sequence_history->node_histories.size());
			for (int s_index = 0; s_index < (int)sequence_history->node_histories.size(); s_index++) {
				history->sequence_input_vals_snapshots[a_index][s_index] = vector<vector<double>>(sequence_history->node_histories[s_index].size());
				history->sequence_state_network_histories[a_index][s_index] = vector<vector<StateNetworkHistory*>>(sequence_history->node_histories[s_index].size());
				for (int n_index = 0; n_index < (int)sequence_history->node_histories[s_index].size(); n_index++) {
					if (sequence_history->node_histories[s_index][n_index]->node->type == NODE_TYPE_ACTION) {
						int node_id = sequence_history->node_histories[s_index][n_index]->node->id;
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)sequence_history->node_histories[s_index][n_index];

						history->sequence_input_vals_snapshots[a_index][s_index][n_index] = input_vals;
						history->sequence_state_network_histories[a_index][s_index][n_index] = vector<StateNetworkHistory*>(this->input_init_types.size(), NULL);
						for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
							if (run_helper.can_zero && rand()%5 == 0) {
								// do nothing
							} else {
								StateNetwork* network = this->sequence_state_networks[a_index][s_index][n_index][i_index];
								StateNetworkHistory* network_history = new StateNetworkHistory(network);
								network->activate(action_node_history->obs_snapshot,
												  action_node_history->starting_state_vals_snapshot,
												  action_node_history->starting_new_state_vals_snapshot,
												  input_vals[i_index],
												  network_history);
								history->sequence_state_network_histories[a_index][s_index][n_index][i_index] = network_history;
								input_vals[i_index] += network->output->acti_vals[0];
							}
						}
					} else {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)sequence_history->node_histories[s_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

						experiment_outer_activate_helper(input_vals,
														 branch_experiment_history->experiment->scope_context.size()+1,
														 run_helper,
														 scope_node_history->inner_scope_history);
					}
				}
			}
		}
	}
}

void Sequence::experiment_activate(vector<double>& flat_vals,
								   vector<ForwardContextLayer>& context,
								   BranchExperimentHistory* branch_experiment_history,
								   RunHelper& run_helper,
								   SequenceHistory* history) {
	vector<double> input_vals(this->input_init_types.size(), 0.0);

	int context_size_diff = (int)context.size() - (int)branch_experiment_history->experiment->scope_context.size();
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		experiment_outer_activate_helper(input_vals,
										 c_index - context_size_diff,
										 run_helper,
										 context[c_index].scope_history);
	}

	experiment_experiment_activate_helper(input_vals,
										  branch_experiment_history,
										  run_helper,
										  history);

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_NONE) {
			// do nothing
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			input_vals[i_index] += context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
				.state_vals[this->input_init_local_input_indexes[i_index]];
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
			input_vals[i_index] += branch_experiment_history->sequence_ending_input_vals_snapshots[
				this->input_init_previous_step_index[i_index]][this->input_init_previous_input_index[i_index]];
		} else {
			map<ClassDefinition*, double>::iterator it = run_helper.last_seen_vals.find(this->input_init_last_seen_classes[i_index]);
			if (it != run_helper.last_seen_vals.end()) {
				double last_seen_scale = (1000000.0-this->state_iter)/1000000.0;
				input_vals[i_index] += last_seen_scale*it->second;
			}
		}
	}

	vector<ForwardContextLayer> sequence_context;
	sequence_context.push_back(ForwardContextLayer());
	sequence_context.back().scope_id = -1;
	sequence_context.back().node_id = -1;

	vector<vector<double>> state_vals(this->scopes.size());
	state_vals[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	sequence_context.back().states_initialized = vector<bool>(this->scopes[0]->num_states, false);
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_layer[i_index] == 0) {
			double val = input_vals[i_index];
			if (this->input_transformations[i_index] != NULL) {
				val = this->input_transformations[i_index]->forward(val);
			}
			state_vals[0][this->input_init_target_index[i_index]] = val;
			sequence_context.back().states_initialized[this->input_init_target_index[i_index]] = true;
		}
	}
	sequence_context.back().state_vals = &(state_vals[0]);

	history->node_histories = vector<vector<AbstractNetworkHistory*>>(this->scopes.size());

	if (this->starting_node_ids.size() > 0) {
		vector<vector<double>> starting_state_vals(this->starting_node_ids.size());
		vector<vector<bool>> starting_states_initialized(this->starting_node_ids.size());

		int curr_scope_id = ((ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]])->inner_scope_id;
		Scope* curr_scope = solution->scopes[curr_scope_id];
		for (int l_index = 0; l_index < this->starting_node_ids.size(); l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

			starting_state_vals[l_index] = vector<double>(next_scope->num_states, 0.0);
			starting_states_initialized[l_index] = vector<bool>(next_scope->num_states,false);

			curr_scope = next_scope;
		}

		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->input_init_layer[i_index] != 0) {
				double val = input_vals[i_index];
				if (this->input_transformations[i_index] != NULL) {
					val = this->input_transformations[i_index]->forward(val);
				}
				starting_state_vals[this->input_init_layer[i_index]-1][this->input_init_target_index[i_index]] = val;
				starting_states_initialized[this->input_init_layer[i_index]-1][this->input_init_target_index[i_index]] = true;
			}
		}

		vector<int> starting_node_ids_copy = this->starting_node_ids;

		vector<*vector<double>> inner_starting_state_vals(this->starting_node_ids.size());
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
			inner_starting_state_vals[l_index] = &(starting_state_vals[l_index]);
		}

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids_copy,
									 inner_starting_state_vals,
									 starting_states_initialized,
									 flat_vals,
									 context,
									 exit_depth,
									 exit_node_id,
									 run_helper,
									 node_history);

		// don't need starting_state_vals anymore as scope_node will set needed values back into context
	}

	vector<vector<double>*> next_starting_state_vals;
	vector<vector<bool>>& next_starting_states_initialized;
	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 0) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

				run_helper.scale_factor *= scope_node->scope_scale_mod->weight;

				sequence_context.push_back(ForwardContextLayer);
				sequence_context.back().scope_id = -1;
				sequence_context.back().node_id = -1;

				// TODO: add to context, but always add -1
				// TODO: handle if halfway

				state_vals[l_index+1] = vector<double>(scopes[l_index+1]->num_states, 0.0);
				sequence_context.back().states_initialized = vector<bool>(scopes[l_index+1]->num_states, false);
				for (int i_index = 0; i_index < (int)scope_node->inner_input_indexes.size(); i_index++) {
					if (scope_node->inner_input_types[0])
					double val = state_vals[l_index][scope_node->inner_input_indexes[i_index]];
					if (scope_node->inner_input_transformations[i_index] != NULL) {
						val = scope_node->inner_input_transformations[i_index]->forward(val);
					}
					state_vals[l_index+1][scope_node->inner_input_target_indexes[i_index]] = val;

				}

				// HERE
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					history->node_histories[l_index].push_back(action_node_history);
					action_node->activate(flat_vals,
										  sequence_context
										  run_helper,
										  action_node_history);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

					// unused
					int inner_exit_depth;
					int inner_exit_node_id;

					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
					history->node_histories[l_index].push_back(scope_node_history);
					scope_node->activate(flat_vals,
										 sequence_context,
										 inner_exit_depth,
										 inner_exit_node_id,
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
				ClassDefinition* inner_input_class = scope_node->inner_input_classes[l_index+1][s_index];
				run_helper.last_seen_vals[inner_input_class] = state_vals[l_index+1][s_index];
			}

			run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
		}
	}

	starting_state_vals[0] = state_vals[0];
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		double val = starting_state_vals[this->input_init_layer[i_index]][this->input_init_target_index[i_index]];
		if (this->input_transformations[i_index] != NULL) {
			val = this->input_transformations[i_index]->backward(val);
		}
		input_vals[i_index] = val;
	}
	branch_experiment_history->sequence_ending_input_vals_snapshots[this->step_index] = input_vals;
	branch_experiment_history->sequence_input_classes_snapshots[this->step_index] = input_classes;

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
				.state_vals[this->input_init_local_input_indexes[i_index]] = input_vals[i_index];
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
			Sequence* curr_sequence = branch_experiment_history->experiment->sequences[this->input_init_previous_step_index[i_index]];
			int curr_input_index = this->input_init_previous_input_index[i_index];
			while (true) {
				if (curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_NONE) {
					// do nothing
					break;
				} else if (curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					context[context.size()-1 - curr_sequence->input_init_local_scope_depths[curr_input_index]]
						.state_vals[curr_sequence->input_init_local_input_indexes[curr_input_index]] = input_vals[i_index];

					break;
				} else if (curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
					Sequence* next_sequence = branch_experiment_history->experiment->sequences[
						curr_sequence->input_init_previous_step_index[curr_input_index]];
					int next_input_index = curr_sequence->input_init_previous_input_index[curr_input_index];

					curr_sequence = next_sequence;
					curr_input_index = next_input_index;

					// continue
				} else {
					// curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_LAST_SEEN
					// do nothing
					break;
				}
			}
		}
	}
}
