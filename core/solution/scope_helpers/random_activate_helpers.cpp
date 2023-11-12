#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "scope_node.h"

using namespace std;

void node_random_activate_helper(AbstractNode*& curr_node,
								 vector<int>& scope_context,
								 vector<int>& node_context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 int& num_nodes,
								 ScopeHistory* history) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;

		ActionNodeHistory* node_history = new ActionNodeHistory(node);
		history->node_histories[0].push_back(node_history);

		num_nodes++;

		curr_node = node->next_node;
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;

		int inner_exit_depth = -1;
		AbstractNode* inner_exit_node = NULL;

		node->random_activate(scope_context,
							  node_context,
							  inner_exit_depth,
							  inner_exit_node,
							  num_nodes,
							  history->node_histories[0]);

		if (inner_exit_depth == -1) {
			curr_node = node->next_node;
		} else if (inner_exit_depth == 0) {
			curr_node = inner_exit_node;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node = inner_exit_node;
		}
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->random_activate(is_branch,
							  scope_context,
							  node_context,
							  num_nodes,
							  history->node_histories[0]);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		if (node->exit_depth == 0) {
			curr_node = node->exit_node;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->exit_node;
		}
	}
}

void Scope::random_activate(vector<AbstractNode*>& starting_nodes,
							vector<int>& scope_context,
							vector<int>& node_context,
							int& exit_depth,
							AbstractNode*& exit_node,
							int& num_nodes,
							ScopeHistory* history) {
	// this->is_loop == false

	history->node_histories.push_back(vector<AbstractNodeHistory*>());

	AbstractNode* curr_node = starting_nodes[0];
	starting_nodes.erase(starting_nodes.begin());
	if (starting_nodes.size() > 0) {
		ScopeNode* scope_node = (ScopeNode*)curr_node;

		int inner_exit_depth = -1;
		AbstractNode* inner_exit_node = NULL;

		scope_node->halfway_random_activate(starting_nodes,
											scope_context,
											node_context,
											inner_exit_depth,
											inner_exit_node,
											num_nodes,
											history->node_histories[0]);

		if (inner_exit_depth == -1) {
			curr_node = scope_node->next_node;
		} else if (inner_exit_depth == 0) {
			curr_node = inner_exit_node;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node = inner_exit_node;
		}
	}

	while (true) {
		if (exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_random_activate_helper(curr_node,
									scope_context,
									node_context,
									exit_depth,
									exit_node,
									num_nodes,
									history);
	}
}
