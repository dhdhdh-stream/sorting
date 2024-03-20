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

void node_measure_activate_helper(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  RunHelper& run_helper,
								  Metrics& metrics,
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
			node->measure_activate(curr_node,
								   problem,
								   context,
								   exit_depth,
								   exit_node,
								   run_helper,
								   metrics,
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
			if (node->throw_id != -1) {
				run_helper.throw_id = node->throw_id;
			} else {
				exit_depth = node->exit_depth-1;
				exit_node = node->next_node;
			}
		}

		break;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
	}
}

void Scope::measure_activate(AbstractNode* starting_node,
							 Problem* problem,
							 vector<ContextLayer>& context,
							 int& exit_depth,
							 AbstractNode*& exit_node,
							 RunHelper& run_helper,
							 Metrics& metrics,
							 ScopeHistory* history) {
	map<Scope*, int>::iterator it = metrics.scope_counts.find(this);
	if (it == metrics.scope_counts.end()) {
		metrics.scope_counts[this] = 1;
	} else {
		it->second++;
	}

	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (run_helper.exceeded_limit
				|| run_helper.throw_id != -1
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		if (context.size() > 1) {
			ScopeNode* scope_node = (ScopeNode*)context[context.size()-2].node;
			if (scope_node->exit_nodes.find(curr_node) != scope_node->exit_nodes.end()) {
				break;
			}
		}

		node_measure_activate_helper(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper,
									 metrics,
									 history);
	}

	run_helper.curr_depth--;
}
