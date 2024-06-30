#if defined(MDEBUG) && MDEBUG

#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void new_action_capture_verify_node_activate_helper(
		AbstractNode*& curr_node,
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
			node->new_action_capture_verify_activate(
				curr_node,
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
}

void Scope::new_action_capture_verify_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* history) {
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

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

		new_action_capture_verify_node_activate_helper(
			curr_node,
			problem,
			context,
			run_helper,
			history);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = history->node_histories.begin();
			it != history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_BRANCH:
		case NODE_TYPE_INFO_BRANCH:
			run_helper.branch_node_ancestors.erase(it->first);
			break;
		}
	}

	context.pop_back();
}

#endif /* MDEBUG */