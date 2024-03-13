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
					   run_helper,
					   node_history);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;
		node->activate(curr_node,
					   problem,
					   context,
					   exit_depth,
					   run_helper,
					   history->node_histories);
	} else {
		ExitNode* node = (ExitNode*)curr_node;
		node->activate(context,
					   exit_depth,
					   run_helper);
	}
}

void Scope::activate(Problem* problem,
					 vector<ContextLayer>& context,
					 int& exit_depth,
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
				|| run_helper.throw_id != -1
				|| exit_depth != 0
				|| curr_node == NULL) {
			break;
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 exit_depth,
							 run_helper,
							 history);
	}

	if (this->sample_run == NULL) {
		uniform_int_distribution<int> null_sample_distribution(0, 9);
		if (null_sample_distribution(generator) == 0) {
			this->sample_run = new ScopeHistory(history);
		}
	} else {
		uniform_int_distribution<int> non_null_sample_distribution(0, 999);
		if (non_null_sample_distribution(generator) == 0) {
			delete this->sample_run;
			this->sample_run = new ScopeHistory(history);
		}
	}

	if (history->pass_through_experiment_history != NULL) {
		history->pass_through_experiment_history->scope_history = new ScopeHistory(history);
	}

	run_helper.curr_depth--;
}
