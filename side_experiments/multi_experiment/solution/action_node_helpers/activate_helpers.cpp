#include "action_node.h"

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	problem->perform_action(this->action);
	run_helper.num_actions++;

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
