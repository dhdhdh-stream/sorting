#include "sequence.h"

using namespace std;

Sequence::Sequence(vector<Scope*> scopes,
				   vector<int> starting_node_ids,
				   vector<int> input_init_types,
				   vector<int> input_init_target_layers,
				   vector<int> input_init_target_indexes,
				   vector<int> input_init_local_scope_depths,
				   vector<int> input_init_local_input_indexes,
				   vector<int> input_init_last_seen_class_ids,
				   vector<bool> input_has_transform,
				   vector<Transformation> input_transformations,
				   vector<vector<int>> node_ids) {
	this->scopes = scopes;
	this->starting_node_ids = starting_node_ids;
	this->input_init_types = input_init_types;
	this->input_init_target_layers = input_init_target_layers;
	this->input_init_target_indexes = input_init_target_indexes;
	this->input_init_local_scope_depths = input_init_local_scope_depths;
	this->input_init_local_input_indexes = input_init_local_input_indexes;
	this->input_init_last_seen_class_ids = input_init_last_seen_class_ids;
	this->input_has_transform = input_has_transform;
	this->input_transformations = input_transformations;
	this->node_ids = node_ids;
}

Sequence::~Sequence() {
	// handle in experiment transforms
}

void Sequence::activate(vector<double>& flat_vals,
						vector<ForwardContextLayer>& context,
						BranchExperimentHistory* branch_experiment_history,
						RunHelper& run_helper,
						SequenceHistory* history) {
	vector<double> input_vals(this->input_init_types.size(), 0.0);

	if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE) {
		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
				double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
				}
				input_vals[i_index] += val;
			} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LAST_SEEN) {
				map<int, double>::iterator it = run_helper.last_seen_vals.find(this->input_init_last_seen_class_ids[i_index]);
				if (it != run_helper.last_seen_vals.end()) {
					double val = it->second;
					if (this->input_has_transform[i_index]) {
						val = this->input_transformations[i_index].forward(val);
					}
					input_vals[i_index] += val;
				}
			}
		}
	} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
		int context_size_diff = (int)context.size() - (int)this->experiment->scope_context.size() - 1;
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			int context_index = c_index - context_size_diff;
			if (context_index < 0) {
				context_index = 0;
			}
			experiment_pre_activate_helper(context_index,
										   input_vals,
										   run_helper,
										   context[c_index].scope_history);
		}

		experiment_experiment_activate_helper(input_vals,
											  branch_experiment_history,
											  run_helper);

		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
				double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
				}
				input_vals[i_index] += val;
			} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LAST_SEEN) {
				map<int, double>::iterator it = run_helper.last_seen_vals.find(this->input_init_last_seen_class_ids[i_index]);
				if (it != run_helper.last_seen_vals.end()) {
					double val = it->second;
					if (this->input_has_transform[i_index]) {
						val = this->input_transformations[i_index].forward(val);
					}
					double last_seen_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
					input_vals[i_index] += last_seen_scale*val;
				}
			}
		}
	} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
		vector<int> temp_scope_context;
		vector<int> temp_node_context;
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			clean_pre_activate_helper(true,
									  temp_scope_context,
									  temp_node_context,
									  input_vals,
									  run_helper,
									  context[c_index].scope_history);

			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					if (c_index == context.size()-1 - this->input_init_local_scope_depths[i_index]) {
						if (!this->input_is_new_class[i_index]) {
							/**
							 * - if also used by previous sequence, value has already been set back
							 *   - (and in order to be new class, can't be any state updates until after previous sequence, so this is safe)
							 */
							double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
								.state_vals->at(this->input_init_local_input_indexes[i_index]);
							if (this->input_has_transform[i_index]) {
								val = this->input_transformations[i_index].forward(val);
							}
							double original_val_scale = this->experiment->state_iter/1000000.0;
							input_vals[i_index] += original_val_scale*val;
						}
					}
				}
			}
		}

		clean_experiment_activate_helper(temp_scope_context,
										 temp_node_context,
										 input_vals,
										 branch_experiment_history,
										 run_helper);

		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
				double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
				}
				double original_val_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
				input_vals[i_index] += original_val_scale*val;
			}
		}
	} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN) {
		int context_size_diff = (int)context.size() - (int)this->experiment->scope_context.size() - 1;
		vector<int> temp_scope_context;
		vector<int> temp_node_context;
		for (int c_index = 0; c_index < (int)context.size(); c_index++) {
			clean_pre_activate_helper(true,
									  temp_scope_context,
									  temp_node_context,
									  input_vals,
									  run_helper,
									  context[c_index].scope_history);

			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					if (c_index == context.size()-1 - this->input_init_local_scope_depths[i_index]) {
						if (!this->input_is_new_class[i_index]) {
							double val = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
								.state_vals->at(this->input_init_local_input_indexes[i_index]);
							if (this->input_has_transform[i_index]) {
								val = this->input_transformations[i_index].forward(val);
							}
							input_vals[i_index] += val;
						}
					}
				}
			}

			for (int cc_index = 0; cc_index < (int)this->experiment->corr_calc_scope_depths.size(); cc_index++) {
				if (c_index == context.size()-1 - this->experiment->corr_calc_scope_depths[cc_index]) {
					double curr_val = context[context.size()-1 - this->experiment->corr_calc_scope_depths[cc_index]].state_vals->at(this->experiment->corr_calc_input_indexes[cc_index]);
					for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
						if (this->input_init_types[i_index] != SEQUENCE_INPUT_INIT_LOCAL
								|| this->input_is_new_class[i_index]) {
							if (this->input_furthest_layer_needed_in[i_index] <= c_index - context_size_diff) {
								this->corr_calc_new_average_vals[cc_index][i_index] = 0.9999*this->corr_calc_new_average_vals[cc_index][i_index] + 0.0001*input_vals[i_index];
								double curr_new_variance = (this->corr_calc_new_average_vals[cc_index][i_index] - input_vals[i_index])*(this->corr_calc_new_average_vals[cc_index][i_index] - input_vals[i_index]);
								this->corr_calc_new_variances[cc_index][i_index] = 0.9999*this->corr_calc_new_variances[cc_index][i_index] + 0.0001*curr_new_variance;
								double curr_covariance = (this->experiment->corr_calc_average_vals[cc_index] - curr_val)*(this->corr_calc_new_average_vals[cc_index][i_index] - input_vals[i_index]);
								this->corr_calc_covariances[cc_index][i_index] = 0.9999*this->corr_calc_covariances[cc_index][i_index] + 0.0001*curr_covariance;
								this->new_transformations[cc_index][i_index].backprop(curr_val, input_vals[i_index]);
							}
						}
						/**
						 * - don't calculate correlation for non-new class
						 *   - should have already been calculated in previous experiments
						 */
					}
				}
			}
		}

		clean_experiment_activate_helper(temp_scope_context,
										 temp_node_context,
										 input_vals,
										 branch_experiment_history,
										 run_helper);

		for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
			this->corr_calc_state_average_vals[s_index] = 0.9999*this->corr_calc_state_average_vals[s_index] + 0.0001*run_helper.new_state_vals[s_index];
			double curr_state_variance = (this->corr_calc_state_average_vals[s_index] - run_helper.new_state_vals[s_index])*(this->corr_calc_state_average_vals[s_index] - run_helper.new_state_vals[s_index]);
			this->corr_calc_state_variances[s_index] = 0.9999*this->corr_calc_state_variances[s_index] + 0.0001*curr_state_variance;

			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (this->input_init_types[i_index] != SEQUENCE_INPUT_INIT_LOCAL
						|| this->input_is_new_class[i_index]) {
					this->corr_calc_input_average_vals[s_index][i_index] = 0.9999*this->corr_calc_input_average_vals[s_index][i_index] + 0.0001*input_vals[i_index];
					double curr_state_variance = (this->corr_calc_input_average_vals[s_index][i_index] - input_vals[i_index])*(this->corr_calc_input_average_vals[s_index][i_index] - input_vals[i_index]);
					this->corr_calc_input_variances[s_index][i_index] = 0.9999*this->corr_calc_input_variances[s_index][i_index] + 0.0001*curr_state_variance;
					double curr_covariance = (this->corr_calc_state_average_vals[s_index] - run_helper.new_state_vals[s_index])*(this->corr_calc_input_average_vals[s_index][i_index] - input_vals[i_index]);
					this->corr_calc_new_covariances[s_index][i_index] = 0.9999*this->corr_calc_new_covariances[s_index][i_index] + 0.0001*curr_covariance;
					this->new_new_transformations[s_index][i_index].backprop(run_helper.new_state_vals[s_index], input_vals[i_index]);
				}
			}
			/**
			 * - don't calculate correlation for non-new class
			 *   - already calculated in BranchExperiment
			 */
		}
	} else {
		// this->experiment->state == BRANCH_EXPERIMENT_STATE_WRAPUP
		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->last_layer_indexes[i_index] != -1) {
				input_vals[i_index] = context.back().state_vals[this->last_layer_indexes[i_index]];
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
		inner_state_vals[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]] = input_vals[i_index];
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
		run_helper.scope_state_networks = &(this->experiment->state_networks.find(scope_id)->second);
		run_helper.scope_score_networks = &(this->experiment->score_networks.find(scope_id)->second);
		// initialize on experiment start
		run_helper.scope_distance = (int)this->experiment->scope_context.size()+1;

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
		input_vals[i_index] = inner_state_vals[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]];
	}

	context.pop_back();

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE
					|| this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
				double val = input_vals[i_index];
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
				}
				context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
					.state_vals->at(this->input_init_local_input_indexes[i_index]) = val;
			} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
				double val = input_vals[i_index];
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
				}
				if (this->input_is_new_class[i_index]) {
					double val_diff = val - context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals[this->input_init_local_input_indexes[i_index]];

					double set_back_scale = (1000000.0-this->experiment->state_iter)/1000000.0;

					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) += set_back_scale*val_diff;
				} else {
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) = val;
				}
			} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN) {
				if (!this->input_is_new_class[i_index]) {
					double val = input_vals[i_index];
					if (this->input_has_transform[i_index]) {
						val = this->input_transformations[i_index].forward(val);
					}
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_vals->at(this->input_init_local_input_indexes[i_index]) = val;
				}
			} else {
				// this->experiment->state == BRANCH_EXPERIMENT_STATE_WRAPUP
				if (this->last_layer_indexes[i_index] != -1) {
					context.back().state_vals[this->last_layer_indexes[i_index]] = input_vals[i_index];
				}
			}
		}
	}
}

