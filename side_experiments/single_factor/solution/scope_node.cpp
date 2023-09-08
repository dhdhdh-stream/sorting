#include "scope_node.h"

using namespace std;



void ScopeNode::activate(vector<double>& flat_vals,
						 vector<ContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 vector<vector<AbstractNodeHistory*>>& node_histories) {
	ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this);
	node_histories.back().push_back(scope_node_history);

	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<vector<double>> inner_state_vals(this->starting_node_ids.size());
	vector<vector<double>> inner_state_weights(this->starting_node_ids.size());

	inner_state_vals[0] = vector<double>(inner_scope->num_states, 0.0);
	inner_state_weights[0] = vector<double>(inner_scope->num_states, 0.0);
	for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
		inner_state_weights[0][inner_scope->initialized_locally_indexes[i_index]] = 1.0;
	}

	Scope* curr_scope = inner_scope;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
		inner_state_weights[1+l_index] = vector<double>(next_scope->num_states, 0.0);
		for (int i_index = 0; i_index < (int)next_scope->initialized_locally_indexes.size(); i_index++) {
			inner_state_weights[1+l_index][next_scope->initialized_locally_indexes[i_index]] = 1.0;
		}

		curr_scope = next_scope;
	}


	for (int s_index = 0; s_index < (int)this->input_target_layers.size(); s_index++) {
		if (this->input_target_layers[s_index] != -1) {
			if (context.back().state_weights[s_index] != 0.0) {
				inner_state_vals[this->input_target_layers[s_index]][this->input_target_indexes[s_index]] = context.back().state_vals->at(s_index);
				inner_state_weights[this->input_target_layers[s_index]][this->input_target_indexes[s_index]] = context.back().state_weights[s_index];
			}
		} else {
			scope_node_history->pre_obs_snapshot.push_back(context.back().state_vals->at(s_index));
		}
	}

	for (int n_index = 0; n_index < (int)this->pre_state_networks.size(); n_index++) {
		if (inner_state_weights[0][this->pre_state_network_indexes[n_index]] != 0.0) {
			this->pre_state_networks[n_index]->activate(scope_node_history->pre_obs_snapshot[this->pre_state_network_indexes[n_index]],
														inner_state_vals[0]->at(this->pre_state_network_target_indexes[n_index]));
		}
	}

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = &(inner_state_vals[0]);
	context.back().state_weights = inner_state_weights[0];
	inner_state_weights.erase(inner_state_weights.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	scope_node_history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
	}

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	inner_scope->activate(starting_node_ids_copy,
						  inner_state_vals_copy,
						  inner_state_weights,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	context.pop_back();

	context.back().node_id = -1;

	for (int s_index = 0; s_index < (int)this->input_target_layers.size(); s_index++) {
		if (context.back().state_weights[s_index] != 0.0) {
			context.back().state_vals->at(s_index) = inner_state_vals[this->input_target_layers[s_index]][this->input_target_indexes[s_index]];
		}
	}

	if (inner_exit_depth != -1) {
		for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
			scope_node_history->post_obs_snapshot.push_back(inner_state_vals[0][
				inner_scope->initialized_locally_indexes[i_index]]);
		}

		for (int n_index = 0; n_index < (int)this->post_state_networks.size(); n_index++) {
			if (context.back().state_weights[this->post_state_network_target_indexes[n_index]] != 0.0) {
				this->post_state_networks[n_index]->activate(scope_node_history->post_obs_snapshot[this->post_state_network_indexes[n_index]],
															 context.back().state_vals->at(this->post_state_network_target_indexes[n_index]));
			}
		}

		if (run_helper.phase == EXPLORE_PHASE_EXPERIMENT) {
			Explore* explore = run_helper.explore_history->explore;

			// HERE
			for (int s_index = 0; s_index < (int)this->experiment_hook_state_indexes.size(); s_index++) {
				bool matches_context = true;
				if (this->experiment_hook_scope_contexts[s_index].size() > context.size()) {
					matches_context = false;
				} else {
					for (int c_index = 0; c_index < (int)this->experiment_hook_scope_contexts[s_index].size()-1; c_index++) {
						if (this->experiment_hook_scope_contexts[s_index][c_index] != context[context.size() - this->experiment_hook_scope_contexts[s_index].size() + c_index].scope_id
								|| this->experiment_hook_node_contexts[s_index][c_index] != context[context.size() - this->experiment_hook_scope_contexts[s_index].size() + c_index].node_id) {
							matches_context = false;
							break;
						}
					}
				}

				if (matches_context) {
					StateNetwork* state_network = explore->state_networks[this->experiment_hook_state_indexes[s_index]][this->experiment_hook_network_indexes[s_index]];
					state_network->activate(flat_vals[0],
											run_helper.explore_history->state_vals[this->experiment_hook_state_indexes[s_index]]);
				}
			}

			if (this->experiment_hook_test_index != -1) {
				bool matches_context = true;
				if (this->experiment_hook_test_scope_contexts.size() > context.size()) {
					matches_context = false;
				} else {
					for (int c_index = 0; c_index < (int)this->experiment_hook_test_scope_contexts.size()-1; c_index++) {
						if (this->experiment_hook_test_scope_contexts[c_index] != context[context.size() - this->experiment_hook_test_scope_contexts.size() + c_index].scope_id
								|| this->experiment_hook_test_node_contexts[c_index] != context[context.size() - this->experiment_hook_test_scope_contexts.size() + c_index].node_id) {
							matches_context = false;
							break;
						}
					}
				}

				if (matches_context) {
					run_helper.explore_history->test_obs_vals[this->experiment_hook_test_index].push_back(flat_vals[0]);
				}
			}
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::explore_halfway_activate(vector<int>& starting_node_ids,
										 vector<vector<double>*>& starting_state_vals,
										 vector<vector<bool>>& starting_state_weights,
										 vector<double>& flat_vals,
										 vector<ContextLayer>& context,
										 int& inner_exit_depth,
										 int& inner_exit_node_id,
										 RunHelper& run_helper,
										 vector<vector<AbstractNodeHistory*>>& node_histories) {
	ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this);
	node_histories.back().push_back(scope_node_history);

	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	int furthest_matching_layer = 0;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
		if (l_index >= (int)starting_node_ids.size()
				|| starting_node_ids[l_index] != this->starting_node_ids[l_index]) {
			break;
		} else {
			furthest_matching_layer++;
		}
	}
	for (int s_index = 0; s_index < (int)this->input_target_layers.size(); s_index++) {
		if (this->input_target_layers[s_index] <= furthest_matching_layer) {
			if (context.back().state_weights[s_index] != 0.0) {
				starting_state_vals[this->input_target_layers[s_index]]->at(this->input_target_indexes[s_index]) = context.back().state_vals->at(s_index);
				starting_state_weights[this->input_target_layers[s_index]][this->input_target_indexes[s_index]] = context.back().state_weights[s_index];
			}
		} else {
			scope_node_history->pre_obs_snapshot.push_back(context.back().state_vals->at(s_index));
		}
	}

	for (int n_index = 0; n_index < (int)this->pre_state_networks.size(); n_index++) {
		if (starting_state_weights[0][this->pre_state_network_indexes[n_index]] != 0.0) {
			this->pre_state_networks[n_index]->activate(scope_node_history->pre_obs_snapshot[this->pre_state_network_indexes[n_index]],
														starting_state_vals[0]->at(this->pre_state_network_target_indexes[n_index]));
		}
	}

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];
	context.back().state_weights = starting_state_weights[0];
	starting_state_weights.erase(starting_state_weights.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	scope_node_history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<vector<double>*> inner_state_vals(starting_state_vals.size()-1);
	for (int l_index = 0; l_index < (int)starting_state_vals.size()-1; l_index++) {
		inner_state_vals[l_index] = starting_state_vals[1+l_index];
	}

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	inner_scope->activate(starting_node_ids,
						  inner_state_vals,
						  starting_state_weights,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	context.pop_back();

	context.back().node_id = -1;

	for (int s_index = 0; s_index < (int)this->input_target_layers.size(); s_index++) {
		if (this->input_target_layers[s_index] <= furthest_matching_layer) {
			if (context.back().state_weights[s_index] != 0.0) {
				context.back().state_vals->at(this->input_indexes[s_index]) = starting_state_vals[this->input_target_layers[s_index]]->at(this->input_target_indexes[s_index]);
			}
		}
	}

	if (inner_exit_depth != -1) {
		for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
			scope_node_history->post_obs_snapshot.push_back(starting_state_vals[0][
				inner_scope->initialized_locally_indexes[i_index]]);
		}

		for (int n_index = 0; n_index < (int)this->post_state_networks.size(); n_index++) {
			if (context.back().state_weights[this->post_state_network_target_indexes[n_index]] != 0.0) {
				this->post_state_networks[n_index]->activate(scope_node_history->post_obs_snapshot[this->post_state_network_indexes[n_index]],
															 context.back().state_vals->at(this->post_state_network_target_indexes[n_index]));
			}
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::backprop(double& scale_factor_error,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	double inner_scale_factor_error = 0.0;
	inner_scope->backprop(inner_scale_factor_error,
						  run_helper,
						  history->inner_scope_history);

	if (run_helper.phase == UPDATE_PHASE_NONE) {
		this->scope_scale_mod->backprop(inner_scale_factor_error, 0.0002);
	}
	scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}


