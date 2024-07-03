/**
 * - aggressively look to create new actions
 *   - waiting causes fracturing, making future improvement more difficult
 *   - but after creating, give time to adjust
 */

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
#include "solution_set.h"

using namespace std;

void create_experiment(RunHelper& run_helper) {
	uniform_int_distribution<int> info_distribution(0, 19);
	if (run_helper.info_scope_nodes_seen.size() > 0
			&& info_distribution(generator) == 0) {
		uniform_int_distribution<int> scope_distribution(0, run_helper.info_scope_nodes_seen.size()-1);
		map<InfoScope*, set<AbstractNode*>>::iterator scope_it =
			next(run_helper.info_scope_nodes_seen.begin(), scope_distribution(generator));
		uniform_int_distribution<int> node_distribution(0, scope_it->second.size()-1);
		set<AbstractNode*>::iterator it = next(scope_it->second.begin(), node_distribution(generator));
		AbstractNode* explore_node = *it;

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
		uniform_int_distribution<int> scope_distribution(0, run_helper.scope_nodes_seen.size()-1);
		map<Scope*, set<pair<AbstractNode*,bool>>>::iterator scope_it =
			next(run_helper.scope_nodes_seen.begin(), scope_distribution(generator));
		uniform_int_distribution<int> node_distribution(0, scope_it->second.size()-1);
		set<pair<AbstractNode*,bool>>::iterator it = next(scope_it->second.begin(), node_distribution(generator));
		AbstractNode* explore_node = it->first;
		bool explore_is_branch = it->second;

		int score_type;
		if (explore_node->parent->id == 0) {
			score_type = SCORE_TYPE_TRUTH;
		} else {
			uniform_int_distribution<int> score_type_distribution(0, 1);
			score_type = score_type_distribution(generator);
		}

		Scope* explore_scope = (Scope*)explore_node->parent;

		uniform_int_distribution<int> non_new_distribution(0, (int)explore_node->parent->nodes.size()-1);
		if (solution_set->timestamp >= solution_set->next_possible_new_scope_timestamp
				&& explore_scope->new_action_experiment == NULL
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
				uniform_int_distribution<int> type_distribution(0, 2);
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
