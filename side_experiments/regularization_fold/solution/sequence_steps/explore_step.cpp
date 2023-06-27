#include "sequence.h"

using namespace std;

void Sequence::explore_activate(vector<double>& flat_vals,
								vector<ForwardContextLayer>& context,
								vector<Sequence*>& sequences,
								vector<vector<double>>& sequence_ending_input_vals_snapshots,
								vector<vector<ClassDefinition*>>& sequence_input_classes_snapshot,
								RunHelper& run_helper) {
	vector<double> input_vals(this->input_init_types.size(), 0.0);
	vector<ClassDefinition*> input_classes(this->input_init_types.size(), NULL);

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_NONE) {
			// do nothing
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			input_vals[i_index] = ontext[context.size()-1 - this->input_init_local_scope_depths[i_index]]
				.state_vals[this->input_init_local_input_indexes[i_index]];
			input_classes[i_index] = context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
				.state_classes[this->input_init_local_input_indexes[i_index]];
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
			input_vals[i_index] = sequence_ending_input_vals_snapshots[
				this->input_init_previous_step_index[i_index]][this->input_init_previous_input_index[i_index]];
			input_classes[i_index] = sequence_input_classes_snapshots[
				this->input_init_previous_step_index[i_index]][this->input_init_previous_input_index[i_index]];
		} else {
			map<ClassDefinition*, double>::iterator it = run_helper.last_seen_vals.find(this->input_init_last_seen_types[i_index]);
			if (it != run_helper.last_seen_vals.end()) {
				input_vals[i_index] = it->second;
			}
		}
	}

	vector<vector<double>> starting_state_vals(this->starting_node_ids.size()+1);
	vector<vector<ClassDefinition*>> starting_state_classes(this->starting_node_ids.size()+1);
	starting_state_vals[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	starting_state_classes[0] = vector<ClassDefinition*>(this->scopes[0]->num_states, NULL);
	if (this->starting_node_ids.size() > 0) {
		int curr_scope_id = ((ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]])->inner_scope_id;
		Scope* curr_scope = solution->scopes[curr_scope_id];
		for (int l_index = 0; l_index < this->starting_node_ids.size(); l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

			starting_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
			starting_state_classes[1+l_index] = vector<ClassDefinition*>(next_scope->num_states, NULL);

			curr_scope = next_scope;
		}
	}
	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		double val = input_vals[i_index];
		if (this->input_transformations[i_index] != NULL) {
			val = this->input_transformations[i_index]->forward(val);
		}
		starting_state_vals[this->input_init_layer[i_index]][this->input_init_target_index[i_index]] = val;
		
		if (this->input_inner_classes[i_index] == NULL) {
			starting_state_classes[this->input_init_layer[i_index]][this->input_init_target_index[i_index]] = input_classes[i_index];
		} else {
			starting_state_classes[this->input_init_layer[i_index]][this->input_init_target_index[i_index]] = this->input_inner_classes[i_index];
		}
	}

	if (this->starting_node_ids.size() > 0) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		vector<*vector<double>> inner_starting_state_vals(starting_state_vals.size());
		for (int s_index = 0; s_index < (int)starting_state_vals.size(); s_index++) {
			inner_starting_state_vals = &starting_state_vals[s_index];
		}

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		vector<ForwardContextLayer> context;
		int exit_depth;
		int exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		scope_node->halfway_activate(starting_node_ids_copy,
									 inner_starting_state_vals,
									 starting_state_classes,
									 flat_vals,
									 context,
									 exit_depth,
									 exit_node_id,
									 run_helper,
									 node_history);
		delete node_history;
	}

	vector<vector<double>> state_vals(this->scopes.size());
	vector<vector<ClassDefinition*>> state_classes(this->scopes.size());
	state_vals[0] = starting_state_vals[0];
	state_classes[0] = starting_state_classes[0];

	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 0) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];

				run_helper.scale_factor *= scope_node->scope_scale_mod->weight;

				state_vals[l_index+1] = vector<double>(scopes[l_index+1]->num_states, 0.0);
				state_classes[l_index+1] = vector<ClassDefinition*>(scopes[l_index+1]->num_states, NULL);
				for (int i_index = 0; i_index < (int)scope_node->inner_input_indexes.size(); i_index++) {
					double val = state_vals[l_index][scope_node->inner_input_indexes[i_index]];
					if (scope_node->inner_input_transformations[i_index] != NULL) {
						val = scope_node->inner_input_transformations[i_index]->forward(val);
					}
					state_vals[l_index+1][scope_node->inner_input_target_indexes[i_index]] = val;

					if (scope_node->inner_input_classes[i_index] == NULL) {
						state_classes[l_index+1] = state_classes[l_index][scope_node->inner_input_indexes[i_index]];
					} else {
						state_classes[l_index+1] = scope_node->inner_input_classes[i_index];
					}
				}

				// no need to update last_seen_vals
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					action_node->activate(flat_vals,
										  state_vals[l_index],
										  state_classes[l_index],
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
										 state_classes[l_index],
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
				if (state_classes[l_index+1][s_index] != NULL) {
					run_helper.last_seen_vals[state_classes[l_index+1][s_index]] = state_vals[l_index+1][s_index];
				}
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
	sequence_ending_input_vals_snapshots[this->step_index] = input_vals;
	sequence_input_classes_snapshots[this->step_index] = input_classes;

	for (int i_index = 0; i_index < (int)this->input_init_types.size(); i_index++) {
		if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_LOCAL) {
			context[context.size()-1 - this->input_init_local_scope_depths[i_index]]
				.state_vals[this->input_init_local_input_indexes[i_index]] = input_vals[i_index];
		} else if (this->input_init_types[i_index] == SEQUENCE_INPUT_INIT_PREVIOUS) {
			Sequence* curr_sequence = sequences[this->input_init_previous_step_index[i_index]];
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
					Sequence* next_sequence = sequences[curr_sequence->input_init_previous_step_index[curr_input_index]];
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
