#include "action_node.h"

#include "constants.h"
#include "globals.h"
#include "potential_commit.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::commit_gather_activate(AbstractNode*& curr_node,
										Problem* problem,
										int& node_count,
										AbstractNode*& potential_node_context,
										bool& potential_is_branch) {
	problem->perform_action(this->action);

	curr_node = this->next_node;

	if (solution->timestamp >= MAINTAIN_ITERS
			|| (this->parent->id == 0 || this->parent->id > solution->num_existing_scopes)) {
		uniform_int_distribution<int> select_distribution(0, node_count);
		node_count++;
		if (select_distribution(generator) == 0) {
			potential_node_context = this;
			potential_is_branch = false;
		}
	}
}

void ActionNode::commit_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 PotentialCommit* potential_commit) {
	problem->perform_action(this->action);

	curr_node = this->next_node;

	if (potential_commit->node_context == this) {
		potential_commit->activate(curr_node,
								   problem,
								   context,
								   run_helper);
	}
}
