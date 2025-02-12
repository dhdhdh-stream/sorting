#include "action_node.h"

#include "problem.h"

using namespace std;

void ActionNode::measure_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	problem->perform_action(this->action);
	run_helper.num_actions++;

	curr_node = this->next_node;
}
