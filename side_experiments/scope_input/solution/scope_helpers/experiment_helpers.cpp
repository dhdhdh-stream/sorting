#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "problem.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

void Scope::experiment_activate(Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* history) {
	if (this->new_scope_experiment != NULL) {
		this->new_scope_experiment->pre_activate(run_helper);
	}

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		switch (curr_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)curr_node;
				node->experiment_activate(curr_node,
										  problem,
										  run_helper,
										  history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)curr_node;
				node->experiment_activate(curr_node,
										  problem,
										  run_helper,
										  history);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)curr_node;
				node->experiment_activate(curr_node,
										  problem,
										  run_helper,
										  history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)curr_node;
				node->experiment_activate(curr_node,
										  problem,
										  run_helper,
										  history);
			}
			break;
		}

		run_helper.num_actions++;
	}

	if (this->new_scope_experiment != NULL) {
		if (run_helper.experiment_history != NULL
				&& run_helper.experiment_history->experiment == this->new_scope_experiment) {
			this->new_scope_experiment->back_activate(
				run_helper,
				history);
		}
	}
}
