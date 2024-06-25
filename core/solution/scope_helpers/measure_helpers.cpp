#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "problem.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_set.h"

using namespace std;

void measure_node_activate_helper(AbstractNode*& curr_node,
								  Metrics& metrics,
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
			node->measure_activate(metrics,
								   curr_node,
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
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* node = (BranchEndNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	}
}

void Scope::measure_activate(Metrics& metrics,
							 Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* history) {
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

	if (context.size() > solution->scopes.size() + 1) {
		run_helper.exceeded_limit = true;
		return;
	}

	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	context.back().scope_history = history;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		run_helper.num_actions++;
		if (run_helper.num_actions > solution->num_actions_limit) {
			run_helper.exceeded_limit = true;
			break;
		}

		measure_node_activate_helper(curr_node,
									 metrics,
									 problem,
									 context,
									 run_helper,
									 history);
	}

	if (metrics.experiment_scope == this) {
		metrics.scope_histories.push_back(history->deep_copy());
	}
	if (metrics.new_scope == this) {
		metrics.new_scope_histories.push_back(history->deep_copy());
	}

	context.pop_back();
}
