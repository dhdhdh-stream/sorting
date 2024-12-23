#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "new_action_experiment.h"
#include "return_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_experiment_activate_helper(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	}
}

void Scope::experiment_activate(Problem* problem,
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

		node_experiment_activate_helper(curr_node,
										problem,
										context,
										run_helper);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	if (this->new_action_experiment != NULL) {
		if (run_helper.experiment_histories.size() == 1
				&& run_helper.experiment_histories.back()->experiment == this->new_action_experiment) {
			this->new_action_experiment->back_activate(
				context,
				run_helper);
		}
	}

	context.pop_back();
}
