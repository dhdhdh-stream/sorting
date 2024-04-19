#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "abstract_experiment.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			ActionNodeHistory* node_history = new ActionNodeHistory(node);
			history->node_histories.push_back(node_history);
			node->activate(curr_node,
						   problem,
						   context,
						   exit_depth,
						   exit_node,
						   run_helper,
						   node_history);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			ScopeNodeHistory* node_history = new ScopeNodeHistory(node);
			history->node_histories.push_back(node_history);
			node->activate(curr_node,
						   problem,
						   context,
						   exit_depth,
						   exit_node,
						   run_helper,
						   node_history);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   exit_depth,
						   exit_node,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_EXIT:
		{
			ExitNode* node = (ExitNode*)curr_node;
			exit_depth = node->exit_depth-1;
			exit_node = node->next_node;
		}

		break;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
	}

	if (num_actions_until_experiment != -1) {
		if (num_actions_until_experiment > 0) {
			num_actions_until_experiment--;
		}
	}
}

void Scope::activate(Problem* problem,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 AbstractNode*& exit_node,
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
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		if (num_actions_until_experiment == 0) {
			inner_experiment(problem,
							 run_helper);
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper,
							 history);
	}

	if (history->experiment_history != NULL) {
		history->experiment_history->scope_history = new ScopeHistory(history);
	}

	run_helper.curr_depth--;
}
