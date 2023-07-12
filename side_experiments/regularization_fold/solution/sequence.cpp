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
			if (this->experiment->state == EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
				input_vals[i_index] += context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
			} else if (this->experiment->state == EXPERIMENT_STATE_NEW_CLASS) {
				double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
				if (this->input_is_new_class[i_index]) {
					double new_class_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_vals[i_index] += new_class_scale*val;
				} else {
					input_vals[i_index] += val;
				}
			} else {
				// this->experiment->state == EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_class[i_index]) {
					// TODO: change when val is set?
					// - or simply make unable to depend on self
					//   - can depend on new state if absolutely need it
					input_vals[i_index] += context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]);
				}
			}
		} else {
			if (this->experiment->state == EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
				map<ClassDefinition*, double>::iterator it = run_helper.last_seen_vals.find(this->input_init_last_seen_classes[i_index]);
				if (it != run_helper.last_seen_vals.end()) {
					double last_seen_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_vals[i_index] += last_seen_scale*it->second;
				}
			}
		}
	}

	vector<vector<double>> inner_state_vals(this->starting_node_ids.size());
	vector<vector<bool>> inner_states_initialized(this->starting_node_ids.size());
	inner_state_vals[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	inner_states_initialized[0] = vector<double>(this->scopes[0]->num_states, false);
	Scope* curr_scope = this->scopes[0];
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
		inner_states_initialized[1+l_index] = vector<bool>(next_scope->num_states, false);

		curr_scope = next_scope;
	}
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		double val = input_vals[i_index];
		if (this->input_transformations[i_index] != NULL) {
			val = this->input_transformations[i_index]->forward(val);
		}
		inner_state_vals[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]] = val;
		
		inner_states_initialized[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]] = true;
	}

	context.push_back(ForwardContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().state_vals = &(inner_state_vals[0]);
	context.back().states_initialized = inner_states_initialized[0];
	inner_states_initialized.erase(inner_states_initialized.begin());

	history->node_histories = vector<vector<AbstractNetworkHistory*>>(this->scopes.size());

	if (this->starting_node_ids.size() > 1) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		starting_node_ids_copy.erase(starting_node_ids_copy.begin());

		vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
		}

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids_copy,
									 inner_state_vals_copy,
									 inner_states_initialized,
									 flat_vals,
									 context,
									 inner_exit_depth,
									 inner_exit_node_id,
									 run_helper,
									 node_history);
	}

	vector<int> next_starting_node_ids;
	vector<vector<double>*> next_starting_state_vals;
	vector<vector<bool>> next_starting_states_initialized;
	vector<EndingScopeNodeActivateHelper> ending_scope_node_helpers;
	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		int scope_id = this->scopes[l_index]->id;
		run_helper.scope_state_networks = &(this->experiment->state_networks[scope_id]);
		run_helper.scope_score_networks = &(this->experiment->score_networks[scope_id]);
		// initialize on experiment start

		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 1) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
				ending_scope_node_helpers.push_back(EndingScopeNodeActivateHelper(scope_node));
				ending_scope_node_helpers.back().forward(next_starting_node_ids,
														 next_starting_state_vals,
														 next_starting_states_initialized,
														 context,
														 run_helper);
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					history->node_histories[l_index].push_back(action_node_history);
					action_node->activate(flat_vals,
										  context,
										  run_helper,
										  action_node_history);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

					// unused
					int inner_exit_depth;
					int inner_exit_node_id;

					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
					history->node_histories[l_index].push_back(scope_node_history);
					if (next_starting_node_ids.size() > 0) {
						// note: first node in layer
						scope_node->halfway_activate(next_starting_node_ids,
													 next_starting_state_vals,
													 next_starting_states_initialized,
													 flat_vals,
													 context,
													 inner_exit_depth,
													 inner_exit_node_id,
													 run_helper,
													 scope_node_history);

						next_starting_node_ids.clear();
						next_starting_state_vals.clear();
						next_starting_states_initialized.clear();
					} else {
						scope_node->activate(flat_vals,
											 context,
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

	for (int e_index = (int)ending_scope_node_helpers.size()-1; e_index >= 0; e_index--) {
		ending_scope_node_helpers[e_index].backward(context,
													run_helper);
	}

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		double val = inner_state_vals[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]];
		if (this->input_transformations[i_index] != NULL) {
			val = this->input_transformations[i_index]->backward(val);
		}
		input_vals[i_index] = val;
	}

	context.pop_back();

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			if (this->experiment->state == EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
				context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]) = input_vals[i_index];
			} else if (this->experiment->state == EXPERIMENT_STATE_NEW_CLASS) {
				if (this->input_is_new_class[i_index]) {
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
				// this->experiment->state == EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_class[i_index]) {
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) = input_vals[i_index];
				}
			}
		}
	}
}

