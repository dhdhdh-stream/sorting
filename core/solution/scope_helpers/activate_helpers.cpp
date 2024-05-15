#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "abstract_experiment.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "new_action_experiment.h"
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

	EvalHistory* eval_history;
	if (solution->explore_id == this->id) {
		if (solution->explore_type == EXPLORE_TYPE_SCORE) {
			eval_history = new EvalHistory(this->eval);

			this->eval->activate_start(problem,
									   run_helper,
									   eval_history);
		} else {

		}
	}

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (run_helper.exceeded_limit
				|| curr_node == NULL) {
			break;
		}

		if (this->num_actions_until_random > 0) {
			this->num_actions_until_random--;
			if (this->num_actions_until_random == 0) {
				this->num_actions_until_random = -1;

				random_sequence(curr_node,
								problem,
								context,
								run_helper);
			}
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 run_helper,
							 history);
	}

	if (solution->explore_id == this->id) {
		if (solution->explore_type == EXPLORE_TYPE_SCORE) {
			this->eval->activate_end(problem,
									 run_helper,
									 history);
		} else {

		}
	}

	if (history->callback_experiment != NULL) {
		switch (history->experiment_history->experiment->type) {
		history->callback_experiment->back_activate(
			history,
			eval_history,
			run_helper);
	}

	if (solution->explore_id == this->id) {
		if (solution->explore_type == EXPLORE_TYPE_SCORE) {
			if (run_helper.experiments_seen_order.size() == 0) {
				if (!run_helper.exceeded_limit) {
					create_experiment(history);
				}
			}
		} else {
			this->eval->experiment_activate(problem,
											run_helper);
		}
	}

	run_helper.curr_depth--;
}
