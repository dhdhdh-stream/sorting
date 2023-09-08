#include "ending_scope_node_helper.h"

using namespace std;

EndingScopeNodeHelper::EndingScopeNodeHelper(ScopeNode* scope_node) {
	this->scope_node = scope_node;
}

void EndingScopeNodeHelper::forward(vector<int>& next_starting_node_ids,
									vector<vector<double>*>& next_starting_state_vals,
									vector<vector<double>>& next_starting_state_weights,
									vector<ContextLayer>& context,
									RunHelper& run_helper) {
	run_helper.scale_factor *= this->scope_node->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->scope_node->inner_scope_id];

	if (next_starting_node_ids.size() == 0) {
		this->is_halfway = false;

		this->inner_state_vals = vector<vector<double>>(this->scope_node->starting_node_ids.size());
		next_starting_state_weights = vector<vector<double>>(this->scope_node->starting_node_ids.size());

		this->inner_state_vals[0] = vector<double>(inner_scope->num_states, 0.0);
		next_starting_state_weights[0] = vector<double>(inner_scope->num_states, 0.0);
		for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
			next_starting_state_weights[0][inner_scope->initialized_locally_indexes[i_index]] = 1.0;
		}

		Scope* curr_scope = inner_scope;
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size()-1; l_index++) {
			ScopeNode* l_scope_node = (ScopeNode*)curr_scope->nodes[this->scope_node->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[l_scope_node->inner_scope_id];

			this->inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
			next_starting_state_weights[1+l_index] = vector<double>(next_scope->num_states, 0.0);
			for (int i_index = 0; i_index < (int)next_scope->initialized_locally_indexes.size(); i_index++) {
				next_starting_state_weights[1+l_index][next_scope->initialized_locally_indexes[i_index]] = 1.0;
			}

			curr_scope = next_scope;
		}

		vector<double> pre_obs_snapshots;
		for (int s_index = 0; s_index < (int)this->scope_node->input_target_layers.size(); s_index++) {
			if (this->scope_node->input_target_layers[s_index] != -1) {
				if (context.back().states_weights[s_index] != 0.0) {
					this->inner_state_vals[this->scope_node->input_target_layers[s_index]][this->scope_node->input_target_indexes[s_index]]
						= context.back().state_vals->at(s_index);
					next_starting_state_weights[this->scope_node->input_target_layers[i_index]][this->scope_node->input_target_indexes[s_index]]
						= context.back().state_weights[s_index];
				}
			} else {
				pre_obs_snapshots.push_back(context.back().state_vals->at(s_index));
			}
		}

		for (int n_index = 0; n_index < (int)this->scope_node->pre_state_networks.size(); n_index++) {
			if (inner_state_weights[0][this->scope_node->pre_state_network_indexes[n_index]] != 0.0) {
				this->scope_node_pre_state_networks[n_index]->activate(
					pre_obs_snapshot[this->scope_node->pre_state_network_indexes[n_index]],
					inner_state_vals[0]->at(this->scope_node->pre_state_network_target_indexes[n_index]));
			}
		}

		// no need to update context.back().node_id

		context.push_back(ContextLayer());

		context.back().scope_id = -1;
		context.back().node_id = -1;

		context.back().state_vals = &(this->inner_state_vals[0]);
		context.back().state_weights = next_starting_state_weights[0];
		next_starting_state_weights.erase(next_starting_state_weights.begin());

		next_starting_node_ids = this->scope_node->starting_node_ids;
		next_starting_node_ids.erase(next_starting_node_ids.begin());

		next_starting_state_vals = vector<vector<double>*>(this->scope_node->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size()-1; l_index++) {
			next_starting_state_vals[l_index] = &(this->inner_state_vals[1+l_index]);
		}
	} else {
		this->is_halfway = true;

		this->furthest_matching_layer = 0;
		for (int l_index = 0; l_index < (int)this->scope_node->starting_node_ids.size(); l_index++) {
			if (l_index >= (int)next_starting_node_ids.size()
					|| next_starting_node_ids[l_index] != this->scope_node->starting_node_ids[l_index]) {
				break;
			} else {
				this->furthest_matching_layer++;
			}
		}
		vector<double> pre_obs_snapshots;
		for (int s_index = 0; s_index < (int)this->scope_node->input_target_layers.size(); s_index++) {
			if (this->scope_node->input_target_layers[s_index] <= this->furthest_matching_layer) {
				if (context.back().state_weights[s_index] != 0.0) {
					next_starting_state_vals[this->scope_node->input_target_layers[s_index]]->at(this->scope_node->input_target_indexes[s_index])
						= context.back().state_vals->at(s_index);
					next_starting_state_weights[this->scope_node->input_target_layers[s_index]][this->scope_node->input_target_indexes[s_index]]
						= context.back().state_weights[s_index];
				}
			} else {
				pre_obs_snapshots.push_back(context.back().state_vals->at(s_index));
			}
		}

		for (int n_index = 0; n_index < (int)this->scope_node->pre_state_networks.size(); n_index++) {
			if (inner_state_weights[0][this->scope_node->pre_state_network_indexes[n_index]] != 0.0) {
				this->scope_node_pre_state_networks[n_index]->activate(
					pre_obs_snapshot[this->scope_node->pre_state_network_indexes[n_index]],
					inner_state_vals[0]->at(this->scope_node->pre_state_network_target_indexes[n_index]));
			}
		}

		// no need to update context.back().node_id

		context.push_back(ContextLayer());

		context.back().scope_id = -1;
		context.back().node_id = -1;

		context.back().state_vals = next_starting_state_vals[0];
		context.back().state_weights = next_starting_state_weights[0];
		next_starting_state_weights.erase(next_starting_state_weights.begin());

		// no need to set context.back().scope_history

		next_starting_node_ids.erase(next_starting_node_ids.begin());

		this->starting_state_vals_save = next_starting_state_vals;
		next_starting_state_vals.erase(next_starting_state_vals.begin());
	}

	run_helper.curr_depth++;
}

