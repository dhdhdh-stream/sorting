#if defined(MDEBUG) && MDEBUG

#include "scope.h"

using namespace std;

void node_verify_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper,
								 ScopeHistory* history) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;
		node->activate(curr_node,
					   problem,
					   context,
					   exit_depth,
					   exit_node,
					   run_helper,
					   history->node_histories);
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;
		node->verify_activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper,
							  history->node_histories);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;
		node->verify_activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper,
							  history->node_histories);
	} else {
		ExitNode* node = (ExitNode*)curr_node;
		exit_depth = node->exit_depth-1;
		exit_node = node->exit_node;
	}
}

void Scope::verify_activate(Problem* problem,
							vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper,
							ScopeHistory* history) {
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	AbstractNode* curr_node = this->starting_node;
	while (true) {
		if (run_helper.exceeded_limit
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_verify_activate_helper(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper,
									history);
	}

	run_helper.curr_depth--;
}

#endif /* MDEBUG */