#include "scope.h"

#include <iostream>

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper);
		}

		break;
	}
}

void Scope::activate(Problem* problem,
					 vector<ContextLayer>& context,
					 RunHelper& run_helper) {
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	if (this->new_action_experiment != NULL) {
		this->new_action_experiment->pre_activate(context,
												  run_helper);
	}

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 run_helper);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	if (this->new_action_experiment != NULL) {
		if (run_helper.experiment_histories.size() == 1
				&& run_helper.experiment_histories.back()->experiment == this->new_action_experiment) {
			this->new_action_experiment->back_activate(
				context,
				run_helper);
		}
	}

	if (run_helper.experiments_seen_order.size() == 0) {
		uniform_int_distribution<int> select_distribution(0, num_actions-1);
		int select_index = select_distribution(generator);
		if (select_index < (int)context.back().nodes_seen.size()) {
			run_helper.selected_node = context.back().nodes_seen[select_index];
		}
	}

	for (int b_index = 0; b_index < (int)context.back().nodes_seen.size(); b_index++) {
		AbstractNode* node = context.back().nodes_seen[b_index].first;
		if (node->type == NODE_TYPE_BRANCH) {
			run_helper.branch_node_ancestors.erase(node);
		}
	}

	context.pop_back();
}
