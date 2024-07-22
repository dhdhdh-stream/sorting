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
	if (run_helper.nodes_seen.size() > 0) {
		AbstractNode* explore_node;
		bool explore_is_branch;
		uniform_int_distribution<int> even_distribution(0, 1);
		if (even_distribution(generator) == 0) {
			uniform_int_distribution<int> explore_node_distribution(0, run_helper.nodes_seen.size()-1);
			int explore_node_index = explore_node_distribution(generator);
			map<pair<AbstractNode*,bool>, int>::iterator it = next(run_helper.nodes_seen.begin(), explore_node_index);
			explore_node = it->first.first;
			explore_is_branch = it->first.second;
		} else {
			int sum_count = 0;
			for (map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.begin();
					it != run_helper.nodes_seen.end(); it++) {
				sum_count += it->second;
			}
			uniform_int_distribution<int> random_distribution(1, sum_count);
			int random_index = random_distribution(generator);
			for (map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.begin();
					it != run_helper.nodes_seen.end(); it++) {
				random_index -= it->second;
				if (random_index <= 0) {
					explore_node = it->first.first;
					explore_is_branch = it->first.second;
					break;
				}
			}
		}
		/**
		 * - don't weigh based on number of nodes within scope
		 *   - can get trapped by small useless scopes
		 *   - may be good for certain decision heavy scopes to have lots of nodes
		 */

		Scope* explore_scope = (Scope*)explore_node->parent;

		/**
		 * - don't focus on generalization/reuse
		 *   - may block progress if incompatible spots are grouped together
		 *   - (though may also greatly speed up future progress of course)
		 */
		uniform_int_distribution<int> non_new_distribution(0, 4);
		if (explore_scope->new_action_experiment == NULL
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
			/**
			 * - weigh towards PassThroughExperiments as cheaper and potentially just as effective
			 *   - solutions are often made of relatively few distinct decisions, but applied such that has good coverage
			 *     - like tessellation, but have to get both the shape and the pattern correct
			 *       - and PassThroughExperiments help with both
			 */
			uniform_int_distribution<int> pass_through_distribution(0, 1);
			if (pass_through_distribution(generator) == 0) {
				PassThroughExperiment* new_experiment = new PassThroughExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);

				explore_node->experiments.push_back(new_experiment);
			} else {
				BranchExperiment* new_experiment = new BranchExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);

				explore_node->experiments.push_back(new_experiment);
			}
		}
	}
}