void Sequence::backprop(vector<BackwardContextLayer>& context,
						double& scale_factor_error,
						RunHelper& run_helper,
						SequenceHistory* history) {
	if (this->experiment->state == BRANCH_EXPERIMENT_STATE_WRAPUP) {
		for (int l_index = 0; l_index < (int)this->scopes.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];
			run_helper.scale_factor *= scope_node->scope_scale_mod->weight;
		}

		/**
		 * - can track separate inner_scale_factor_errors and update separate scope nodes/scale mods
		 *   - but will simply track cumulative for now, and let separate adjust after experiment
		 */
		double cumulative_scale_factor_error = 0.0;
		for (int l_index = (int)this->scopes.size()-1; l_index >= 0; l_index--) {
			for (int n_index = (int)this->node_ids[l_index].size()-1; n_index >= 0; n_index--) {
				if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

					cumulative_scale_factor_error *= scope_node->scope_scale_mod->weight;

					run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
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
						scope_node->backprop(context,
											 cumulative_scale_factor_error,
											 run_helper,
											 scope_node_history);
					}
				}
			}
		}

		scale_factor_error = cumulative_scale_factor_error;
	} else {
		vector<double> input_errors(this->input_init_types.size(), 0.0);

		for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
			if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
				if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
					double error = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]);
					if (this->input_has_transform[i_index]) {
						error = this->input_transformations[i_index].backprop_backward(error);
					}
					input_errors[i_index] = error;
				} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
					double error = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]);
					if (this->input_has_transform[i_index]) {
						error = this->input_transformations[i_index].backprop_backward(error);
					}
					if (this->input_is_new_class[i_index]) {
						double new_class_scale = (1000000.0-this->experiment->state_iter)/1000000.0;
						input_errors[i_index] = new_class_scale*error;
					} else {
						input_errors[i_index] = error;
					}
				} else {
					// this->experiment->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN
					if (!this->input_is_new_class[i_index]) {
						double error = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]]
							.state_errors->at(this->input_init_local_input_indexes[i_index]);
						if (this->input_has_transform[i_index]) {
							error = this->input_transformations[i_index].backprop_backward(error);
						}
						input_errors[i_index] = error;
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
			inner_state_errors[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]] = input_errors[i_index];
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
			input_errors[i_index] = inner_state_errors[this->input_init_target_layers[i_index]][this->input_init_target_indexes[i_index]];
		}

		if (this->experiment->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					double error = input_errors[i_index];
					if (this->input_has_transform[i_index]) {
						error = this->input_transformations[i_index].backprop_forward(error);
					}
					context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
						.state_errors->at(this->input_init_local_input_indexes[i_index]) = error;
				}
			}
		} else if (this->experiment->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					double error = input_errors[i_index];
					if (this->input_has_transform[i_index]) {
						error = this->input_transformations[i_index].backprop_forward(error);
					}
					if (this->input_is_new_class[i_index]) {
						double error_diff = error - context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
							.state_errors->at(this->input_init_local_input_indexes[i_index])

						double set_back_scale = (1000000.0-this->experiment->state_iter)/1000000.0;

						context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
							.state_errors->at(this->input_init_local_input_indexes[i_index]) += set_back_scale*error_diff;
					} else {
						context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
							.state_errors->at(this->input_init_local_input_indexes[i_index]) = error;
					}
				}
			}
		} else {
			// this->experiment->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN
			for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
				if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
					if (!this->input_is_new_class[i_index]) {
						double error = input_errors[i_index];
						if (this->input_has_transform[i_index]) {
							error = this->input_transformations[i_index].backprop_forward(error);
						}
						context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
							.state_errors->at(this->input_init_local_input_indexes[i_index]) = error;
					}
				}
			}
		}

		run_helper.new_input_errors[this->step_index] = input_errors;
	}
}

SequenceHistory::SequenceHistory(Sequence* sequence) {
	this->sequence = sequence;
}

SequenceHistory::~SequenceHistory() {
	for (int l_index = 0; l_index < (int)this->node_histories.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_histories[l_index].size(); n_index++) {
			delete this->node_histories[l_index][n_index];
		}
	}
}
