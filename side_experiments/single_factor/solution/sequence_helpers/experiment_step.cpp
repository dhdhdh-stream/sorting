#include "sequence.h"

using namespace std;

void Sequence::activate(vector<double>& flat_vals,
						vector<ContextLayer>& context,
						SequenceHistory* history) {
	run_helper.curr_depth++;

	vector<double> input_vals(this->input_types.size(), 0.0);
	vector<double> input_weights(this->input_types.size(), 1.0);
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			input_vals[i_index] = context[context.size()-1 - this->input_local_scope_depths[i_index]]
				.state_vals->at(this->input_local_input_indexes[i_index]);
			input_weights[i_index] = context[context.size()-1 - this->input_local_scope_depths[i_index]]
				.state_weights[this->input_local_input_indexes[i_index]];
		}
	}

	vector<vector<double>> inner_state_vals(this->starting_node_ids.size());
	vector<vector<double>> inner_state_weights(this->starting_node_ids.size());

	inner_state_vals[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	inner_state_weights[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	for (int i_index = 0; i_index < (int)this->scopes[0]->initialized_locally_indexes.size(); i_index++) {
		inner_state_weights[0][this->scopes[0]->initialized_locally_indexes[i_index]] = 1.0;
	}

	Scope* curr_scope = this->scopes[0];
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

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = input_vals[i_index];
		inner_state_weights[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = input_weights[i_index];
	}

	vector<ContextLayer> temp_context;
	temp_context.push_back(ContextLayer());

	/**
	 * - set to unique value to enable obs to be specific to sequence
	 */
	temp_context.back().scope_id = -1 - this->step_index;
	temp_context.back().node_id = -1;

	temp_context.back().state_vals = &(inner_state_vals[0]);
	temp_context.back().state_weights = inner_state_weights[0];
	inner_state_weights.erase(inner_state_weights.begin());

	history->node_histories = vector<vector<AbstractNodeHistory*>>(this->scopes.size());

	if (this->starting_node_ids.size() > 1) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		starting_node_ids_copy.erase(starting_node_ids_copy.begin());

		vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
		}

		// before being passed in, starting_node_ids_copy.size() == inner_state_vals_copy.size()

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids_copy,
									 inner_state_vals_copy,
									 inner_state_weights,
									 flat_vals,
									 temp_context,
									 inner_exit_depth,
									 inner_exit_node_id,
									 run_helper,
									 node_history);
	}

	vector<int> next_starting_node_ids;
	vector<vector<double>*> next_starting_state_vals;
	vector<vector<double>> next_starting_state_weights;
	vector<EndingScopeNodeActivateHelper> ending_scope_node_helpers;
	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 1) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
				ending_scope_node_helpers.push_back(EndingScopeNodeHelper(scope_node));
				ending_scope_node_helpers.back().forward(next_starting_node_ids,
														 next_starting_state_vals,
														 next_starting_state_weights,
														 temp_context,
														 run_helper);
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
					action_node->activate(flat_vals,
										  temp_context,
										  run_helper,
										  history->node_histories[l_index]);
				} else if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

					// unused
					int inner_exit_depth;
					int inner_exit_node_id;

					if (next_starting_node_ids.size() > 0) {
						// first node in layer
						scope_node->halfway_activate(next_starting_node_ids,
													 next_starting_state_vals,
													 next_starting_state_weights,
													 flat_vals,
													 temp_context,
													 inner_exit_depth,
													 inner_exit_node_id,
													 run_helper,
													 history->node_histories[l_index]);

						next_starting_node_ids.clear();
						next_starting_state_vals.clear();
						next_starting_state_weights.clear();
					} else {
						scope_node->activate(flat_vals,
											 temp_context,
											 inner_exit_depth,
											 inner_exit_node_id,
											 run_helper,
											 history->node_histories[l_index]);
					}
				} else if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_EXIT) {
					ExitNode* exit_node = (ExitNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
					exit_node->activate(temp_context);
				}
			}
		}
	}

	for (int e_index = (int)ending_scope_node_helpers.size()-1; e_index >= 0; e_index--) {
		ending_scope_node_helpers[e_index].backward(e_index,
													temp_context,
													run_helper,
													history);
	}
	history->initialized_locally_val_snapshots.insert(
		history->initialized_locally_val_snapshots.begin(), vector<double>());
	history->initialized_locally_weight_snapshots.insert(
		history->initialized_locally_weight_snapshots.begin(), vector<double>());
	for (int i_index = 0; i_index < (int)this->scopes[0]->initialized_locally_indexes.size(); i_index++) {
		double state_val = temp_context.back().state_vals->at(inner_scope->initialized_locally_indexes[i_index]);
		double state_weight = temp_context.back().state_weights[inner_scope->initialized_locally_indexes[i_index]];

		history->initialized_locally_val_snapshots[0].push_back(state_val);
		history->initialized_locally_weight_snapshots[0].push_back(state_weight);

		/**
		 * - simply apply weight_mods here instead of applying retroactively
		 */
		run_helper.predicted_score += this->weight_mods[0][i_index]
			*state_weight*state_val*this->scopes[0]->ending_score_scales[i_index]->weight;
	}

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == SEQUENCE_INPUT_TYPE_LOCAL) {
			input_vals[i_index] = inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
			context[context.size()-1 - this->input_local_scope_depths[i_index]]
				.state_vals->at(this->input_local_input_indexes[i_index]) = input_vals[i_index];
		}
	}

	run_helper.curr_depth--;
}

void Sequence::backprop(double& scale_factor_error,
						RunHelper& run_helper,
						SequenceHistory* history) {

}

// TODO: calculate scale_factor off of running_average only
// - scales can be adjusted by weight_mods
// - or just use the sum of predicted score changes
// - or simply do a full backprop on sequences (and ignore pre and post)
