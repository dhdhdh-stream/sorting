#if defined(MDEBUG) && MDEBUG

#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_verify_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper);
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->verify_activate(curr_node,
								  problem,
								  context,
								  run_helper);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* node = (ObsNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}
		break;
	}

	run_helper.num_actions++;
}

void Scope::verify_activate(Problem* problem,
							vector<ContextLayer>& context,
							RunHelper& run_helper) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_verify_activate_helper(curr_node,
									problem,
									context,
									run_helper);
	}

	context.pop_back();
}

#endif /* MDEBUG */