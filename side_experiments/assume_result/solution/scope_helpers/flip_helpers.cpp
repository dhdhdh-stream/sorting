#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "return_node.h"
#include "scope_node.h"

using namespace std;

void node_flip_gather_activate_helper(AbstractNode*& curr_node,
									  Problem* problem,
									  vector<ContextLayer>& context,
									  RunHelper& run_helper,
									  vector<int>& branch_node_indexes) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->flip_gather_activate(curr_node,
									   problem,
									   context,
									   run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->flip_gather_activate(curr_node,
									   problem,
									   context,
									   run_helper,
									   branch_node_indexes);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->flip_gather_activate(curr_node,
									   problem,
									   context,
									   run_helper,
									   branch_node_indexes);
		}

		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
			node->flip_gather_activate(curr_node,
									   problem,
									   context,
									   run_helper);
		}

		break;
	}
}

void Scope::flip_gather_activate(Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 vector<int>& branch_node_indexes) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_flip_gather_activate_helper(curr_node,
										 problem,
										 context,
										 run_helper,
										 branch_node_indexes);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	context.pop_back();
}

void node_flip_activate_helper(AbstractNode*& curr_node,
							   Problem* problem,
							   vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   int target_branch_node_index) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->flip_activate(curr_node,
								problem,
								context,
								run_helper);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->flip_activate(curr_node,
								problem,
								context,
								run_helper,
								target_branch_node_index);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->flip_activate(curr_node,
								problem,
								context,
								run_helper,
								target_branch_node_index);
		}

		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* node = (ReturnNode*)curr_node;
			node->flip_activate(curr_node,
								problem,
								context,
								run_helper);
		}

		break;
	}
}

void Scope::flip_activate(Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  int target_branch_node_index) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_flip_activate_helper(curr_node,
								  problem,
								  context,
								  run_helper,
								  target_branch_node_index);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	context.pop_back();
}
