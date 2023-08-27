#include "scope_node.h"

using namespace std;



void ScopeNode::activate(vector<vector<double>>& flat_vals,
						 vector<ForwardContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<vector<double>> inner_state_vals(this->starting_node_ids.size());
	vector<vector<bool>> inner_states_initialized(this->starting_node_ids.size());
	vector<vector<bool>> inner_is_learn_existing(this->starting_node_ids.size());

	inner_state_vals[0] = vector<double>(inner_scope->num_states, 0.0);
	inner_states_initialized[0] = vector<bool>(inner_scope->num_states, false);
	for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
		inner_states_initialized[0][inner_scope->initialized_locally_indexes[i_index]] = true;
	}
	inner_is_learn_existing[0] = vector<bool>(inner_scope->num_states, false);

	Scope* curr_scope = inner_scope;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
		inner_states_initialized[1+l_index] = vector<bool>(next_scope->num_states, false);
		for (int i_index = 0; i_index < (int)next_scope->initialized_locally_indexes.size(); i_index++) {
			inner_states_initialized[1+l_index][next_scope->initialized_locally_indexes[i_index]] = true;
		}
		inner_is_learn_existing[1+l_index] = vector<bool>(next_scope->num_states, false);

		curr_scope = next_scope;
	}

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (context.back().states_initialized[this->input_indexes[i_index]]) {
			double val = context.back().state_vals->at(this->input_indexes[i_index]);
			if (this->input_has_transform[i_index]) {
				val = this->input_transformations[i_index].forward(val);
			}
			inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = val;

			inner_states_initialized[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = true;
			inner_is_learn_existing[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] =
				context.back().is_learn_existing[this->input_indexes[i_index]];
		}
	}

	for (int i_index = 0; i_index < (int)this->pre_obs_state_indexes.size(); i_index++) {
		history->pre_obs_snapshot.push_back(context.back().state_vals
			->at(this->pre_obs_state_indexes[i_index]));
	}

	for (int n_index = 0; n_index < (int)this->pre_state_networks.size(); n_index++) {
		int state_index = this->pre_state_network_indexes[n_index];
		if (inner_states_initialized[0][state_index]) {
			if (inner_is_learn_existing[0][state_index]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->pre_state_networks[n_index]);
				this->pre_state_networks[n_index]->activate(history->pre_obs_snapshot,
															inner_state_vals[0]->at(state_index),
															network_history);
				history->pre_state_network_indexes.push_back(state_index);
				history->pre_state_network_histories.push_back(network_history);
			} else {
				this->pre_state_networks[n_index]->activate(history->pre_obs_snapshot,
															inner_state_vals[0]->at(state_index));
			}
		}
	}

	context.back().node_id = this->id;

	context.push_back(ForwardContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = &(inner_state_vals[0]);
	context.back().states_initialized = inner_states_initialized[0];
	inner_states_initialized.erase(inner_states_initialized.begin());
	context.back().is_learn_existing = inner_is_learn_existing[0];
	inner_is_learn_existing.erase(inner_is_learn_existing.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
	}

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	inner_scope->activate(starting_node_ids_copy,
						  inner_state_vals_copy,
						  inner_states_initialized,
						  inner_is_learn_existing,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	context.pop_back();

	context.back().node_id = -1;

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (context.back().states_initialized[this->input_indexes[i_index]]) {
			double val = inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
			if (this->input_has_transform[i_index]) {
				val = this->input_transformations[i_index].backward(val);
			}
			context.back().state_vals->at(this->input_indexes[i_index]) = val;
		}
	}

	for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
		history->post_obs_snapshot.push_back(inner_state_vals[0][
			inner_scope->initialized_locally_indexes[i_index]]);
	}

	for (int n_index = 0; n_index < (int)this->post_state_networks.size(); n_index++) {
		int state_index = this->post_state_network_indexes[n_index];
		if (context.back().states_initialized[state_index]) {
			if (context.back().is_learn_existing[state_index]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->post_state_networks[n_index]);
				this->post_state_networks[n_index]->activate(history->post_obs_snapshot,
															 context.back().state_vals->at(state_index),
															 network_history);
				history->state_network_indexes.push_back(state_index);
				history->state_network_histories.push_back(network_history);
			} else {
				this->post_state_networks[n_index]->activate(history->post_obs_snapshot,
															 context.back().state_vals->at(state_index));
			}
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::halfway_activate(vector<int>& starting_node_ids,
								 vector<vector<double>*>& starting_state_vals,
								 vector<vector<bool>>& starting_states_initialized,
								 vector<vector<bool>>& starting_is_learn_existing,
								 vector<vector<double>>& flat_vals,
								 vector<ForwardContextLayer>& context,
								 int& inner_exit_depth,
								 int& inner_exit_node_id,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
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
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (context.back().states_initialized[this->input_indexes[i_index]]) {
				double val = context.back().state_vals->at(this->input_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].forward(val);
				}
				starting_state_vals[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]) = val;

				starting_states_initialized[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = true;
				starting_is_learn_existing[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] =
					context.back().is_learn_existing[this->input_indexes[i_index]];
			}
		}
	}

	for (int i_index = 0; i_index < (int)this->pre_obs_state_indexes.size(); i_index++) {
		history->pre_obs_snapshot.push_back(context.back().state_vals
			->at(this->pre_obs_state_indexes[i_index]));
	}

	for (int n_index = 0; n_index < (int)this->pre_state_networks.size(); n_index++) {
		int state_index = this->pre_state_network_indexes[n_index];
		if (starting_states_initialized[0][state_index]) {
			if (starting_is_learn_existing[0][state_index]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->pre_state_networks[n_index]);
				this->pre_state_networks[n_index]->activate(history->pre_obs_snapshot,
															starting_state_vals[0]->at(state_index),
															network_history);
				history->pre_state_network_indexes.push_back(state_index);
				history->pre_state_network_histories.push_back(network_history);
			} else {
				this->pre_state_networks[n_index]->activate(history->pre_obs_snapshot,
															starting_state_vals[0]->at(state_index));
			}
		}
	}

	context.back().node_id = this->id;

	context.push_back(ForwardContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];
	context.back().states_initialized = starting_states_initialized[0];
	starting_states_initialized.erase(starting_states_initialized.begin());
	context.back().is_learn_existing = starting_is_learn_existing[0];
	starting_is_learn_existing.erase(starting_is_learn_existing.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<vector<double>*> inner_state_vals(starting_state_vals.size()-1);
	for (int l_index = 0; l_index < (int)starting_state_vals.size()-1; l_index++) {
		inner_state_vals[l_index] = starting_state_vals[1+l_index];
	}

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	inner_scope->activate(starting_node_ids,
						  inner_state_vals,
						  starting_states_initialized,
						  starting_is_learn_existing,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	context.pop_back();

	context.back().node_id = -1;

	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (context.back().states_initialized[this->input_indexes[i_index]]) {
				double val = starting_state_vals[this->input_target_layers[i_index]]->at(this->input_target_indexes[i_index]);
				if (this->input_has_transform[i_index]) {
					val = this->input_transformations[i_index].backward(val);
				}
				context.back().state_vals->at(this->input_indexes[i_index]) = val;
			}
		}
	}

	for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
		history->post_obs_snapshot.push_back(starting_state_vals[0][
			inner_scope->initialized_locally_indexes[i_index]]);
	}

	for (int n_index = 0; n_index < (int)this->post_state_networks.size(); n_index++) {
		int state_index = this->post_state_network_indexes[n_index];
		if (context.back().states_initialized[state_index]) {
			if (context.back().is_learn_existing[state_index]) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->post_state_networks[n_index]);
				this->post_state_networks[n_index]->activate(history->post_obs_snapshot,
															 context.back().state_vals->at(state_index),
															 network_history);
				history->state_network_indexes.push_back(state_index);
				history->state_network_histories.push_back(network_history);
			} else {
				this->post_state_networks[n_index]->activate(history->post_obs_snapshot,
															 context.back().state_vals->at(state_index));
			}
		}
	}

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}

void ScopeNode::backprop(vector<BackwardContextLayer>& context,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	if (run_helper.explore_phase == RUN_PHASE_UPDATE_NONE) {
		double inner_scale_factor_error = 0.0;

		vector<int> empty_starting_node_ids_copy;
		vector<vector<double>*> empty_inner_state_errors_copy;
		inner_scope->backprop(empty_starting_node_ids_copy,
							  empty_inner_state_errors_copy,
							  context,
							  inner_scale_factor_error,
							  run_helper,
							  history->inner_scope_history);

		this->scope_scale_mod->backprop(inner_scale_factor_error, 0.0002);

		scale_factor_error += this->scope_scale_mod->weight*inner_scale_factor_error;
	} else {

	}
}


