#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  ScopeHistory* history) {
	if (curr_node->type == NODE_TYPE_ACTION) {
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
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
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
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;
		node->activate(curr_node,
					   problem,
					   context,
					   exit_depth,
					   exit_node,
					   run_helper,
					   history->node_histories);
	} else {
		ExitNode* node = (ExitNode*)curr_node;
		if (node->is_throw) {
			run_helper.throw_id = node->throw_id;
			curr_node = NULL;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->next_node;
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

	AbstractNode* curr_node = this->starting_node;
	while (true) {
		if (run_helper.exceeded_limit
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper,
							 history);
	}

	if (history->pass_through_experiment_history != NULL) {
		history->pass_through_experiment_history->scope_history = new ScopeHistory(history);
	}

	run_helper.curr_depth--;
}
