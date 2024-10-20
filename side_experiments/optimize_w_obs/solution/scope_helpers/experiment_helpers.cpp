#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "condition_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "scope_node.h"

using namespace std;

void node_experiment_activate_helper(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
		}

		break;
	case NODE_TYPE_CONDITION:
		{
			ConditionNode* node = (ConditionNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  history);
		}

		break;
	}
}

void Scope::experiment_activate(Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper,
								ScopeHistory* history) {
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		if (context[c_index].scope_id == this->id) {
			run_helper.exceeded_limit = true;
			return;
		}
	}

	context.push_back(ContextLayer());

	context.back().scope_id = this->id;

	if (this->new_scope_experiment != NULL) {
		this->new_scope_experiment->pre_activate(context,
												 run_helper);
	}

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_experiment_activate_helper(curr_node,
										problem,
										context,
										run_helper,
										history);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	if (this->new_scope_experiment != NULL) {
		if (run_helper.experiment_histories.size() == 1
				&& run_helper.experiment_histories.back()->experiment == this->new_scope_experiment) {
			this->new_scope_experiment->back_activate(
				context,
				run_helper,
				history);
		}
	}

	context.pop_back();
}
