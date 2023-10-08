#include "helpers.h"

#include <iostream>
#include <set>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"

using namespace std;

void finalize_existing_state(Scope* parent_scope,
							 State* score_state,
							 BranchNode* new_branch_node,
							 double new_branch_weight) {
	int new_local_index = parent_scope->num_local_states;
	parent_scope->num_local_states++;

	set<ScopeNode*> local_scope_nodes_to_mod;
	set<Scope*> input_scopes_to_mod;
	set<pair<Scope*, ScopeNode*>> input_scope_nodes_to_mod;

	for (int n_index = 0; n_index < (int)score_state->nodes.size(); n_index++) {
		if (score_state->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)score_state->nodes[n_index];

			int score_state_index;
			for (int s_index = 0; s_index < (int)action_node->score_state_defs.size(); s_index++) {
				if (action_node->score_state_defs[s_index] == score_state) {
					score_state_index = s_index;
					break;
				}
			}

			if (action_node->score_state_scope_contexts[score_state_index].size() == 1) {
				action_node->state_is_local.push_back(true);
				action_node->state_indexes.push_back(new_local_index);
				action_node->state_defs.push_back(score_state);
				action_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* containing_scope = solution->scopes[action_node->score_state_scope_contexts[score_state_index].back()];
				int new_input_id = containing_scope->num_input_states;
				action_node->state_is_local.push_back(false);
				action_node->state_indexes.push_back(new_input_id);
				action_node->state_defs.push_back(score_state);
				action_node->state_network_indexes.push_back(n_index);

				ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[action_node->score_state_node_contexts[score_state_index][0]];
				local_scope_nodes_to_mod.insert(local_scope_node);
				for (int c_index = 1; c_index < (int)action_node->score_state_scope_contexts[score_state_index].size()-1; c_index++) {
					Scope* input_scope = solution->scopes[action_node->score_state_scope_contexts[score_state_index][c_index]];
					ScopeNode* input_scope_node = (ScopeNode*)input_scope->nodes[action_node->score_state_node_contexts[score_state_index][c_index]];
					input_scopes_to_mod.insert(input_scope);
					input_scope_nodes_to_mod.insert({input_scope, input_scope_node});
				}
				input_scopes_to_mod.insert(containing_scope);
			}

			action_node->score_state_scope_contexts.erase(action_node->score_state_scope_contexts.begin() + score_state_index);
			action_node->score_state_node_contexts.erase(action_node->score_state_node_contexts.begin() + score_state_index);
			action_node->score_state_defs.erase(action_node->score_state_defs.begin() + score_state_index);
			action_node->score_state_network_indexes.erase(action_node->score_state_network_indexes.begin() + score_state_index);
		} else if (score_state->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)score_state->nodes[n_index];

			int score_state_index;
			for (int s_index = 0; s_index < (int)scope_node->score_state_defs.size(); s_index++) {
				if (scope_node->score_state_defs[s_index] == score_state) {
					score_state_index = s_index;
					break;
				}
			}

			if (scope_node->score_state_scope_contexts[score_state_index].size() == 1) {
				scope_node->state_is_local.push_back(true);
				scope_node->state_indexes.push_back(new_local_index);
				scope_node->state_obs_indexes.push_back(scope_node->score_state_obs_indexes[score_state_index]);
				scope_node->state_defs.push_back(score_state);
				scope_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* containing_scope = solution->scopes[scope_node->score_state_scope_contexts[score_state_index].back()];
				int new_input_id = containing_scope->num_input_states;
				scope_node->state_is_local.push_back(false);
				scope_node->state_indexes.push_back(new_input_id);
				scope_node->state_obs_indexes.push_back(scope_node->score_state_obs_indexes[score_state_index]);
				scope_node->state_defs.push_back(score_state);
				scope_node->state_network_indexes.push_back(n_index);

				ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[scope_node->score_state_node_contexts[score_state_index][0]];
				local_scope_nodes_to_mod.insert(local_scope_node);
				for (int c_index = 1; c_index < (int)scope_node->score_state_scope_contexts[score_state_index].size()-1; c_index++) {
					Scope* input_scope = solution->scopes[scope_node->score_state_scope_contexts[score_state_index][c_index]];
					ScopeNode* input_scope_node = (ScopeNode*)input_scope->nodes[scope_node->score_state_node_contexts[score_state_index][c_index]];
					input_scopes_to_mod.insert(input_scope);
					input_scope_nodes_to_mod.insert({input_scope, input_scope_node});
				}
				input_scopes_to_mod.insert(containing_scope);
			}

			scope_node->score_state_scope_contexts.erase(scope_node->score_state_scope_contexts.begin() + score_state_index);
			scope_node->score_state_node_contexts.erase(scope_node->score_state_node_contexts.begin() + score_state_index);
			scope_node->score_state_obs_indexes.erase(scope_node->score_state_obs_indexes.begin() + score_state_index);
			scope_node->score_state_defs.erase(scope_node->score_state_defs.begin() + score_state_index);
			scope_node->score_state_network_indexes.erase(scope_node->score_state_network_indexes.begin() + score_state_index);
		} else {
			BranchNode* branch_node = (BranchNode*)score_state->nodes[n_index];

			int score_state_index;
			for (int s_index = 0; s_index < (int)branch_node->score_state_defs.size(); s_index++) {
				if (branch_node->score_state_defs[s_index] == score_state) {
					score_state_index = s_index;
					break;
				}
			}

			if (branch_node->score_state_scope_contexts[score_state_index].size() == 1) {
				branch_node->state_is_local.push_back(true);
				branch_node->state_indexes.push_back(new_local_index);
				branch_node->state_defs.push_back(score_state);
				branch_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* containing_scope = solution->scopes[branch_node->score_state_scope_contexts[score_state_index].back()];
				int new_input_id = containing_scope->num_input_states;
				branch_node->state_is_local.push_back(false);
				branch_node->state_indexes.push_back(new_input_id);
				branch_node->state_defs.push_back(score_state);
				branch_node->state_network_indexes.push_back(n_index);

				ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[branch_node->score_state_node_contexts[score_state_index][0]];
				local_scope_nodes_to_mod.insert(local_scope_node);
				for (int c_index = 1; c_index < (int)branch_node->score_state_scope_contexts[score_state_index].size()-1; c_index++) {
					Scope* input_scope = solution->scopes[branch_node->score_state_scope_contexts[score_state_index][c_index]];
					ScopeNode* input_scope_node = (ScopeNode*)input_scope->nodes[branch_node->score_state_node_contexts[score_state_index][c_index]];
					input_scopes_to_mod.insert(input_scope);
					input_scope_nodes_to_mod.insert({input_scope, input_scope_node});
				}
				input_scopes_to_mod.insert(containing_scope);
			}

			branch_node->score_state_scope_contexts.erase(branch_node->score_state_scope_contexts.begin() + score_state_index);
			branch_node->score_state_node_contexts.erase(branch_node->score_state_node_contexts.begin() + score_state_index);
			branch_node->score_state_defs.erase(branch_node->score_state_defs.begin() + score_state_index);
			branch_node->score_state_network_indexes.erase(branch_node->score_state_network_indexes.begin() + score_state_index);
		}
	}
	score_state->nodes.clear();

	if (new_branch_node->branch_scope_context.size() == 1) {
		new_branch_node->shared_state_is_local.push_back(true);
		new_branch_node->shared_state_indexes.push_back(new_local_index);
		new_branch_node->branch_weights.push_back(new_branch_weight);
		new_branch_node->original_state_defs.push_back(score_state);
	} else {
		Scope* containing_scope = solution->scopes[new_branch_node->branch_scope_context[0]];
		int new_input_id = containing_scope->num_input_states;
		new_branch_node->shared_state_is_local.push_back(false);
		new_branch_node->shared_state_indexes.push_back(new_input_id);
		new_branch_node->branch_weights.push_back(new_branch_weight);
		new_branch_node->original_state_defs.push_back(score_state);

		ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[new_branch_node->branch_node_context[0]];
		local_scope_nodes_to_mod.insert(local_scope_node);
		for (int c_index = 1; c_index < (int)new_branch_node->branch_scope_context.size()-1; c_index++) {
			Scope* input_scope = solution->scopes[new_branch_node->branch_scope_context[c_index]];
			ScopeNode* input_scope_node = (ScopeNode*)input_scope->nodes[new_branch_node->branch_node_context[c_index]];
			input_scopes_to_mod.insert(input_scope);
			input_scope_nodes_to_mod.insert({input_scope, input_scope_node});
		}
		input_scopes_to_mod.insert(containing_scope);
	}

	for (set<ScopeNode*>::iterator it = local_scope_nodes_to_mod.begin();
			it != local_scope_nodes_to_mod.end(); it++) {
		ScopeNode* scope_node = *it;
		Scope* inner_scope = scope_node->inner_scope;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_layers.push_back(0);
		scope_node->input_inner_is_local.push_back(false);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->input_outer_is_local.push_back(true);
		scope_node->input_outer_indexes.push_back(new_local_index);
		scope_node->input_init_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->output_outer_is_local.push_back(true);
		scope_node->output_outer_indexes.push_back(new_local_index);
	}
	for (set<pair<Scope*, ScopeNode*>>::iterator it = input_scope_nodes_to_mod.begin();
			it != input_scope_nodes_to_mod.end(); it++) {
		Scope* outer_scope = (*it).first;
		ScopeNode* scope_node = (*it).second;
		Scope* inner_scope = scope_node->inner_scope;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_layers.push_back(0);
		scope_node->input_inner_is_local.push_back(false);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->input_outer_is_local.push_back(false);
		scope_node->input_outer_indexes.push_back(outer_scope->num_input_states);
		scope_node->input_init_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->output_outer_is_local.push_back(false);
		scope_node->output_outer_indexes.push_back(outer_scope->num_input_states);
	}
	for (set<Scope*>::iterator it = input_scopes_to_mod.begin();
			it != input_scopes_to_mod.end(); it++) {
		Scope* scope = *it;
		scope->num_input_states++;
	}
}

