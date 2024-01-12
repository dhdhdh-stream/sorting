#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void clean_state(PotentialScopeNode* potential_scope_node) {
	Scope* scope = potential_scope_node->scope;

	int new_num_local_states = 0;
	vector<int> new_original_local_state_ids;

	vector<int> input_mappings(scope->num_local_states);
	for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
		if (scope->used_states[s_index]) {
			input_mappings[s_index] = new_num_local_states;
			new_num_local_states++;
			new_original_local_state_ids.push_back(scope->original_local_state_ids[s_index]);
		} else {
			input_mappings[s_index] = -1;
		}
	}

	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)it->second;

			vector<bool> new_state_is_local;
			vector<int> new_state_indexes;
			vector<int> new_state_obs_indexes;
			vector<State*> new_state_defs;
			vector<int> new_state_network_indexes;

			vector<int> obs_index_mapping(action_node->state_is_local.size(), -1);
			for (int n_index = 0; n_index < (int)action_node->state_is_local.size(); n_index++) {
				int new_state_index = input_mappings[action_node->state_indexes[n_index]];

				if (new_state_index != -1) {
					if (action_node->state_obs_indexes[n_index] == -1) {
						obs_index_mapping[n_index] = (int)new_state_is_local.size();

						new_state_is_local.push_back(true);
						new_state_is_local.push_back(new_state_index);
						new_state_obs_indexes.push_back(-1);
						new_state_defs.push_back(action_node->state_defs[n_index]);
						new_state_network_indexes.push_back(action_node->state_network_indexes[n_index]);
					} else {
						if (obs_index_mapping[action_node->state_obs_indexes[n_index]] != -1) {
							obs_index_mapping[n_index] = (int)new_state_is_local.size();

							new_state_is_local.push_back(true);
							new_state_is_local.push_back(new_state_index);
							new_state_obs_indexes.push_back(obs_index_mapping[action_node->state_obs_indexes[n_index]]);
							new_state_defs.push_back(action_node->state_defs[n_index]);
							new_state_network_indexes.push_back(action_node->state_network_indexes[n_index]);
						}
					}
				}
			}

			action_node->state_is_local = new_state_is_local;
			action_node->state_indexes = new_state_indexes;
			action_node->state_obs_indexes = new_state_obs_indexes;
			action_node->state_defs = new_state_defs;
			action_node->state_network_indexes = new_state_network_indexes;

			action_node->is_potential = false;
		} else if (it->second->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)it->second;

			vector<int> new_input_types;
			vector<bool> new_input_inner_is_local;
			vector<int> new_input_inner_indexes;
			vector<bool> new_input_outer_is_local;
			vector<int> new_input_outer_indexes;
			vector<double> new_input_init_vals;

			vector<bool> new_output_inner_is_local;
			vector<int> new_output_inner_indexes;
			vector<bool> new_output_outer_is_local;
			vector<int> new_output_outer_indexes;

			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
					int new_state_index = input_mappings[scope_node->input_outer_indexes[i_index]];

					if (new_state_index != -1) {
						new_input_types.push_back(INPUT_TYPE_STATE);
						new_input_inner_is_local.push_back(scope_node->input_inner_is_local[i_index]);
						new_input_inner_indexes.push_back(scope_node->input_inner_indexes[i_index]);
						new_input_outer_is_local.push_back(true);
						new_input_outer_indexes.push_back(new_state_index);
						new_input_init_vals.push_back(0.0);
					}
				} else {
					new_input_types.push_back(INPUT_TYPE_CONSTANT);
					new_input_inner_is_local.push_back(scope_node->input_inner_is_local[i_index]);
					new_input_inner_indexes.push_back(scope_node->input_inner_indexes[i_index]);
					new_input_outer_is_local.push_back(false);
					new_input_outer_indexes.push_back(-1);
					new_input_init_vals.push_back(scope_node->input_init_vals[i_index]);
				}
			}

			for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
				int new_state_index = input_mappings[scope_node->output_outer_indexes[o_index]];

				if (new_state_index != -1) {
					new_output_inner_is_local.push_back(scope_node->output_inner_is_local[o_index]);
					new_output_inner_indexes.push_back(scope_node->output_inner_indexes[o_index]);
					new_output_outer_is_local.push_back(true);
					new_output_outer_indexes.push_back(new_state_index);
				}
			}

			scope_node->input_types = new_input_types;
			scope_node->input_inner_is_local = new_input_inner_is_local;
			scope_node->input_inner_indexes = new_input_inner_indexes;
			scope_node->input_outer_is_local = new_input_outer_is_local;
			scope_node->input_outer_indexes = new_input_outer_indexes;
			scope_node->input_init_vals = new_input_init_vals;

			scope_node->output_inner_is_local = new_output_inner_is_local;
			scope_node->output_inner_indexes = new_output_inner_indexes;
			scope_node->output_outer_is_local = new_output_outer_is_local;
			scope_node->output_outer_indexes = new_output_outer_indexes;

			scope_node->is_potential = false;
		} else {
			BranchNode* branch_node = (BranchNode*)it->second;

			vector<bool> new_decision_state_is_local;
			vector<int> new_decision_state_indexes;
			vector<double> new_decision_original_weights;
			vector<double> new_decision_branch_weights;

			for (int s_index = 0; s_index < (int)branch_node->decision_state_is_local.size(); s_index++) {
				int new_state_index = branch_node->decision_state_indexes[s_index];

				// new_state_index != -1
				new_decision_state_is_local.push_back(true);
				new_decision_state_indexes.push_back(new_state_index);
				new_decision_original_weights.push_back(branch_node->decision_original_weights[s_index]);
				new_decision_branch_weights.push_back(branch_node->decision_branch_weights[s_index]);
			}

			branch_node->decision_state_is_local = new_decision_state_is_local;
			branch_node->decision_state_indexes = new_decision_state_indexes;
			branch_node->decision_original_weights = new_decision_original_weights;
			branch_node->decision_branch_weights = new_decision_branch_weights;
		}
	}

	{
		vector<int> new_input_types;
		vector<int> new_input_inner_indexes;
		vector<int> new_input_scope_depths;
		vector<bool> new_input_outer_is_local;
		vector<int> new_input_outer_indexes;
		vector<double> new_input_init_vals;

		vector<int> new_output_inner_indexes;
		vector<int> new_output_scope_depths;
		vector<bool> new_output_outer_is_local;
		vector<int> new_output_outer_indexes;

		for (int i_index = 0; i_index < (int)potential_scope_node->input_types.size(); i_index++) {
			int new_state_index = input_mappings[potential_scope_node->input_inner_indexes[i_index]];

			if (new_state_index != -1) {
				new_input_types.push_back(potential_scope_node->input_types[i_index]);
				new_input_inner_indexes.push_back(new_state_index);
				new_input_scope_depths.push_back(potential_scope_node->input_scope_depths[i_index]);
				new_input_outer_is_local.push_back(potential_scope_node->input_outer_is_local[i_index]);
				new_input_outer_indexes.push_back(potential_scope_node->input_outer_indexes[i_index]);
				new_input_init_vals.push_back(potential_scope_node->input_init_vals[i_index]);
			}
		}

		/**
		 * - TODO: potentially track output impact separately to remove separately
		 */
		for (int o_index = 0; o_index < (int)potential_scope_node->output_inner_indexes.size(); o_index++) {
			int new_state_index = input_mappings[potential_scope_node->output_inner_indexes[o_index]];

			if (new_state_index != -1) {
				new_output_inner_indexes.push_back(new_state_index);
				new_output_scope_depths.push_back(potential_scope_node->output_scope_depths[o_index]);
				new_output_outer_is_local.push_back(potential_scope_node->output_outer_is_local[o_index]);
				new_output_outer_indexes.push_back(potential_scope_node->output_outer_indexes[o_index]);
			}
		}

		potential_scope_node->input_types = new_input_types;
		potential_scope_node->input_inner_indexes = new_input_inner_indexes;
		potential_scope_node->input_scope_depths = new_input_scope_depths;
		potential_scope_node->input_outer_is_local = new_input_outer_is_local;
		potential_scope_node->input_outer_indexes = new_input_outer_indexes;
		potential_scope_node->input_init_vals = new_input_init_vals;

		potential_scope_node->output_inner_indexes = new_output_inner_indexes;
		potential_scope_node->output_scope_depths = new_output_scope_depths;
		potential_scope_node->output_outer_is_local = new_output_outer_is_local;
		potential_scope_node->output_outer_indexes = new_output_outer_indexes;

		potential_scope_node->is_cleaned = true;
	}

	scope->used_states.clear();
}
