#include "solution.h"

using namespace std;

void Solution::finalize_state(Scope* parent_scope,
							  State* score_state,
							  BranchNode* new_branch_node) {
	int new_local_id = parent_scope->num_local_states;
	parent_scope->num_local_states++;

	vector<ScopeNode*> local_scope_nodes_to_mod;
	vector<Scope*> input_scopes_to_mod;
	vector<ScopeNode*> input_scope_nodes_to_mod;

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

			if (action_node->score_state_scope_contexts.size() == 1) {
				action_node->state_is_local.push_back(true);
				action_node->state_ids.push_back(new_local_id);
				action_node->state_defs.push_back(score_state);
				action_node->state_network_indexes.push_back(n_index);
			} else {
				Scope* containing_scope = solution->scopes[action_node->score_state_scope_contexts.back()];
				int new_

			}

			action_node->score_state_scope_contexts.erase(action_node->score_state_scope_contexts.begin() + score_state_index);
			action_node->score_state_node_contexts.erase(action_node->score_state_node_contexts.begin() + score_state_index);
			action_node->score_state_defs.erase(action_node->score_state_defs.begin() + score_state_index);
			action_node->score_state_network_indexes.erase(action_node->score_state_network_indexes.begin() + score_state_index);
		} else if (score_state->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)score_state->nodes[n_index];

		} else {
			BranchNode* branch_node = (BranchNode*)score_state->nodes[n_index];

		}
	}
}
