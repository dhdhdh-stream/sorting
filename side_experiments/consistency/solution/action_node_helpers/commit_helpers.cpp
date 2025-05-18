#include "action_node.h"

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ActionNode::commit_activate(Problem* problem,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history) {
	problem->perform_action(this->action);
}
