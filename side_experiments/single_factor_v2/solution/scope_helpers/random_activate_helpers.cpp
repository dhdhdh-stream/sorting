#include "scope.h"

using namespace std;

void Scope::random_activate(vector<int>& starting_node_ids,
							vector<int>& scope_context,
							vector<int>& node_context,
							int& exit_depth,
							int& exit_node_id,
							int& num_nodes,
							ScopeHistory* history) {
	exit_depth = -1;

	// this->is_loop == false

	history->node_histories.push_back(vector<AbstractNodeHistory*>());

	int curr_node_id = starting_node_ids[0];
	starting_node_ids.erase(starting_node_ids.begin());
	if (starting_node_ids.size() > 0) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);

		uniform_int_distribution<int> distribution(0, 1);
		if (distribution(generator) == 0) {
			int inner_exit_depth;
			int inner_exit_node_id;

			scope_node->halfway_random_activate(starting_node_ids,
												scope_context,
												node_context,
												inner_exit_depth,
												inner_exit_node_id,
												num_nodes,
												node_history);

			if (inner_exit_depth == -1) {
				curr_node_id = scope_node->next_node_id;
			} else if (inner_exit_depth == 0) {
				curr_node_id = inner_exit_node_id;
			} else {
				exit_depth = inner_exit_depth-1;
				exit_node_id = inner_exit_node_id;
			}
		} else {
			curr_node_id = scope_node->next_node_id;
			/**
			 * - don't worry about missing early exits
			 *   - can change distribution, but won't lead to missing nodes
			 *     - as early exits will always be on path
			 *   - same as for loops in the following
			 */
		}
	}

	while (true) {
		if (curr_node_id == -1 || exit_depth != -1) {
			break;
		}

		node_random_activate_helper(curr_node_id,
									scope_context,
									node_context,
									exit_depth,
									exit_node_id,
									num_nodes,
									history);
	}
}

void Scope::node_random_activate_helper(int& curr_node_id,
										vector<int>& scope_context,
										vector<int>& node_context,
										int& exit_depth,
										int& exit_node_id,
										ScopeHistory* history) {
	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		ActionNodeHistory* node_history = new ActionNodeHistory(action_node);
		history->node_histories[0].push_back(node_history);

		num_nodes++;

		curr_node_id = action_node->next_node_id;
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);

		uniform_int_distribution<int> distribution(0, 1);
		if (distribution(generator) == 0) {
			int inner_exit_depth;
			int inner_exit_node_id;

			scope_node->random_activate(scope_context,
										node_context,
										inner_exit_depth,
										inner_exit_node_id,
										num_nodes,
										node_history);

			num_nodes++;

			if (inner_exit_depth == -1) {
				curr_node_id = scope_node->next_node_id;
			} else if (inner_exit_depth == 0) {
				curr_node_id = inner_exit_node_id;
			} else {
				exit_depth = inner_exit_depth-1;
				exit_node_id = inner_exit_node_id;
			}
		} else {
			curr_node_id = scope_node->next_node_id;
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
		history->node_histories[0].push_back(node_history);

		bool& is_branch;
		branch_node->random_activate(scope_context,
									 node_context,
									 is_branch,
									 node_history);

		num_nodes++;

		if (is_branch) {
			curr_node_id = branch_node->branch_next_node_id;
		} else {
			curr_node_id = branch_node->original_next_node_id;
		}
	} else {
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth;
			exit_node_id = exit_node->exit_node_id;
		}
	}
}
