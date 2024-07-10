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
	AbstractNode* explore_node = run_helper.selected_node.first;
	bool explore_is_branch = run_helper.selected_node.second;

	Scope* explore_scope = (Scope*)explore_node->parent;

	uniform_int_distribution<int> non_new_distribution(0, (int)explore_node->parent->nodes.size()-1);
	if (solution_set->timestamp >= solution_set->next_possible_new_scope_timestamp
			&& explore_scope->new_action_experiment == NULL
			&& explore_node->parent->nodes.size() > 10
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
		uniform_int_distribution<int> branch_distribution(0, 3);
		// if (branch_distribution(generator) == 0) {
		if (true) {
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
