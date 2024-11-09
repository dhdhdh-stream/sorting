#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "return_node.h"
#include "scope_node.h"

using namespace std;

void node_continue_activate_helper(AbstractNode*& curr_node,
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

void Scope::continue_activate(Problem* problem,
							  vector<ContextLayer>& context,
							  int curr_layer,
							  RunHelper& run_helper) {
	AbstractNode* curr_node;
	if (curr_layer == (int)context.size()-1) {
		curr_node = context.back().node;
	} else {
		curr_node = context[curr_layer].node;

		ScopeNode* node = (ScopeNode*)curr_node;
		node->continue_activate(curr_node,
								problem,
								context,
								curr_layer+1,
								run_helper);
	}

	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_continue_activate_helper(curr_node,
									  problem,
									  context,
									  run_helper);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	context.pop_back();
}

void node_continue_experiment_activate_helper(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper);
		}

		break;
	}
}

void Scope::continue_experiment_activate(Problem* problem,
										 vector<ContextLayer>& context,
										 int curr_layer,
										 RunHelper& run_helper) {
	AbstractNode* curr_node;
	if (curr_layer == (int)context.size()-1) {
		curr_node = context.back().node;
	} else {
		curr_node = context[curr_layer].node;

		ScopeNode* node = (ScopeNode*)curr_node;
		node->continue_experiment_activate(curr_node,
										   problem,
										   context,
										   curr_layer+1,
										   run_helper);
	}

	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_continue_experiment_activate_helper(
			curr_node,
			problem,
			context,
			run_helper);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	context.pop_back();
}
