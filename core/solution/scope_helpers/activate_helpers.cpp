#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "abstract_experiment.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
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
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* node = (InfoScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* node = (InfoBranchNode*)curr_node;
			node->activate(curr_node,
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

	if (solution->num_actions_until_experiment > 0) {
		solution->num_actions_until_experiment--;
	}
}

void Scope::activate(Problem* problem,
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

		// if (this->is_improve_eval
		// 		&& solution->num_actions_until == 0) {
		// 	random_sequence(problem);
		// }

		node_activate_helper(curr_node,
							 problem,
							 context,
							 run_helper,
							 history);
	}

	if (history->experiment_history != NULL) {
		history->experiment_history->scope_history = new ScopeHistory(history);
	}

	// if (this->is_improve_eval) {
	// 	this->eval->experiment_activate();
	// }

	run_helper.curr_depth--;
}