void finalize_new_state(Scope* parent_scope,
						map<int, Sequence*>& sequence_mappings,
						State* score_state,
						vector<AbstractNode*>& nodes,
						vector<vector<int>>& scope_contexts,
						vector<vector<int>>& node_contexts,
						vector<int>& obs_indexes,
						BranchNode* new_branch_node) {
	int new_local_index = parent_scope->num_local_states;
	parent_scope->num_local_states++;

	set<ScopeNode*> local_scope_nodes_to_mod;
	set<Sequence*> local_sequences_to_mod;
	set<Scope*> input_scopes_to_mod;
	set<pair<Scope*, ScopeNode*>> input_scope_nodes_to_mod;
	set<pair<Scope*, Sequence*>> input_sequences_to_mod;

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)nodes[n_index];

			if (scope_contexts[n_index].size() == 1) {
				action_node->state_is_local.push_back(true);
				action_node->state_indexes.push_back(new_local_index);
				action_node->state_defs.push_back(score_state);
				action_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* curr_scope = parent_scope;
				{
					if (node_contexts[n_index][0] == -1) {
						int inner_scope_id = scope_contexts[n_index][1];
						Sequence* local_sequence = sequence_mappings[inner_scope_id];
						local_sequences_to_mod.insert(local_sequence);

						curr_scope = local_sequence->scope;
					} else {
						ScopeNode* local_scope_node = (ScopeNode*)curr_scope->nodes[node_contexts[n_index][0]];
						local_scope_nodes_to_mod.insert(local_scope_node);

						curr_scope = local_scope_node->inner_scope;
					}
				}
				for (int c_index = 1; c_index < (int)scope_contexts[n_index].size()-1; c_index++) {
					input_scopes_to_mod.insert(curr_scope);

					if (node_contexts[n_index][c_index] == -1) {
						int inner_scope_id = scope_contexts[n_index][c_index+1];
						Sequence* input_sequence = sequence_mappings[inner_scope_id];
						input_sequences_to_mod.insert({curr_scope, input_sequence});

						curr_scope = input_sequence->scope;
					} else {
						ScopeNode* input_scope_node = (ScopeNode*)curr_scope->nodes[node_contexts[n_index][c_index]];
						input_scope_nodes_to_mod.insert({curr_scope, input_scope_node});

						curr_scope = input_scope_node->inner_scope;
					}
				}
				input_scopes_to_mod.insert(curr_scope);

				int new_input_id = curr_scope->num_input_states;
				action_node->state_is_local.push_back(false);
				action_node->state_indexes.push_back(new_input_id);
				action_node->state_defs.push_back(score_state);
				action_node->state_network_indexes.push_back(n_index);
			}
		} else if (nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)nodes[n_index];

			if (scope_contexts[n_index].size() == 1) {
				scope_node->state_is_local.push_back(true);
				scope_node->state_indexes.push_back(new_local_index);
				scope_node->state_obs_indexes.push_back(obs_indexes[n_index]);
				scope_node->state_defs.push_back(score_state);
				scope_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* curr_scope = parent_scope;
				{
					if (node_contexts[n_index][0] == -1) {
						int inner_scope_id = scope_contexts[n_index][1];
						Sequence* local_sequence = sequence_mappings[inner_scope_id];
						local_sequences_to_mod.insert(local_sequence);

						curr_scope = local_sequence->scope;
					} else {
						ScopeNode* local_scope_node = (ScopeNode*)curr_scope->nodes[node_contexts[n_index][0]];
						local_scope_nodes_to_mod.insert(local_scope_node);

						curr_scope = local_scope_node->inner_scope;
					}
				}
				for (int c_index = 1; c_index < (int)scope_contexts[n_index].size()-1; c_index++) {
					input_scopes_to_mod.insert(curr_scope);

					if (node_contexts[n_index][c_index] == -1) {
						int inner_scope_id = scope_contexts[n_index][c_index+1];
						Sequence* input_sequence = sequence_mappings[inner_scope_id];
						input_sequences_to_mod.insert({curr_scope, input_sequence});

						curr_scope = input_sequence->scope;
					} else {
						ScopeNode* input_scope_node = (ScopeNode*)curr_scope->nodes[node_contexts[n_index][c_index]];
						input_scope_nodes_to_mod.insert({curr_scope, input_scope_node});

						curr_scope = input_scope_node->inner_scope;
					}
				}
				input_scopes_to_mod.insert(curr_scope);

				int new_input_id = curr_scope->num_input_states;
				scope_node->state_is_local.push_back(false);
				scope_node->state_indexes.push_back(new_input_id);
				scope_node->state_obs_indexes.push_back(obs_indexes[n_index]);
				scope_node->state_defs.push_back(score_state);
				scope_node->state_network_indexes.push_back(n_index);
			}
		} else {
			BranchNode* branch_node = (BranchNode*)nodes[n_index];

			if (scope_contexts[n_index].size() == 1) {
				branch_node->state_is_local.push_back(true);
				branch_node->state_indexes.push_back(new_local_index);
				branch_node->state_defs.push_back(score_state);
				branch_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* curr_scope = parent_scope;
				{
					if (node_contexts[n_index][0] == -1) {
						int inner_scope_id = scope_contexts[n_index][1];
						Sequence* local_sequence = sequence_mappings[inner_scope_id];
						local_sequences_to_mod.insert(local_sequence);

						curr_scope = local_sequence->scope;
					} else {
						ScopeNode* local_scope_node = (ScopeNode*)curr_scope->nodes[node_contexts[n_index][0]];
						local_scope_nodes_to_mod.insert(local_scope_node);

						curr_scope = local_scope_node->inner_scope;
					}
				}
				for (int c_index = 1; c_index < (int)scope_contexts[n_index].size()-1; c_index++) {
					input_scopes_to_mod.insert(curr_scope);

					if (node_contexts[n_index][c_index] == -1) {
						int inner_scope_id = scope_contexts[n_index][c_index+1];
						Sequence* input_sequence = sequence_mappings[inner_scope_id];
						input_sequences_to_mod.insert({curr_scope, input_sequence});

						curr_scope = input_sequence->scope;
					} else {
						ScopeNode* input_scope_node = (ScopeNode*)curr_scope->nodes[node_contexts[n_index][c_index]];
						input_scope_nodes_to_mod.insert({curr_scope, input_scope_node});

						curr_scope = input_scope_node->inner_scope;
					}
				}
				input_scopes_to_mod.insert(curr_scope);

				int new_input_id = curr_scope->num_input_states;
				branch_node->state_is_local.push_back(false);
				branch_node->state_indexes.push_back(new_input_id);
				branch_node->state_defs.push_back(score_state);
				branch_node->state_network_indexes.push_back(n_index);
			}
		}
	}
	score_state->nodes.clear();

	if (new_branch_node->branch_scope_context.size() == 1) {
		new_branch_node->branch_state_is_local.push_back(true);
		new_branch_node->branch_state_indexes.push_back(new_local_index);
		new_branch_node->branch_state_defs.push_back(score_state);
	} else {
		Scope* containing_scope = solution->scopes[new_branch_node->branch_scope_context[0]];
		int new_input_id = containing_scope->num_input_states;
		new_branch_node->branch_state_is_local.push_back(false);
		new_branch_node->branch_state_indexes.push_back(new_input_id);
		new_branch_node->branch_state_defs.push_back(score_state);

		ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[new_branch_node->branch_node_context[0]];
		local_scope_nodes_to_mod.insert(local_scope_node);
		for (int c_index = 1; c_index < (int)new_branch_node->branch_scope_context.size()-1; c_index++) {
			Scope* input_scope = solution->scopes[new_branch_node->branch_scope_context[c_index]];
			ScopeNode* input_scope_node = (ScopeNode*)input_scope->nodes[new_branch_node->branch_node_context[c_index]];
			input_scopes_to_mod.insert(input_scope);
			input_scope_nodes_to_mod.insert({input_scope, input_scope_node});
		}
		input_scopes_to_mod.insert(containing_scope);
	}

	for (set<ScopeNode*>::iterator it = local_scope_nodes_to_mod.begin();
			it != local_scope_nodes_to_mod.end(); it++) {
		ScopeNode* scope_node = *it;
		Scope* inner_scope = scope_node->inner_scope;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_layers.push_back(0);
		scope_node->input_inner_is_local.push_back(false);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->input_outer_is_local.push_back(true);
		scope_node->input_outer_indexes.push_back(new_local_index);
		scope_node->input_init_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->output_outer_is_local.push_back(true);
		scope_node->output_outer_indexes.push_back(new_local_index);
	}
	for (set<Sequence*>::iterator it = local_sequences_to_mod.begin();
			it != local_sequences_to_mod.end(); it++) {
		Sequence* sequence = *it;
		Scope* inner_scope = sequence->scope;

		sequence->input_types.push_back(INPUT_TYPE_STATE);
		sequence->input_inner_indexes.push_back(inner_scope->num_input_states);
		sequence->input_scope_depths.push_back(0);
		sequence->input_outer_is_local.push_back(true);
		sequence->input_outer_indexes.push_back(new_local_index);
		sequence->input_init_vals.push_back(0.0);

		sequence->output_inner_indexes.push_back(inner_scope->num_input_states);
		sequence->output_scope_depths.push_back(0);
		sequence->output_outer_is_local.push_back(true);
		sequence->output_outer_indexes.push_back(new_local_index);
	}
	for (set<pair<Scope*, ScopeNode*>>::iterator it = input_scope_nodes_to_mod.begin();
			it != input_scope_nodes_to_mod.end(); it++) {
		Scope* outer_scope = (*it).first;
		ScopeNode* scope_node = (*it).second;
		Scope* inner_scope = scope_node->inner_scope;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_layers.push_back(0);
		scope_node->input_inner_is_local.push_back(false);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->input_outer_is_local.push_back(false);
		scope_node->input_outer_indexes.push_back(outer_scope->num_input_states);
		scope_node->input_init_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->output_outer_is_local.push_back(false);
		scope_node->output_outer_indexes.push_back(outer_scope->num_input_states);
	}
	for (set<pair<Scope*, Sequence*>>::iterator it = input_sequences_to_mod.begin();
			it != input_sequences_to_mod.end(); it++) {
		Scope* outer_scope = (*it).first;
		Sequence* sequence = (*it).second;
		Scope* inner_scope = sequence->scope;

		sequence->input_types.push_back(INPUT_TYPE_STATE);
		sequence->input_inner_indexes.push_back(inner_scope->num_input_states);
		sequence->input_scope_depths.push_back(0);
		sequence->input_outer_is_local.push_back(false);
		sequence->input_outer_indexes.push_back(outer_scope->num_input_states);
		sequence->input_init_vals.push_back(0.0);

		sequence->output_inner_indexes.push_back(inner_scope->num_input_states);
		sequence->output_scope_depths.push_back(0);
		sequence->output_outer_is_local.push_back(false);
		sequence->output_outer_indexes.push_back(outer_scope->num_input_states);
	}
	for (set<Scope*>::iterator it = input_scopes_to_mod.begin();
			it != input_scopes_to_mod.end(); it++) {
		Scope* scope = *it;
		scope->num_input_states++;
	}

	solution->states[score_state->id] = score_state;
}

