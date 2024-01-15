#include "scope.h"

#include <iostream>
#include <stdexcept>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "state_network.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

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

		BranchNodeHistory* node_history = new BranchNodeHistory(node);
		history->node_histories.push_back(node_history);
		node->activate(curr_node,
					   problem,
					   context,
					   exit_depth,
					   exit_node,
					   run_helper);
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		if (node->is_exit) {
			run_helper.has_exited = true;
		} else {
			if (node->exit_depth == 0) {
				curr_node = node->exit_node;
			} else {
				exit_depth = node->exit_depth-1;
				exit_node = node->exit_node;
			}
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
		if (run_helper.has_exited
				|| exit_depth != -1
				|| curr_node == NULL
				|| run_helper.exceeded_limit) {
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

	if (history->inner_pass_through_experiment != NULL) {
		history->inner_pass_through_experiment->parent_scope_end_activate(
			context,
			run_helper,
			history);
	}

	run_helper.curr_depth--;
}
