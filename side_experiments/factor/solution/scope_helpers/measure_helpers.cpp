#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope_node.h"

using namespace std;

void node_measure_activate_helper(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->measure_activate(curr_node,
								   problem,
								   context,
								   run_helper,
								   history);
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->activate(curr_node,
						   context,
						   run_helper,
						   history);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* node = (ObsNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history);
		}
		break;
	}

	run_helper.num_actions++;
}

void Scope::measure_activate(Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* history) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		curr_node->average_instances_per_run += 1.0;

		node_measure_activate_helper(curr_node,
									 problem,
									 context,
									 run_helper,
									 history);
	}

	context.pop_back();
}
