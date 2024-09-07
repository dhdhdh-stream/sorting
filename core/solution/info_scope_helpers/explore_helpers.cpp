#include "info_scope.h"

#include "action_node.h"
#include "globals.h"
#include "solution.h"

using namespace std;

void InfoScope::explore_activate(Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 AbstractScopeHistory*& history) {
	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	history = new InfoScopeHistory(this);
	context.back().scope_history = history;

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

		ActionNode* node = (ActionNode*)curr_node;
		node->activate(curr_node,
					   problem,
					   context,
					   run_helper,
					   history->node_histories);
	}

	context.pop_back();
}
