#if defined(MDEBUG) && MDEBUG

#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_verify_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			ScopeNodeHistory* node_history = new ScopeNodeHistory();
			node_history->index = (int)history->node_histories.size();
			history->node_histories[node] = node_history;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  node_history);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->verify_activate(curr_node,
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

void Scope::verify_activate(Problem* problem,
							vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* history) {
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

		node_verify_activate_helper(curr_node,
									problem,
									context,
									run_helper,
									history);
	}

	run_helper.curr_depth--;
}

#endif /* MDEBUG */