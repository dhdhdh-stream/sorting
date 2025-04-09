#include "action_node.h"

#include "problem.h"
#include "scope.h"

using namespace std;

void ActionNode::new_activate(Problem* problem) {
	problem->perform_action(this->action);
}
