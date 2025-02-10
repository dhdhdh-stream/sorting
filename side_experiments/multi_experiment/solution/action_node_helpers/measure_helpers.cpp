#include "action_node.h"

#include "problem.h"

using namespace std;

void ActionNode::measure_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	double score = problem->perform_action(this->action);
	run_helper.sum_score += score;
	if (score < 0.0) {
		run_helper.early_exit = true;
	}

	curr_node = this->next_node;
}
