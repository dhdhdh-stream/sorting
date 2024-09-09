#include "scope.h"

#include <iostream>

#include "absolute_return_node.h"
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
	case NODE_TYPE_ABSOLUTE_RETURN:
		{
			AbsoluteReturnNode* node = (AbsoluteReturnNode*)curr_node;
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
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		if (context[c_index].scope == this) {
			run_helper.exceeded_limit = true;
			return;
		}
	}

	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	context.back().starting_location = problem->get_location();

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

	delete context.back().starting_location;
	for (map<AbstractNode*, ProblemLocation*>::iterator it = context.back().location_history.begin();
			it != context.back().location_history.end(); it++) {
		delete it->second;
	}

	context.pop_back();
}
