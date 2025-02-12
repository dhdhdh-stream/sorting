#include "action_node.h"

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ActionNode::commit_activate(Problem* problem,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	problem->perform_action(this->action);
	run_helper.num_actions++;
}
