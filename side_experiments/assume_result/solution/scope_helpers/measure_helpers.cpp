#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "return_node.h"
#include "scope_node.h"

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  vector<int>& scope_counts) {
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
			node->measure_activate(curr_node,
								   problem,
								   context,
								   run_helper,
								   scope_counts);
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

void Scope::measure_activate(Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 vector<int>& scope_counts) {
	scope_counts[this->id]++;

	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 run_helper,
							 scope_counts);

		if (run_helper.exceeded_limit) {
			break;
		}
	}

	for (int b_index = 0; b_index < (int)context.back().branch_nodes_seen.size(); b_index++) {
		run_helper.branch_node_ancestors.erase(context.back().branch_nodes_seen[b_index]);
	}
	context.back().branch_nodes_seen.clear();

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
								 run_helper,
								 scope_counts);

			if (run_helper.exceeded_limit) {
				break;
			}
		}

		for (int b_index = 0; b_index < (int)context.back().branch_nodes_seen.size(); b_index++) {
			run_helper.branch_node_ancestors.erase(context.back().branch_nodes_seen[b_index]);
		}
	}

	context.pop_back();
}
