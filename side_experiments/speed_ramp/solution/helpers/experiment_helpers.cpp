#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void create_experiment(SolutionWrapper* wrapper) {
	// vector<AbstractNode*> possible_exits;

	// AbstractNode* starting_node;
	// switch (this->node_context->type) {
	// case NODE_TYPE_START:
	// 	{
	// 		StartNode* start_node = (StartNode*)this->node_context;
	// 		starting_node = start_node->next_node;
	// 	}
	// 	break;
	// case NODE_TYPE_ACTION:
	// 	{
	// 		ActionNode* action_node = (ActionNode*)this->node_context;
	// 		starting_node = action_node->next_node;
	// 	}
	// 	break;
	// case NODE_TYPE_SCOPE:
	// 	{
	// 		ScopeNode* scope_node = (ScopeNode*)this->node_context;
	// 		starting_node = scope_node->next_node;
	// 	}
	// 	break;
	// case NODE_TYPE_BRANCH:
	// 	{
	// 		BranchNode* branch_node = (BranchNode*)this->node_context;
	// 		if (this->is_branch) {
	// 			starting_node = branch_node->branch_next_node;
	// 		} else {
	// 			starting_node = branch_node->original_next_node;
	// 		}
	// 	}
	// 	break;
	// case NODE_TYPE_OBS:
	// 	{
	// 		ObsNode* obs_node = (ObsNode*)this->node_context;
	// 		starting_node = obs_node->next_node;
	// 	}
	// 	break;
	// }

	// this->scope_context->random_exit_activate(
	// 	starting_node,
	// 	possible_exits);

	// int random_index;
	// geometric_distribution<int> exit_distribution(0.2);
	// while (true) {
	// 	random_index = exit_distribution(generator);
	// 	if (random_index < (int)possible_exits.size()) {
	// 		break;
	// 	}
	// }
	// this->curr_exit_next_node = possible_exits[random_index];
}
