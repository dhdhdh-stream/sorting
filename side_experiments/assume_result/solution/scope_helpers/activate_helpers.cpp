#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "new_action_experiment.h"
#include "return_node.h"
#include "scope_node.h"

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
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
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
	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	if (this->new_action_experiment != NULL) {
		this->new_action_experiment->pre_activate(context,
												  run_helper);
	}

	uniform_int_distribution<int> new_action_location_distribution(0, 1);
	bool new_action_is_front = new_action_location_distribution(generator) == 0;

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

	if (this->new_action_experiment != NULL
			&& new_action_is_front) {
		if (run_helper.experiment_histories.size() == 1
				&& run_helper.experiment_histories.back()->experiment == this->new_action_experiment) {
			this->new_action_experiment->back_activate(
				context,
				run_helper);
		}
	}

	for (int b_index = 0; b_index < (int)context.back().nodes_seen.size(); b_index++) {
		AbstractNode* node = context.back().nodes_seen[b_index].first;
		if (node->type == NODE_TYPE_BRANCH) {
			run_helper.branch_node_ancestors.erase(node);
		}
	}

	context.back().nodes_seen.clear();

	if (!run_helper.exceeded_limit) {
		{
			map<AbstractNode*, pair<int,int>>::iterator it
				= context.back().location_history.find(this->nodes[0]);
			Minesweeper* minesweeper = (Minesweeper*)problem;
			minesweeper->current_x = it->second.first;
			minesweeper->current_y = it->second.second;
		}

		curr_node = this->nodes[1];
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

		if (this->new_action_experiment != NULL
				&& !new_action_is_front) {
			if (run_helper.experiment_histories.size() == 1
					&& run_helper.experiment_histories.back()->experiment == this->new_action_experiment) {
				this->new_action_experiment->back_activate(
					context,
					run_helper);
			}
		}

		for (int b_index = 0; b_index < (int)context.back().nodes_seen.size(); b_index++) {
			AbstractNode* node = context.back().nodes_seen[b_index].first;
			if (node->type == NODE_TYPE_BRANCH) {
				run_helper.branch_node_ancestors.erase(node);
			}
		}
	}

	context.pop_back();
}
