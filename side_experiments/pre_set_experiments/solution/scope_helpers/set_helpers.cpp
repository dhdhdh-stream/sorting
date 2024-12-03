#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void node_set_activate_helper(AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  RunHelper& run_helper,
							  bool& experiment_seen,
							  map<pair<AbstractNode*,bool>, int>& nodes_seen) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->set_activate(curr_node,
							   problem,
							   context,
							   run_helper,
							   experiment_seen,
							   nodes_seen);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->set_activate(curr_node,
							   problem,
							   context,
							   run_helper,
							   experiment_seen,
							   nodes_seen);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->set_activate(curr_node,
							   problem,
							   context,
							   run_helper,
							   experiment_seen,
							   nodes_seen);
		}

		break;
	}

	run_helper.num_actions++;
}

void Scope::set_activate(Problem* problem,
						 vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 bool& experiment_seen,
						 map<pair<AbstractNode*,bool>, int>& nodes_seen) {
	context.push_back(ContextLayer());

	context.back().scope = this;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_set_activate_helper(curr_node,
								 problem,
								 context,
								 run_helper,
								 experiment_seen,
								 nodes_seen);
	}

	context.pop_back();
}
