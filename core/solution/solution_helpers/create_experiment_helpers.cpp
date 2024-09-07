#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope.h"
#include "new_action_experiment.h"
#include "new_info_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment.h"
#include "solution.h"

using namespace std;

void create_experiment(RunHelper& run_helper) {
	uniform_int_distribution<int> info_distribution(0, 19);
	if (run_helper.info_scope_nodes_seen.size() > 0
			&& info_distribution(generator) == 0) {
		AbstractNode* explore_node;
		uniform_int_distribution<int> even_distribution(0, 1);
		if (even_distribution(generator) == 0) {
			uniform_int_distribution<int> explore_node_distribution(0, run_helper.info_scope_nodes_seen.size()-1);
			int explore_node_index = explore_node_distribution(generator);
			map<AbstractNode*, int>::iterator it = next(run_helper.info_scope_nodes_seen.begin(), explore_node_index);
			explore_node = it->first;
		} else {
			int sum_count = 0;
			for (map<AbstractNode*, int>::iterator it = run_helper.info_scope_nodes_seen.begin();
					it != run_helper.info_scope_nodes_seen.end(); it++) {
				sum_count += it->second;
			}
			uniform_int_distribution<int> random_distribution(1, sum_count);
			int random_index = random_distribution(generator);
			for (map<AbstractNode*, int>::iterator it = run_helper.info_scope_nodes_seen.begin();
					it != run_helper.info_scope_nodes_seen.end(); it++) {
				random_index -= it->second;
				if (random_index <= 0) {
					explore_node = it->first;
					break;
				}
			}
		}

		uniform_int_distribution<int> score_type_distribution(0, 1);
		int score_type = score_type_distribution(generator);

		InfoScope* explore_scope = (InfoScope*)explore_node->parent;

		InfoPassThroughExperiment* new_experiment = new InfoPassThroughExperiment(
			explore_scope,
			explore_node,
			score_type);

		explore_scope->experiment = new_experiment;
		explore_node->experiments.push_back(new_experiment);
	} else {
		AbstractNode* explore_node;
		bool explore_is_branch;
		uniform_int_distribution<int> even_distribution(0, 1);
		if (even_distribution(generator) == 0) {
			uniform_int_distribution<int> explore_node_distribution(0, run_helper.scope_nodes_seen.size()-1);
			int explore_node_index = explore_node_distribution(generator);
			map<pair<AbstractNode*,bool>, int>::iterator it = next(run_helper.scope_nodes_seen.begin(), explore_node_index);
			explore_node = it->first.first;
			explore_is_branch = it->first.second;
		} else {
			int sum_count = 0;
			for (map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.scope_nodes_seen.begin();
					it != run_helper.scope_nodes_seen.end(); it++) {
				sum_count += it->second;
			}
			uniform_int_distribution<int> random_distribution(1, sum_count);
			int random_index = random_distribution(generator);
			for (map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.scope_nodes_seen.begin();
					it != run_helper.scope_nodes_seen.end(); it++) {
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
		 *     - may be good for certain decision heavy scopes to have lots of nodes
		 */

		int score_type;
		if (explore_node->parent->id == 0) {
			score_type = SCORE_TYPE_TRUTH;
		} else {
			uniform_int_distribution<int> score_type_distribution(0, 1);
			score_type = score_type_distribution(generator);
		}

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
				explore_is_branch,
				score_type);

			if (new_action_experiment->result == EXPERIMENT_RESULT_FAIL) {
				delete new_action_experiment;
			} else {
				explore_scope->new_action_experiment = new_action_experiment;
				explore_node->experiments.push_back(new_action_experiment);
			}
		} else {
			uniform_int_distribution<int> branch_distribution(0, 2);
			// if (branch_distribution(generator) == 0) {
			if (true) {
				// uniform_int_distribution<int> type_distribution(0, 2);
				uniform_int_distribution<int> type_distribution(0, 1);
				// switch (type_distribution(generator)) {
				switch (2) {
				case 0:
					{
						NewInfoExperiment* new_experiment = new NewInfoExperiment(
							explore_node->parent,
							explore_node,
							explore_is_branch,
							score_type,
							NULL);

						explore_node->experiments.push_back(new_experiment);
					}
					break;
				case 1:
					{
						BranchExperiment* new_experiment = new BranchExperiment(
							explore_node->parent,
							explore_node,
							explore_is_branch,
							score_type,
							NULL);

						explore_node->experiments.push_back(new_experiment);
					}
					break;
				case 2:
					{
						SeedExperiment* new_experiment = new SeedExperiment(
							explore_node->parent,
							explore_node,
							explore_is_branch,
							score_type);

						explore_node->experiments.push_back(new_experiment);
					}
					break;
				}
			} else {
				PassThroughExperiment* new_experiment = new PassThroughExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					score_type,
					NULL);

				explore_node->experiments.push_back(new_experiment);
			}
		}
	}
}
