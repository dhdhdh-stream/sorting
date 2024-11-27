#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "scope_node.h"
#include "solution.h"

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
	}
}

void Scope::result_activate(Problem* problem,
							vector<ContextLayer>& context,
							RunHelper& run_helper) {
	context.push_back(ContextLayer());

	context.back().scope_id = this->id;

	if (this->new_scope_experiment != NULL) {
		this->new_scope_experiment->pre_activate(context,
												 run_helper);
	}

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_result_activate_helper(curr_node,
									problem,
									context,
									run_helper);
	}

	context.pop_back();
}
