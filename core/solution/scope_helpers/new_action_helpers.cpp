#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void new_action_node_activate_helper(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->new_action_activate(curr_node,
									  problem,
									  history->node_histories);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->new_action_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history->node_histories);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->new_action_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* node = (InfoScopeNode*)curr_node;
			node->new_action_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* node = (InfoBranchNode*)curr_node;
			node->new_action_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history->node_histories);
		}

		break;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
	}
}

void Scope::new_action_activate(AbstractNode* starting_node,
								set<AbstractNode*>& included_nodes,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								ScopeHistory* history) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (run_helper.exceeded_limit
				|| curr_node == NULL
				|| included_nodes.find(curr_node) == included_nodes.end()) {
			break;
		}

		new_action_node_activate_helper(curr_node,
										problem,
										context,
										run_helper,
										history);
	}

	run_helper.curr_depth--;
}

void Scope::new_action_activate(Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								ScopeHistory* history) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (run_helper.exceeded_limit
				|| curr_node == NULL) {
			break;
		}

		new_action_node_activate_helper(curr_node,
										problem,
										context,
										run_helper,
										history);
	}

	run_helper.curr_depth--;
}