void Sequence::backprop(vector<BackwardContextLayer>& context,
						double& scale_factor_error,
						RunHelper& run_helper,
						SequenceHistory* history) {
	vector<double> input_errors(this->input_init_types.size(), 0.0);

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			if (this->experiment->state == EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
				input_errors[i_index] = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
					.state_errors->at(this->input_init_local_input_indexes[i_index]);
			} else if (this->experiment->state == EXPERIMENT_STATE_NEW_CLASS) {
				if (this->input_is_new_class[i_index]) {
					double new_class_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_errors[i_index] = new_class_scale*context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]);
				} else {
					input_errors[i_index] = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]);
				}
			} else {
				// this->experiment->state == EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_class[i_index]) {
					input_errors[i_index] = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]);
				}
			}
		}
	}

	vector<vector<double>> inner_state_errors(this->starting_node_ids.size());
	inner_state_errors[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	Scope* curr_scope = this->scopes[0];
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_errors[1+l_index] = vector<double>(next_scope->num_states, 0.0);

		curr_scope = next_scope;
	}
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		double error = input_errors[i_index];
		if (this->input_transformations[i_index] != NULL) {
			error = this->input_transformations[i_index]->backprop_backward(error);
		}
		inner_state_errors[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]] = error;
	}

	context.push_back(BackwardContextLayer());

	context.back().state_errors = &(inner_state_errors[0]);

	vector<int> next_starting_node_ids;
	vector<vector<double>*> next_starting_state_errors;
	vector<EndingScopeNodeBackpropHelper> ending_scope_node_helpers;
	for (int l_index = 0; l_index < (int)this->scopes.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];
		ending_scope_node_helpers.push_back(EndingScopeNodeBackpropHelper(scope_node));
		ending_scope_node_helpers.back().backward(next_starting_node_ids,
												  next_starting_state_errors,
												  context,
												  run_helper);
	}

	double cumulative_scale_factor_error = 0.0;
	for (int l_index = (int)this->scopes.size()-1; l_index >= 0; l_index--) {
		for (int n_index = (int)this->node_ids[l_index].size()-1; n_index >= 0; n_index--) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 1) {
				continue;	// i.e., break
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ending_scope_node_helpers[l_index].forward(context,
														   cumulative_scale_factor_error,
														   run_helper);
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_ACTION) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)history->node_histories[l_index][n_index];
					ActionNode* action_node = (ActionNode*)action_node_history->node;
					action_node->backprop(context,
										  cumulative_scale_factor_error,
										  run_helper,
										  action_node_history);
				} else {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[l_index][n_index];
					ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
					if (n_index == 0 && next_starting_node_ids.size() > 0) {
						scope_node->halfway_backprop(next_starting_node_ids,
													 next_starting_state_errors,
													 context,
													 cumulative_scale_factor_error,
													 run_helper,
													 scope_node_history);

						next_starting_node_ids.clear();
						next_starting_state_errors.clear();
					} else {
						scope_node->backprop(context,
											 cumulative_scale_factor_error,
											 run_helper,
											 scope_node_history);
					}
				}
			}
		}
	}

	if (this->starting_node_ids.size() > 1) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		starting_node_ids_copy.erase(starting_node_ids_copy.begin());

		vector<vector<double>*> inner_state_errors_copy(this->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			inner_state_errors_copy[l_index] = &(inner_state_errors[1+l_index]);
		}

		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[0][0];
		ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
		scope_node->halfway_backprop(starting_node_ids_copy,
									 inner_state_errors_copy,
									 context,
									 cumulative_scale_factor_error,
									 run_helper,
									 scope_node_history);
	}

	scale_factor_error = cumulative_scale_factor_error;

	context.pop_back();

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		double error = inner_state_errors[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]];
		if (this->input_transformations[i_index] != NULL) {
			error = this->input_transformations[i_index]->backprop_forward(error);
		}
		input_errors[i_index] = error;
	}

	// set back local errors in case used by previous sequence
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			if (this->experiment->state == EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {

			} else if (this->experiment->state == EXPERIMENT_STATE_NEW_CLASS) {

			} else {
				// this->experiment->state == EXPERIMENT_STATE_CLEANUP
				if (!this->input_is_new_class[i_index]) {
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]) = ;
				}
			}
		}
	}
	// HERE

	// TODO: set run_helper.new_input_errors
}

