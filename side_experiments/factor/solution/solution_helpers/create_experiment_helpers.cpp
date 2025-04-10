#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "commit_experiment.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void gather_nodes_seen_helper(ScopeHistory* scope_history,
							  map<pair<AbstractNode*,bool>, int>& nodes_seen) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->experiment == NULL) {
			switch (h_it->second->node->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_OBS:
				{
					map<pair<AbstractNode*,bool>, int>::iterator seen_it = nodes_seen
						.find({h_it->second->node, false});
					if (seen_it == nodes_seen.end()) {
						nodes_seen[{h_it->second->node, false}] = 1;
					} else {
						seen_it->second++;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

					gather_nodes_seen_helper(scope_node_history->scope_history,
											 nodes_seen);

					map<pair<AbstractNode*,bool>, int>::iterator seen_it = nodes_seen
						.find({h_it->second->node, false});
					if (seen_it == nodes_seen.end()) {
						nodes_seen[{h_it->second->node, false}] = 1;
					} else {
						seen_it->second++;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
					map<pair<AbstractNode*,bool>, int>::iterator seen_it = nodes_seen
						.find({h_it->second->node, branch_node_history->is_branch});
					if (seen_it == nodes_seen.end()) {
						nodes_seen[{h_it->second->node, branch_node_history->is_branch}] = 1;
					} else {
						seen_it->second++;
					}
				}
				break;
			}
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   int improvement_iter) {
	map<pair<AbstractNode*,bool>,int> nodes_seen;
	gather_nodes_seen_helper(scope_history,
							 nodes_seen);

	if (nodes_seen.size() > 0) {
		AbstractNode* explore_node;
		bool explore_is_branch;
		uniform_int_distribution<int> even_distribution(0, 1);
		if (even_distribution(generator) == 0) {
			uniform_int_distribution<int> explore_node_distribution(0, nodes_seen.size()-1);
			int explore_node_index = explore_node_distribution(generator);
			map<pair<AbstractNode*,bool>, int>::iterator it = next(nodes_seen.begin(), explore_node_index);
			explore_node = it->first.first;
			explore_is_branch = it->first.second;
		} else {
			int sum_count = 0;
			for (map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.begin();
					it != nodes_seen.end(); it++) {
				sum_count += it->second;
			}
			uniform_int_distribution<int> random_distribution(1, sum_count);
			int random_index = random_distribution(generator);
			for (map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.begin();
					it != nodes_seen.end(); it++) {
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

		if (explore_node->parent->exceeded) {
			uniform_int_distribution<int> non_new_distribution(0, 19);
			if (explore_node->parent->new_scope_experiment == NULL
					&& non_new_distribution(generator) != 0) {
				NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);

				if (new_scope_experiment->result == EXPERIMENT_RESULT_FAIL) {
					delete new_scope_experiment;
				} else {
					explore_node->parent->new_scope_experiment = new_scope_experiment;
					explore_node->experiment = new_scope_experiment;
				}
			} else {
				if (improvement_iter > 3) {
					PassThroughExperiment* new_experiment = new PassThroughExperiment(
						explore_node->parent,
						explore_node,
						explore_is_branch);

					if (new_experiment->result == EXPERIMENT_RESULT_FAIL) {
						delete new_experiment;
					} else {
						explore_node->experiment = new_experiment;
					}
				}
			}
		} else {
			/**
			 * - weigh towards PassThroughExperiments as cheaper and potentially just as effective
			 *   - solutions are often made of relatively few distinct decisions, but applied such that has good coverage
			 *     - like tessellation, but have to get both the shape and the pattern correct
			 *       - and PassThroughExperiments help with both
			 */
			// if (improvement_iter == 0) {
			if (false) {
				CommitExperiment* new_commit_experiment = new CommitExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);
				explore_node->experiment = new_commit_experiment;
			} else {
				uniform_int_distribution<int> pass_through_distribution(0, 1);
				if (pass_through_distribution(generator) == 0
						&& improvement_iter > 3) {
					PassThroughExperiment* new_experiment = new PassThroughExperiment(
						explore_node->parent,
						explore_node,
						explore_is_branch);

					if (new_experiment->result == EXPERIMENT_RESULT_FAIL) {
						delete new_experiment;
					} else {
						explore_node->experiment = new_experiment;
					}
				} else {
					BranchExperiment* new_experiment = new BranchExperiment(
						explore_node->parent,
						explore_node,
						explore_is_branch);

					explore_node->experiment = new_experiment;
				}
			}
		}
	}
}
