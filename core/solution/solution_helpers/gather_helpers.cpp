#include "solution_helpers.h"

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_helper(vector<AbstractScope*>& scope_context,
							vector<AbstractNode*>& node_context,
							vector<vector<AbstractScope*>>& possible_scope_contexts,
							vector<vector<AbstractNode*>>& possible_node_contexts,
							vector<int>& possible_obs_indexes,
							AbstractScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		node_context.back() = it->first;

		switch (it->first->type) {
		case NODE_TYPE_ACTION:
		case NODE_TYPE_BRANCH_END:
			for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(o_index);
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				node_context.back() = it->first;

				uniform_int_distribution<int> distribution(0, 2);
				if (distribution(generator) != 0) {
					gather_possible_helper(scope_context,
										   node_context,
										   possible_scope_contexts,
										   possible_node_contexts,
										   possible_obs_indexes,
										   scope_node_history->scope_history);
				}

				node_context.back() = NULL;
			}

			break;
		case NODE_TYPE_BRANCH:
		case NODE_TYPE_INFO_BRANCH:
			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_obs_indexes.push_back(-1);

			break;
		}

		node_context.back() = NULL;
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void gather_new_action_included_nodes_branch_helper(
		AbstractNode* start_node,
		vector<AbstractNode*>& included_nodes) {
	BranchEndNode* branch_end_node;
	switch (start_node->type) {
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)start_node;
			branch_end_node = branch_node->branch_end_node;
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)start_node;
			branch_end_node = info_branch_node->branch_end_node;
		}
		break;
	}

	// original
	{
		AbstractNode* curr_node;
		switch (start_node->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)start_node;
				curr_node = branch_node->original_next_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)start_node;
				curr_node = info_branch_node->original_next_node;
			}
			break;
		}

		while (curr_node != branch_end_node) {
			included_nodes.push_back(curr_node);

			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;
					curr_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;
					curr_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					gather_new_action_included_nodes_branch_helper(
						curr_node,
						included_nodes);

					curr_node = branch_node->branch_end_node->next_node;
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* info_branch_node = (InfoBranchNode*)curr_node;

					gather_new_action_included_nodes_branch_helper(
						curr_node,
						included_nodes);

					curr_node = info_branch_node->branch_end_node->next_node;
				}
				break;
			// if NODE_TYPE_BRANCH_END, then has to be end
			}
		}
	}

	// branch
	{
		AbstractNode* curr_node;
		switch (start_node->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)start_node;
				curr_node = branch_node->branch_next_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)start_node;
				curr_node = info_branch_node->branch_next_node;
			}
			break;
		}

		while (curr_node != branch_end_node) {
			included_nodes.push_back(curr_node);

			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;
					curr_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;
					curr_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					gather_new_action_included_nodes_branch_helper(
						curr_node,
						included_nodes);

					curr_node = branch_node->branch_end_node->next_node;
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* info_branch_node = (InfoBranchNode*)curr_node;

					gather_new_action_included_nodes_branch_helper(
						curr_node,
						included_nodes);

					curr_node = info_branch_node->branch_end_node->next_node;
				}
				break;
			// if NODE_TYPE_BRANCH_END, then has to be end
			}
		}
	}

	included_nodes.push_back(branch_end_node);
}

void gather_new_action_included_nodes(AbstractNode* starting_node,
									  vector<AbstractNode*>& included_nodes) {
	geometric_distribution<int> num_included_nodes_distribution(0.4);
	int num_included_nodes = 3 + num_included_nodes_distribution(generator);

	AbstractNode* curr_node = starting_node;

	while (true) {
		if (curr_node == NULL
				|| curr_node->type == NODE_TYPE_BRANCH_END
				|| (int)included_nodes.size() >= num_included_nodes) {
			break;
		}

		included_nodes.push_back(curr_node);

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)curr_node;
				curr_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)curr_node;
				curr_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)curr_node;

				gather_new_action_included_nodes_branch_helper(
					curr_node,
					included_nodes);

				curr_node = branch_node->branch_end_node->next_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)curr_node;

				gather_new_action_included_nodes_branch_helper(
					curr_node,
					included_nodes);

				curr_node = info_branch_node->branch_end_node->next_node;
			}
			break;
		}
	}
}
