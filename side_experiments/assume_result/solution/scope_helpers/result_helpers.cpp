#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "return_node.h"
#include "scope_node.h"

using namespace std;

void node_result_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->result_activate(curr_node,
								  problem,
								  context,
								  run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->result_activate(curr_node,
								  problem,
								  context,
								  run_helper);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->result_activate(curr_node,
								  problem,
								  context,
								  run_helper);
		}

		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
			node->result_activate(curr_node,
								  problem,
								  context,
								  run_helper);
		}

		break;
	}
}

void Scope::result_activate(Problem* problem,
							vector<ContextLayer>& context,
							RunHelper& run_helper) {
	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_result_activate_helper(curr_node,
									problem,
									context,
									run_helper);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	for (int b_index = 0; b_index < (int)context.back().nodes_seen.size(); b_index++) {
		AbstractNode* node = context.back().nodes_seen[b_index].first;
		if (node->type == NODE_TYPE_BRANCH) {
			run_helper.branch_node_ancestors.erase(node);
		}
	}

	context.pop_back();
}
