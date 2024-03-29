#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "globals.h"
#include "problem.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_step_through_activate_helper(AbstractNode*& curr_node,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   int& exit_depth,
									   AbstractNode*& exit_node,
									   RunHelper& run_helper,
									   ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			ActionNodeHistory* node_history = new ActionNodeHistory(node);
			history->node_histories.push_back(node_history);
			node->step_through_activate(curr_node,
										problem,
										context,
										run_helper,
										node_history);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			ScopeNodeHistory* node_history = new ScopeNodeHistory(node);
			history->node_histories.push_back(node_history);
			node->step_through_activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper,
										node_history);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->step_through_activate(curr_node,
										problem,
										context,
										run_helper,
										history->node_histories);
		}

		break;
	case NODE_TYPE_EXIT:
		{
			ExitNode* node = (ExitNode*)curr_node;
			if (node->throw_id != -1) {
				run_helper.throw_id = node->throw_id;
			} else {
				exit_depth = node->exit_depth-1;
				exit_node = node->next_node;
			}
		}

		break;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
	}
}

void Scope::step_through_activate(AbstractNode* starting_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  int& exit_depth,
								  AbstractNode*& exit_node,
								  RunHelper& run_helper,
								  ScopeHistory* history) {
	if (run_helper.curr_depth > solution->depth_limit) {
		string input_gate;
		cin >> input_gate;

		cout << "context:" << endl;
		for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			cout << c_index << ": " << context[c_index].scope->id << " " << context[c_index].node->id << endl;
		}
		cout << context.size()-1 << ": " << this->id << " -1" << endl;
		problem->print();
		cout << "exceeded_limit" << endl;

		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	AbstractNode* curr_node = starting_node;
	while (true) {
		if (run_helper.exceeded_limit
				|| run_helper.throw_id != -1
				|| exit_depth != -1
				|| curr_node == NULL) {
			break;
		}

		if (context.size() > 1) {
			ScopeNode* scope_node = (ScopeNode*)context[context.size()-2].node;
			if (scope_node->exit_nodes.find(curr_node) != scope_node->exit_nodes.end()) {
				break;
			}
		}

		node_step_through_activate_helper(curr_node,
										  problem,
										  context,
										  exit_depth,
										  exit_node,
										  run_helper,
										  history);
	}

	run_helper.curr_depth--;
}