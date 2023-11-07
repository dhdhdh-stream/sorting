#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "branch_stub_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void Scope::activate(vector<AbstractNode*>& starting_nodes,
					 vector<map<int, StateStatus>>& starting_input_state_vals,
					 vector<map<int, StateStatus>>& starting_local_state_vals,
					 Problem& problem,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 AbstractNode*& exit_node,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	history->node_histories.push_back(vector<AbstractNodeHistory*>());

	AbstractNode* curr_node = starting_nodes[0];
	starting_nodes.erase(starting_nodes.begin());
	if (starting_nodes.size() > 0) {
		ScopeNode* scope_node = (ScopeNode*)curr_node;
		ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(scope_node_history);
		scope_node->halfway_activate(starting_nodes,
									 starting_input_state_vals,
									 starting_local_state_vals,
									 curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper,
									 scope_node_history);
	}

	while (true) {
		if (curr_node == NULL || exit_depth != -1 || run_helper.exceeded_depth) {
			break;
		}

		node_activate_helper(0,
							 curr_node,
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

void Scope::node_activate_helper(int iter_index,
								 AbstractNode*& curr_node,
								 Problem& problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper,
								 ScopeHistory* history) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)curr_node;
		ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
		history->node_histories[iter_index].push_back(action_node_history);
		action_node->activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper,
							  action_node_history);
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)curr_node;
		ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[iter_index].push_back(scope_node_history);
		scope_node->activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper,
							 scope_node_history);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)curr_node;

		bool is_branch;
		branch_node->activate(is_branch,
							  context);

		if (is_branch) {
			curr_node = branch_node->branch_next_node;
		} else {
			curr_node = branch_node->original_next_node;
		}
	} else {
		ExitNode* exit_node = (ExitNode*)curr_node;

		if (exit_node->exit_depth == 0) {
			curr_node = exit_node->exit_node;
		} else {
			exit_depth = exit_node->exit_depth-1;
			exit_node = exit_node->exit_node;
		}
	}
}