ScopeNode* finalize_sequence(vector<int>& scope_context,
							 vector<int>& node_context,
							 Sequence* new_sequence,
							 map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
							 map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	ScopeNode* new_scope_node = new ScopeNode();

	solution->scopes[new_sequence->scope->id] = new_sequence->scope;
	new_scope_node->inner_scope = new_sequence->scope;
	new_sequence->scope = NULL;

	new_scope_node->starting_node_ids = vector<int>{0};

	for (int i_index = 0; i_index < (int)new_sequence->input_types.size(); i_index++) {
		if (new_sequence->input_types[i_index] == INPUT_TYPE_STATE) {
			if (new_sequence->input_scope_depths[i_index] == 0) {
				new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_scope_node->input_inner_layers.push_back(0);
				new_scope_node->input_inner_is_local.push_back(false);
				new_scope_node->input_inner_indexes.push_back(new_sequence->input_inner_indexes[i_index]);
				new_scope_node->input_outer_is_local.push_back(new_sequence->input_outer_is_local[i_index]);
				new_scope_node->input_outer_indexes.push_back(new_sequence->input_outer_indexes[i_index]);
				new_scope_node->input_init_vals.push_back(0.0);
			} else {
				map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
					.find({new_sequence->input_scope_depths[i_index], {new_sequence->input_outer_is_local[i_index], new_sequence->input_outer_indexes[i_index]}});
				if (input_it != input_scope_depths_mappings.end()) {
					new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
					new_scope_node->input_inner_layers.push_back(0);
					new_scope_node->input_inner_is_local.push_back(false);
					new_scope_node->input_inner_indexes.push_back(new_sequence->input_inner_indexes[i_index]);
					new_scope_node->input_outer_is_local.push_back(false);
					new_scope_node->input_outer_indexes.push_back(input_it->second);
					new_scope_node->input_init_vals.push_back(0.0);
				} else {
					map<pair<int, pair<bool,int>>, int>::iterator output_it = output_scope_depths_mappings
						.find({new_sequence->input_scope_depths[i_index], {new_sequence->input_outer_is_local[i_index], new_sequence->input_outer_indexes[i_index]}});
					if (output_it != output_scope_depths_mappings.end()) {
						new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
						new_scope_node->input_inner_layers.push_back(0);
						new_scope_node->input_inner_is_local.push_back(false);
						new_scope_node->input_inner_indexes.push_back(new_sequence->input_inner_indexes[i_index]);
						new_scope_node->input_outer_is_local.push_back(false);
						new_scope_node->input_outer_indexes.push_back(output_it->second);
						new_scope_node->input_init_vals.push_back(0.0);

						input_scope_depths_mappings[{new_sequence->input_scope_depths[i_index],
							{new_sequence->input_outer_is_local[i_index], new_sequence->input_outer_indexes[i_index]}}] = output_it->second;
					} else {
						{
							int context_index = (int)scope_context.size()-1 - new_sequence->input_scope_depths[i_index];
							Scope* outer_scope = solution->scopes[scope_context[context_index]];
							ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[node_context[context_index]];
							Scope* inner_scope = scope_node->inner_scope;

							scope_node->input_types.push_back(INPUT_TYPE_STATE);
							scope_node->input_inner_layers.push_back(0);
							scope_node->input_inner_is_local.push_back(false);
							scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
							// not incremented yet
							scope_node->input_outer_is_local.push_back(new_sequence->input_outer_is_local[i_index]);
							scope_node->input_outer_indexes.push_back(new_sequence->input_outer_indexes[i_index]);
							scope_node->input_init_vals.push_back(0.0);
						}
						for (int c_index = new_sequence->input_scope_depths[i_index]-1; c_index >= 1; c_index--) {
							int context_index = (int)scope_context.size()-1 - c_index;
							Scope* outer_scope = solution->scopes[scope_context[context_index]];
							ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[node_context[context_index]];
							Scope* inner_scope = scope_node->inner_scope;

							int new_input_id = outer_scope->num_input_states;
							outer_scope->num_input_states++;

							scope_node->input_types.push_back(INPUT_TYPE_STATE);
							scope_node->input_inner_layers.push_back(0);
							scope_node->input_inner_is_local.push_back(false);
							scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
							// not incremented yet
							scope_node->input_outer_is_local.push_back(false);
							scope_node->input_outer_indexes.push_back(new_input_id);
							scope_node->input_init_vals.push_back(0.0);
						}
						{
							Scope* outer_scope = solution->scopes[scope_context.back()];

							int new_input_id = outer_scope->num_input_states;
							outer_scope->num_input_states++;

							new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
							new_scope_node->input_inner_layers.push_back(0);
							new_scope_node->input_inner_is_local.push_back(false);
							new_scope_node->input_inner_indexes.push_back(new_sequence->input_inner_indexes[i_index]);
							new_scope_node->input_outer_is_local.push_back(false);
							new_scope_node->input_outer_indexes.push_back(new_input_id);
							new_scope_node->input_init_vals.push_back(0.0);

							input_scope_depths_mappings[{new_sequence->input_scope_depths[i_index],
								{new_sequence->input_outer_is_local[i_index], new_sequence->input_outer_indexes[i_index]}}] = new_input_id;
						}
					}
				}
			}
		} else {
			new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
			new_scope_node->input_inner_layers.push_back(0);
			new_scope_node->input_inner_is_local.push_back(false);
			new_scope_node->input_inner_indexes.push_back(new_sequence->input_inner_indexes[i_index]);
			new_scope_node->input_outer_is_local.push_back(false);
			new_scope_node->input_outer_indexes.push_back(-1);
			new_scope_node->input_init_vals.push_back(new_sequence->input_init_vals[i_index]);
		}
	}

	for (int o_index = 0; o_index < (int)new_sequence->output_inner_indexes.size(); o_index++) {
		if (new_sequence->output_scope_depths[o_index] == 0) {
			new_scope_node->output_inner_indexes.push_back(new_sequence->output_inner_indexes[o_index]);
			new_scope_node->output_outer_is_local.push_back(new_sequence->output_outer_is_local[o_index]);
			new_scope_node->output_outer_indexes.push_back(new_sequence->output_outer_indexes[o_index]);
		} else {
			map<pair<int, pair<bool,int>>, int>::iterator it = output_scope_depths_mappings
				.find({new_sequence->output_scope_depths[o_index], {new_sequence->output_outer_is_local[o_index], new_sequence->output_outer_indexes[o_index]}});
			if (it != output_scope_depths_mappings.end()) {
				new_scope_node->output_inner_indexes.push_back(new_sequence->output_inner_indexes[o_index]);
				new_scope_node->output_outer_is_local.push_back(false);
				new_scope_node->output_outer_indexes.push_back(it->second);
			} else {
				int containing_index;
				int inner_index;
				{
					Scope* outer_scope = solution->scopes[scope_context.back()];

					int outer_index = -1;
					for (int i_index = 0; i_index < (int)new_scope_node->input_types.size(); i_index++) {
						if (new_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
							if (new_scope_node->input_inner_indexes[i_index] == new_sequence->output_inner_indexes[o_index]
									&& !new_scope_node->input_outer_is_local[i_index]) {
								outer_index = new_scope_node->input_outer_indexes[i_index];
								break;
							}
							/**
							 * - if was local but needs to be passed out, create new input
							 */
						}
					}

					if (outer_index == -1) {
						outer_index = outer_scope->num_input_states;
						outer_scope->num_input_states++;
					}

					new_scope_node->output_inner_indexes.push_back(new_sequence->output_inner_indexes[o_index]);
					new_scope_node->output_outer_is_local.push_back(false);
					new_scope_node->output_outer_indexes.push_back(outer_index);

					containing_index = outer_index;

					inner_index = outer_index;
				}
				for (int c_index = 1; c_index < new_sequence->output_scope_depths[o_index]-1; c_index++) {
					int context_index = (int)scope_context.size()-1 - c_index;
					Scope* outer_scope = solution->scopes[scope_context[context_index]];
					ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[node_context[context_index]];

					int outer_index = -1;
					for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
						if (scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
							if (scope_node->input_inner_indexes[i_index] == inner_index
									&& !scope_node->input_outer_is_local[i_index]) {
								outer_index = scope_node->input_outer_indexes[i_index];
								break;
							}
						}
					}

					if (outer_index == -1) {
						outer_index = outer_scope->num_input_states;
						outer_scope->num_input_states++;
					}

					scope_node->output_inner_indexes.push_back(inner_index);
					scope_node->output_outer_is_local.push_back(false);
					scope_node->output_outer_indexes.push_back(outer_index);

					inner_index = outer_index;
				}
				{
					Scope* outer_scope = solution->scopes[scope_context.back()];
					ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[node_context.back()];

					scope_node->output_inner_indexes.push_back(inner_index);
					scope_node->output_outer_is_local.push_back(new_sequence->output_outer_is_local[o_index]);
					scope_node->output_outer_indexes.push_back(new_sequence->output_outer_indexes[o_index]);
				}

				output_scope_depths_mappings[{new_sequence->output_scope_depths[o_index],
					{new_sequence->output_outer_is_local[o_index], new_sequence->output_outer_indexes[o_index]}}] = containing_index;
			}
		}
	}

	return new_scope_node;
}

