#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void new_scope_node_activate_helper(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  scope_history);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  scope_history);
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->new_scope_activate(curr_node,
									 context,
									 run_helper,
									 scope_history);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* node = (ObsNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  scope_history);
		}
		break;
	}

	run_helper.num_actions++;
}

void Scope::new_scope_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		new_scope_node_activate_helper(
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
	}

	context.pop_back();
}

#if defined(MDEBUG) && MDEBUG
void new_scope_capture_verify_node_activate_helper(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  scope_history);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  scope_history);
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->new_scope_capture_verify_activate(
				curr_node,
				problem,
				context,
				run_helper,
				scope_history);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* node = (ObsNode*)curr_node;
			node->experiment_activate(curr_node,
									  problem,
									  context,
									  run_helper,
									  scope_history);
		}
		break;
	}

	run_helper.num_actions++;
}

void Scope::new_scope_capture_verify_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		new_scope_capture_verify_node_activate_helper(
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
	}

	context.pop_back();
}
#endif /* MDEBUG */
