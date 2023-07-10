#include "sequence.h"

using namespace std;



void Sequence::activate(vector<double>& flat_vals,
						vector<ForwardContextLayer>& context,
						BranchExperimentHistory* branch_experiment_history,
						RunHelper& run_helper,
						SequenceHistory* history) {
	vector<double> input_vals(this->input_init_types.size(), 0.0);

	if (this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
		int context_size_diff = (int)context.size() - (int)this->scope_context.size();
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			int context_index = c_index - context_size_diff;
			if (context_index < 0) {
				context_index = 0;
			}
			experiment_outer_activate_helper(context_index,
											 input_vals,
											 run_helper,
											 context[c_index].scope_history);
		}

		experiment_experiment_activate_helper(input_vals,
											  branch_experiment_history,
											  run_helper,
											  history);
	} else if (this->experiment->state == EXPERIMENT_STATE_NEW_CLASSES) {
		new_classes_outer_activate_helper(input_vals,
										  run_helper,
										  context[c_index].scope_history);

		new_classes_experiment_activate_helper(input_vals,
											   branch_experiment_history,
											   run_helper,
											   history);
	} else if (this->experiment->state == EXPERIMENT_STATE_CLEANUP) {
		cleanup_outer_activate_helper(input_vals,
									  run_helper,
									  context[c_index].scope_history);

		cleanup_experiment_activate_helper(input_vals,
										   branch_experiment_history,
										   run_helper,
										   history);
	}

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_NONE) {
			// do nothing
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
				input_vals[i_index] += context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
			} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_NEW_CLASS) {
				double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
				if (this->input_is_new_type[i_index]) {
					double new_type_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_vals[i_index] += new_type_scale*val;
				} else {
					input_vals[i_index] += val;
				}
			} else {
				// this->experiment->state == BRANCH_EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_type[i_index]) {
					input_vals[i_index] += context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]);
				}
			}
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
			if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
				input_vals[i_index] += branch_experiment_history->sequence_ending_input_vals_snapshots[
					this->input_init_previous_step_indexes[i_index]][this->input_init_previous_input_indexes[i_index]];
			} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_NEW_CLASS) {
				double val = branch_experiment_history->sequence_ending_input_vals_snapshots[
					this->input_init_previous_step_indexes[i_index]][this->input_init_previous_input_indexes[i_index]];
				if (this->input_is_new_type[i_index]) {
					double new_type_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_vals[i_index] += new_type_scale*val;
				} else {
					input_vals[i_index] += val;
				}
			} else {
				// this->experiment->state == BRANCH_EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_type[i_index]) {
					input_vals[i_index] += branch_experiment_history->sequence_ending_input_vals_snapshots[
						this->input_init_previous_step_indexes[i_index]][this->input_init_previous_input_indexes[i_index]];
				}
			}
		} else {
			if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
				map<ClassDefinition*, double>::iterator it = run_helper.last_seen_vals.find(this->input_init_last_seen_classes[i_index]);
				if (it != run_helper.last_seen_vals.end()) {
					double last_seen_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_vals[i_index] += last_seen_scale*it->second;
				}
			}
		}
	}

	/**
	 * - can use existing context but won't bother for now
	 *   - anyways, a separate variable easily cleans itself up
	 */
	vector<ForwardContextLayer> sequence_context;
	sequence_context.push_back(ForwardContextLayer());
	sequence_context.back().scope_id = -1;
	sequence_context.back().node_id = -1;

	vector<vector<double>> state_vals(this->scopes.size());
	vector<vector<bool>> states_initialized(this->scopes.size());
	state_vals[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	states_initialized[0] = vector<bool>(this->scopes[0]->num_states, false);
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_layer[i_index] == 0) {
			double val = input_vals[i_index];
			if (this->input_transformations[i_index] != NULL) {
				val = this->input_transformations[i_index]->forward(val);
			}
			state_vals[0][this->input_init_target_index[i_index]] = val;
			states_initialized[0][this->input_init_target_index[i_index]] = true;
		}
	}
	sequence_context.back().state_vals = &(state_vals[0]);
	sequence_context.back().states_initialized = states_initialized[0];

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

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids_copy,
									 inner_starting_state_vals,
									 starting_states_initialized,
									 flat_vals,
									 sequence_context,
									 inner_exit_depth,
									 inner_exit_node_id,
									 run_helper,
									 node_history);

		// don't need starting_state_vals anymore as scope_node will set needed values back into context
	}

	vector<int> next_starting_node_ids;
	vector<vector<double>> next_starting_state_vals;
	vector<vector<bool>> next_starting_states_initialized;
	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		int scope_id = this->scopes[l_index]->id;
		run_helper.scope_state_networks = &(this->experiment->state_networks[scope_id]);
		run_helper.scope_score_networks = &(this->experiment->score_networks[scope_id]);
		// initialize on experiment start

		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 0) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

				run_helper.scale_factor *= scope_node->scope_scale_mod->weight;

				// don't bother appending to sequence_context, simply reuse first/last layer

				if (scope_node->scope_node_type == SCOPE_NODE_TYPE_NORMAL) {
					state_vals[l_index+1] = vector<double>(this->scopes[l_index+1]->num_states, 0.0);
					states_initialized[l_index+1] = vector<bool>(this->scopes[l_index+1]->num_states, false);
					for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
						// scope_node->input_target_layers[i_index] == 0
						if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
							if (states_initialized[l_index][scope_node->input_indexes[i_index]]) {
								double val = state_vals[l_index][scope_node->input_indexes[i_index]];
								if (scope_node->input_transformations[i_index] != NULL) {
									val = scope_node->input_transformations[i_index]->forward(val);
								}
								state_vals[l_index+1][scope_node->input_target_indexes[i_index]] = val;

								states_initialized[l_index+1][scope_node->input_target_indexes[i_index]] = true;
							}
						} else {
							states_initialized[l_index+1][scope_node->input_target_indexes[i_index]] = true;
						}
					}
					sequence_context.back().state_vals = &(state_vals[l_index+1]);
					sequence_context.back().states_initialized = states_initialized[l_index+1];
				} else if (scope_node->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_START) {
					state_vals[l_index+1] = vector<double>(scopes[l_index+1]->num_states, 0.0);
					states_initialized[l_index+1] = vector<bool>(scopes[l_index+1]->num_states, false);
					for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
						if (scope_node->input_target_layers[i_index] == 0) {
							if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
								if (states_initialized[l_index][scope_node->input_indexes[i_index]]) {
									double val = state_vals[l_index][scope_node->input_indexes[i_index]];
									if (scope_node->input_transformations[i_index] != NULL) {
										val = scope_node->input_transformations[i_index]->forward(val);
									}
									state_vals[l_index+1][scope_node->input_target_indexes[i_index]] = val;

									states_initialized[l_index+1][scope_node->input_target_indexes[i_index]] = true;
								}
							} else {
								states_initialized[l_index+1][scope_node->input_target_indexes[i_index]] = true;
							}
						}
					}
					sequence_context.back().state_vals = &(state_vals[l_index+1]);
					sequence_context.back().states_initialized = states_initialized[l_index+1];

					next_starting_node_ids = scope_node->starting_node_ids;
					next_starting_state_vals = vector<vector<double>>(scope_node->starting_node_ids.size()-1);
					next_starting_states_initialized = vector<vector<bool>>(scope_node->starting_node_ids.size()-1);

					Scope* curr_scope = solution->scopes[scope_node->inner_scope_id];
					for (int l_index = 0; l_index < scope_node->starting_node_ids.size()-1; l_index++) {
						ScopeNode* l_scope_node = (ScopeNode*)curr_scope->nodes[scope_node->starting_node_ids[l_index]];
						Scope* next_scope = solution->scopes[l_scope_node->inner_scope_id];

						next_starting_state_vals[l_index] = vector<double>(next_scope->num_states, 0.0);
						next_starting_states_initialized[l_index] = vector<bool>(next_scope->num_states, false);

						curr_scope = next_scope;
					}
					for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
						if (scope_node->input_target_layers[i_index] != 0) {
							if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
								if (states_initialized[l_index][scope_node->input_indexes[i_index]]) {
									double val = state_vals[l_index][scope_node->input_indexes[i_index]];
									if (scope_node->input_transformations[i_index] != NULL) {
										val = scope_node->input_transformations[i_index]->forward(val);
									}
									next_starting_state_vals[scope_node->input_target_layers[i_index]-1][scope_node->input_target_indexes[i_index]] = val;

									next_starting_states_initialized[scope_node->input_target_layers[i_index]-1][scope_node->input_target_indexes[i_index]] = true;
								}
							} else {
								next_starting_states_initialized[scope_node->input_target_layers[i_index]-1][scope_node->input_target_indexes[i_index]] = true;
							}
						}
					}
				} else {
					// scope_node->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_CONTINUE

					/**
					 * - also first node in layer
					 * 
					 * - previous set vals for this layer doesn't matter
					 *   - will not be used by any action nodes
					 *   - vals will be reset on layer end
					 *     - doesn't go through exit nodes, so impact that might have previously been set may be missed
					 *       - but hopefully, new state can make up for it
					 *   - (though the following sets vals for next layer)
					 */

					next_starting_node_ids.erase(next_starting_node_ids.begin());
					state_vals[l_index+1] = next_starting_state_vals.begin();
					sequence_context.back().state_vals = &(state_vals[l_index+1]);
					next_starting_state_vals.erase(next_starting_state_vals.begin());
					states_initialized[l_index+1] = next_starting_states_initialized.begin();
					sequence_context.back().states_initialized = states_initialized[l_index+1];
					next_starting_states_initialized.erase(next_starting_states_initialized.begin());
				}
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					history->node_histories[l_index].push_back(action_node_history);
					action_node->activate(flat_vals,
										  sequence_context,
										  run_helper,
										  action_node_history);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

					// unused
					int inner_exit_depth;
					int inner_exit_node_id;

					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
					history->node_histories[l_index].push_back(scope_node_history);
					if (scope_node->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_CONTINUE) {
						// note: first node in layer
						next_starting_node_ids.erase(next_starting_node_ids.begin());

						vector<vector<double>*> inner_starting_state_vals(next_starting_state_vals.size());
						for (int il_index = 0; il_index < (int)next_starting_state_vals.size(); il_index++) {
							inner_starting_state_vals[il_index] = &(next_starting_state_vals[il_index]);
						}

						scope_node->halfway_activate(next_starting_node_ids,
													 inner_starting_state_vals,
													 next_starting_states_initialized,
													 flat_vals,
													 sequence_context,
													 inner_exit_depth,
													 inner_exit_node_id,
													 run_helper,
													 scope_node_history);

						next_starting_node_ids.clear();
						next_starting_state_vals.clear();
						next_starting_states_initialized.clear();
					} else {
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

		run_helper.scope_state_networks = NULL;
		run_helper.scope_score_networks = NULL;
	}

	if (this->scopes.size() > 1) {
		// starting from second-to-last layer
		for (int l_index = (int)this->scopes.size()-2; l_index >= 0; l_index--) {
			ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];

			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_target_layers[i_index] == 0) {
					if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
						if (states_initialized[l_index][scope_node->input_indexes[i_index]]) {
							double val = state_vals[l_index+1][scope_node->input_target_indexes[i_index]];
							if (scope_node->input_transformations[i_index] != NULL) {
								val = scope_node->input_transformations[i_index]->backward(val);
							}
							state_vals[l_index][scope_node->input_indexes[i_index]] = val;
						}
					} else {
						ClassDefinition* inner_input_class = this->scopes[l_index+1]->default_state_classes[this->input_target_indexes[i_index]];
						run_helper.last_seen_vals[inner_input_class] = state_vals[l_index+1][scope_node->input_target_indexes[i_index]];
					}
				}
			}

			run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
		}
	}

	starting_state_vals[0] = state_vals[0];
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_layer[i_index] == 0) {
			double val = starting_state_vals[this->input_init_layer[i_index]][this->input_init_target_index[i_index]];
			if (this->input_transformations[i_index] != NULL) {
				val = this->input_transformations[i_index]->backward(val);
			}
			input_vals[i_index] = val;
		}
	}
	branch_experiment_history->sequence_ending_input_vals_snapshots[this->step_index] = input_vals;

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
				context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]) = input_vals[i_index];
			} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_NEW_CLASS) {
				if (this->input_is_new_type[i_index]) {
					double val_diff = input_vals[i_index] - context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals[this->input_init_local_input_indexes[i_index]];

					double set_back_scale = (1000000.0-this->experiment->state_iter)/1000000.0;

					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) += set_back_scale*val_diff;
				} else {
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) = input_vals[i_index];
				}
			} else {
				// this->experiment->state == BRANCH_EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_class[i_index]) {
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) = input_vals[i_index];
				}
			}
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
			bool path_has_new_type;
			if (this->input_is_new_type[i_index]) {
				path_has_new_type = true;
			}

			Sequence* curr_sequence = this->experiment->sequences[this->input_init_previous_step_indexes[i_index]];
			int curr_input_index = this->input_init_previous_input_indexes[i_index];
			while (true) {
				if (curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_NONE) {
					// do nothing
					break;
				} else if (curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE
							|| this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
						context[context.size()-1 - curr_sequence->input_init_local_scope_depths[curr_input_index]]
							.state_vals->at(curr_sequence->input_init_local_input_indexes[curr_input_index]) = input_vals[i_index];
					} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_NEW_CLASS) {
						if (path_has_new_type) {
							double val_diff = input_vals[i_index] - context[context.size()-1 - curr_sequence->input_init_local_scope_depths[curr_input_index]]
								.state_vals->at(curr_sequence->input_init_local_input_indexes[curr_input_index]);

							double set_back_scale = (1000000.0-this->experiment->state_iter)/1000000.0;

							context[context.size()-1 - curr_sequence->input_init_local_scope_depths[curr_input_index]]
								.state_vals->at(curr_sequence->input_init_local_input_indexes[curr_input_index]) += set_back_scale*val_diff;
						} else {
							context[context.size()-1 - curr_sequence->input_init_local_scope_depths[curr_input_index]]
								.state_vals->at(curr_sequence->input_init_local_input_indexes[curr_input_index]) = input_vals[i_index];
						}
					} else {
						// this->experiment->state == BRANCH_EXPERIMENT_STATE_CLEANUP
						if (!path_has_new_class) {
							context[context.size()-1 - curr_sequence->input_init_local_scope_depths[curr_input_index]]
								.state_vals[curr_sequence->input_init_local_input_indexes[curr_input_index]] = input_vals[i_index];
						}
					}

					break;
				} else if (curr_sequence->input_init_types[curr_input_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
					if (curr_sequence->input_is_new_class[curr_input_index]) {
						path_has_new_class = true;
					}

					Sequence* next_sequence = this->experiment->sequences[
						curr_sequence->input_init_previous_step_indexes[curr_input_index]];
					int next_input_index = curr_sequence->input_init_previous_input_indexes[curr_input_index];

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

void Sequence::backprop(vector<BackwardContextLayer>& context,
						double& scale_factor_error,
						RunHelper& run_helper,
						SequenceHistory* history) {
	/**
	 * - run_helper.new_input_errors already initialized in experiment
	 *   - if SEQUENCE_INPUT_INIT_LOCAL/SEQUENCE_INPUT_INIT_PREVIOUS, then already contains local error
	 *   - also may already contain errors from later step's SEQUENCE_INPUT_INIT_PREVIOUS
	 */

	vector<vector<double>> state_errors(this->scopes.size());
	state_errors[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_layer[i_index] == 0) {
			double error = run_helper.new_input_errors[this->step_index][i_index];
			if (this->input_transformations[i_index] != NULL) {
				error = this->input_transformations[i_index]->backprop_backward(error);
			}
			state_errors[0][this->input_init_target_index[i_index]] = error;
		}
	}
	vector<vector<vector<double>>> starting_state_errors(this->scopes.size());
	vector<vector<double>> next_starting_state_errors;
	for (int l_index = 0; l_index < (int)this->scopes.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

		run_helper.scale_factor *= scope_node->scope_scale_mod->weight;

		if (scope_node->scope_node_type == SCOPE_NODE_TYPE_NORMAL) {
			if (next_starting_state_errors.size() != 0) {
				starting_state_errors[l_index] = next_starting_state_errors;
				next_starting_state_errors.clear();
			}

			state_errors[l_index+1] = vector<double>(this->scopes[l_index+1]->num_states, 0.0);
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				// scope_node->input_target_layers[i_index] == 0
				if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
					double error = state_errors[l_index][scope_node->input_indexes[i_index]];
					if (scope_node->input_transformations[i_index] != NULL) {
						error = scope_node->input_transformations[i_index]->backprop_backward(error);
					}
					state_errors[l_index+1][scope_node->input_target_indexes[i_index]] = error;
				} else {
					// do nothing
				}
			}
		} else if (scope_node->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_START) {
			if (next_starting_state_errors.size() != 0) {
				starting_state_errors[l_index] = next_starting_state_errors;
				next_starting_state_errors.clear();
			}

			state_errors[l_index+1] = vector<double>(this->scopes[l_index+1]->num_states, 0.0);
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				// scope_node->input_target_layers[i_index] == 0
				if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
					double error = state_errors[l_index][scope_node->input_indexes[i_index]];
					if (scope_node->input_transformations[i_index] != NULL) {
						error = scope_node->input_transformations[i_index]->backprop_backward(error);
					}
					state_errors[l_index+1][scope_node->input_target_indexes[i_index]] = error;
				} else {
					// do nothing
				}
			}

			next_starting_state_errors = vector<vector<double>>(scope_node->starting_node_ids.size()-1);

			Scope* curr_scope = solution->scopes[scope_node->inner_scope_id];
			for (int l_index = 0; l_index < scope_node->starting_node_ids.size()-1; l_index++) {
				ScopeNode* l_scope_node = (ScopeNode*)curr_scope->nodes[scope_node->starting_node_ids[l_index]];
				Scope* next_scope = solution->scopes[l_scope_node->inner_scope_id];

				next_starting_state_errors[l_index] = vector<double>(next_scope->num_states, 0.0);

				curr_scope = next_scope;
			}
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_target_layers[i_index] != 0) {
					if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
						double error = state_errors[l_index][scope_node->input_indexes[i_index]];
						if (scope_node->input_transformations[i_index] != NULL) {
							error = scope_node->input_transformations[i_index]->backprop_backward(error);
						}
						next_starting_state_errors[scope_node->input_target_layers[i_index]-1][scope_node->input_target_indexes[i_index]] = error;
					} else {
						// do nothing
					}
				}
			}
		} else {
			// scope_node->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_CONTINUE
			state_errors[l_index+1] = next_starting_state_errors.begin();
			next_starting_state_errors.erase(next_starting_state_errors.begin());
		}
	}

	vector<BackwardContextLayer> sequence_context;
	sequence_context.push_back(BackwardContextLayer());

	double scale_factor_error_helper = 0.0;
	for (int l_index = (int)this->scopes.size()-1; l_index >= 0; l_index--) {
		sequence_context.back().state_errors = &(state_errors[l_index]);

		for (int n_index = (int)this->node_ids[l_index].size()-1; n_index >= 0; n_index--) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 0) {
				continue;	// i.e., break
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

				scale_factor_error_helper *= scope_node->scope_scale_mod->weight;

				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_target_layers[i_index] == 0) {
						if (scope_node->input_types[i_index] == INPUT_TYPE_EXISTING) {
							double error = state_errors[l_index+1][scope_node->input_target_indexes[i_index]];
							if (scope_node->input_transformations[i_index] != NULL) {
								error = scope_node->input_transformations[i_index]->backprop_forward(error);
							}
							state_errors[l_index][scope_node->input_indexes[i_index]] = error;
						}
					}
				}

				run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[n_index]]->type == NODE_TYPE_ACTION) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)history->node_histories[l_index][n_index];
					ActionNode* action_node = (ActionNode*)action_node_history->node;
					action_node->backprop(sequence_context,
										  scale_factor_error_helper,
										  run_helper,
										  action_node_history);
				} else {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[iter_index][h_index];
					ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
					if (scope_node->scope_node_type == SCOPE_NODE_TYPE_HALFWAY_CONTINUE) {
						// note: first node in layer

						vector<vector<double>*> inner_starting_state_errors(starting_state_errors[l_index].size());
						for (int il_index = 0; il_index < (int)starting_state_errors[l_index].size(); il_index++) {
							inner_starting_state_errors[il_index] = &(starting_state_errors[l_index][il_index]);
						}

						scope_node->halfway_backprop(inner_starting_state_errors,
													 sequence_context,
													 scale_factor_error_helper,
													 run_helper,
													 scope_node_history);
					} else {
						scope_node->backprop(sequence_context,
											 scale_factor_error_helper,
											 run_helper,
											 scope_node_history);
					}
				}
			}
		}
	}

	if (this->starting_node_ids.size() > 0) {
		vector<vector<double>> starting_state_errors(this->starting_node_ids.size());

		int curr_scope_id = ((ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]])->inner_scope_id;
		Scope* curr_scope = solution->scopes[curr_scope_id];
		for (int l_index = 0; l_index < this->starting_node_ids.size(); l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

			starting_state_errors[l_index] = vector<double>(next_scope->num_states, 0.0);

			curr_scope = next_scope;
		}
		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->input_init_layer[i_index] != 0) {
				double error = run_helper.new_input_errors[this->step_index][i_index];
				// HERE
				if (this->input_transformations[i_index] != NULL) {
					val = this->input_transformations[i_index]->forward(val);
				}
				starting_state_vals[this->input_init_layer[i_index]-1][this->input_init_target_index[i_index]] = val;
				starting_states_initialized[this->input_init_layer[i_index]-1][this->input_init_target_index[i_index]] = true;
			}
		}



	}

	scale_factor_error_helper = scale_factor_helper;

}


