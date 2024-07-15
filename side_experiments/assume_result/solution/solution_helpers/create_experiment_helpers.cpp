#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_action_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

void create_experiment(RunHelper& run_helper) {
	uniform_int_distribution<int> explore_node_distribution(0, run_helper.nodes_seen.size()-1);
	int explore_node_index = explore_node_distribution(generator);
	set<pair<AbstractNode*,bool>>::iterator it = next(run_helper.nodes_seen.begin(), explore_node_index);
	AbstractNode* explore_node = (*it).first;
	bool explore_is_branch = (*it).second;

	Scope* explore_scope = (Scope*)explore_node->parent;

	uniform_int_distribution<int> non_new_distribution(0, 4);
	if (explore_scope->new_action_experiment == NULL
			&& explore_node->parent->nodes.size() > 15
			&& non_new_distribution(generator) != 0) {
		NewActionExperiment* new_action_experiment = new NewActionExperiment(
			explore_node->parent,
			explore_node,
			explore_is_branch);

		if (new_action_experiment->result == EXPERIMENT_RESULT_FAIL) {
			delete new_action_experiment;
		} else {
			explore_scope->new_action_experiment = new_action_experiment;
			explore_node->experiments.push_back(new_action_experiment);
		}
	} else {
		uniform_int_distribution<int> branch_distribution(0, 1);
		if (branch_distribution(generator) == 0) {
			BranchExperiment* new_experiment = new BranchExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch);

			explore_node->experiments.push_back(new_experiment);
		} else {
			PassThroughExperiment* new_experiment = new PassThroughExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch);

			explore_node->experiments.push_back(new_experiment);
		}
	}
}
