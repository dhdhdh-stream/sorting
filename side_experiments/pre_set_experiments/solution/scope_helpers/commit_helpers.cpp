#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "potential_commit.h"
#include "scope_node.h"

using namespace std;

void node_commit_gather_activate_helper(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										int& node_count,
										AbstractNode*& potential_node_context,
										bool& potential_is_branch) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->commit_gather_activate(curr_node,
										 problem,
										 context,
										 run_helper,
										 node_count,
										 potential_node_context,
										 potential_is_branch);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->commit_gather_activate(curr_node,
										 problem,
										 context,
										 run_helper,
										 node_count,
										 potential_node_context,
										 potential_is_branch);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->commit_gather_activate(curr_node,
										 problem,
										 context,
										 run_helper,
										 node_count,
										 potential_node_context,
										 potential_is_branch);
		}

		break;
	}

	run_helper.num_actions++;
}

void Scope::commit_gather_activate(Problem* problem,
								   vector<ContextLayer>& context,
								   RunHelper& run_helper,
								   int& node_count,
								   AbstractNode*& potential_node_context,
								   bool& potential_is_branch) {
	context.push_back(ContextLayer());

	context.back().scope_id = this->id;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_commit_gather_activate_helper(curr_node,
										   problem,
										   context,
										   run_helper,
										   node_count,
										   potential_node_context,
										   potential_is_branch);
	}

	context.pop_back();
}

void node_commit_activate_helper(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 PotentialCommit* potential_commit) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->commit_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  potential_commit);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->commit_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  potential_commit);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->commit_activate(curr_node,
								  problem,
								  context,
								  run_helper,
								  potential_commit);
		}

		break;
	}

	run_helper.num_actions++;
}

void Scope::commit_activate(Problem* problem,
							vector<ContextLayer>& context,
							RunHelper& run_helper,
							PotentialCommit* potential_commit) {
	context.push_back(ContextLayer());

	context.back().scope_id = this->id;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_commit_activate_helper(curr_node,
									problem,
									context,
									run_helper,
									potential_commit);
	}

	context.pop_back();
}
