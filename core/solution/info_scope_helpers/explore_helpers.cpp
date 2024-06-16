#include "info_scope.h"

#include "action_node.h"
#include "globals.h"
#include "info_scope_node.h"
#include "solution.h"

using namespace std;

void node_explore_activate_helper(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  AbstractScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* node = (InfoScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	}
}

void InfoScope::explore_activate(Problem* problem,
								 RunHelper& run_helper,
								 AbstractScopeHistory*& history) {
	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this;
	inner_context.back().node = NULL;

	history = new InfoScopeHistory(this);
	inner_context.back().scope_history = history;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		run_helper.num_actions++;
		if (run_helper.num_actions > solution->num_actions_limit) {
			run_helper.exceeded_limit = true;
			break;
		}

		node_explore_activate_helper(curr_node,
									 problem,
									 inner_context,
									 run_helper,
									 history);
	}
}
