#if defined(MDEBUG) && MDEBUG

#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "markov_node.h"
#include "minesweeper.h"
#include "return_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void new_action_capture_verify_node_activate_helper(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->new_action_capture_verify_activate(
				curr_node,
				problem,
				context,
				run_helper);
		}

		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	case NODE_TYPE_MARKOV:
		{
			MarkovNode* node = (MarkovNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	}
}

void Scope::new_action_capture_verify_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		if (context[c_index].scope == this) {
			run_helper.exceeded_limit = true;
			return;
		}
	}

	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		new_action_capture_verify_node_activate_helper(
			curr_node,
			problem,
			context,
			run_helper);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	context.pop_back();
}

#endif /* MDEBUG */