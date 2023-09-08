#include "sequence.h"

using namespace std;

void Sequence::explore_activate(vector<double>& flat_vals,
								vector<ContextLayer>& context,
								SequenceHistory* history) {
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

	temp_context.back().scope_id = -1;
	temp_context.back().node_id = -1;

	temp_context.back().state_vals = &(inner_state_vals[0]);
	temp_context.back().state_weights = inner_state_weights[0];
	inner_state_weights.erase(inner_state_weights.begin());

	history->node_histories.push_back(vector<AbstractNodeHistory*>());

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

	/**
	 * TODO:
	 * - don't have branch weight
	 *   - when exploring, just let it keep going, and cut it off
	 *     - so sequences are open ended-ish
	 */
	while (true) {
		if () {
			break;
		}

		if () {

		}
	}
}