void finalize_new_score_state(Scope* parent_scope,
							  map<int, int>& new_scope_node_id_mappings,
							  State* score_state,
							  vector<AbstractNode*>& nodes,
							  vector<vector<int>>& scope_contexts,
							  vector<vector<int>>& node_contexts,
							  vector<int>& obs_indexes) {
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)nodes[n_index];

			action_node->score_state_scope_contexts.push_back(scope_contexts[n_index]);
			action_node->score_state_node_contexts.push_back(node_contexts[n_index]);
			for (int c_index = 0; c_index < (int)action_node->score_state_node_contexts.back().size()-1; c_index++) {
				if (action_node->score_state_node_contexts.back()[c_index] == -1) {
					int inner_scope_id = action_node->score_state_scope_contexts.back()[c_index+1];
					action_node->score_state_node_contexts.back()[c_index] = new_scope_node_id_mappings[inner_scope_id];
					break;
				}
			}
			// in case is new action
			if (action_node->score_state_node_contexts.back().back() == -1) {
				action_node->score_state_node_contexts.back().back() = action_node->id;
			}
			action_node->score_state_defs.push_back(score_state);
			action_node->score_state_network_indexes.push_back(n_index);
		} else if (nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)nodes[n_index];

			scope_node->score_state_scope_contexts.push_back(scope_contexts[n_index]);
			scope_node->score_state_node_contexts.push_back(node_contexts[n_index]);
			for (int c_index = 0; c_index < (int)scope_node->score_state_node_contexts.back().size()-1; c_index++) {
				if (scope_node->score_state_node_contexts.back()[c_index] == -1) {
					int inner_scope_id = scope_node->score_state_scope_contexts.back()[c_index+1];
					scope_node->score_state_node_contexts.back()[c_index] = new_scope_node_id_mappings[inner_scope_id];
					break;
				}
			}
			scope_node->score_state_obs_indexes.push_back(obs_indexes[n_index]);
			scope_node->score_state_defs.push_back(score_state);
			scope_node->score_state_network_indexes.push_back(n_index);
		} else {
			BranchNode* branch_node = (BranchNode*)nodes[n_index];

			branch_node->score_state_scope_contexts.push_back(scope_contexts[n_index]);
			branch_node->score_state_node_contexts.push_back(node_contexts[n_index]);
			for (int c_index = 0; c_index < (int)branch_node->score_state_node_contexts.back().size()-1; c_index++) {
				if (branch_node->score_state_node_contexts.back()[c_index] == -1) {
					int inner_scope_id = branch_node->score_state_scope_contexts.back()[c_index+1];
					branch_node->score_state_node_contexts.back()[c_index] = new_scope_node_id_mappings[inner_scope_id];
					break;
				}
			}
			branch_node->score_state_defs.push_back(score_state);
			branch_node->score_state_network_indexes.push_back(n_index);
		}
	}

	solution->states[score_state->id] = score_state;
}