void EndingScopeNodeHelper::backward(int e_index,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 SequenceHistory* history) {
	history->initialized_locally_val_snapshots.insert(
		history->initialized_locally_val_snapshots.begin(), vector<double>());
	history->initialized_locally_weight_snapshots.insert(
		history->initialized_locally_weight_snapshots.begin(), vector<double>());
	for (int i_index = 0; i_index < (int)inner_scope->initialized_locally_indexes.size(); i_index++) {
		double state_val = context.back().state_vals->at(inner_scope->initialized_locally_indexes[i_index]);
		double state_weight = context.back().state_weights[inner_scope->initialized_locally_indexes[i_index]];

		history->initialized_locally_val_snapshots[0].push_back(state_val);
		history->initialized_locally_weight_snapshots[0].push_back(state_weight);

		/**
		 * - simply apply weight_mods here instead of applying retroactively
		 */
		run_helper.predicted_score += this->weight_mods[e_index+1][i_index]
			*state_weight*state_val*inner_scope->ending_score_scales[i_index]->weight;
	}

	run_helper.curr_depth--;

	context.pop_back();

	// no need to update context.back().node_id

	if (this->is_halfway == false) {
		for (int s_index = 0; s_index < (int)this->scope_node->input_target_layers.size(); s_index++) {
			if (context.back().state_weights[s_index] != 0.0) {
				context.back().state_vals->at(s_index) = this->inner_state_vals[this->scope_node->input_target_layers[s_index]][this->scope_node->input_target_indexes[s_index]];
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->scope_node->input_target_layers.size(); s_index++) {
			if (this->scope_node->input_target_layers[s_index] <= this->furthest_matching_layer) {
				if (context.back().state_weights[s_index] != 0.0) {
					context.back().state_vals->at(s_index) = this->starting_state_vals_save[this->scope_node->input_target_layers[s_index]]->at(this->scope_node->input_target_indexes[s_index]);
				}
			}
		}
	}

	for (int n_index = 0; n_index < (int)this->scope_node->post_state_networks.size(); n_index++) {
		if (context.back().state_weights[this->scope_node->post_state_network_target_indexes[n_index]] != 0.0) {
			this->scope_node->post_state_networks[n_index]->activate(
				history->initialized_locally_val_snapshots[0][this->scope_node->post_state_network_indexes[n_index]],
				context.back().state_vals->at(this->scope_node->post_state_network_target_indexes[n_index]));
		}
	}

	run_helper.scale_factor /= this->scope_node->scope_scale_mod->weight;
}
