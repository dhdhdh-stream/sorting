#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
			if (node->experiment == NULL) {
				StartNode* start_node = (StartNode*)node;
				if (start_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = start_node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_ACTION:
			if (node->experiment == NULL) {
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = action_node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				gather_helper(scope_node_history->scope_history,
							  node_count,
							  explore_node,
							  explore_is_branch);

				if (node->experiment == NULL
						&& scope_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = scope_node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			if (node->experiment == NULL) {
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = branch_node;
							explore_is_branch = true;
						}
					}
				} else {
					if (branch_node->original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = branch_node;
							explore_is_branch = false;
						}
					}
				}
			}
			break;
		case NODE_TYPE_OBS:
			if (node->experiment == NULL) {
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = obs_node;
						explore_is_branch = false;
					}
				}
			}
			break;
		}
	}
}

void create_branch_experiment(ScopeHistory* scope_history,
							  SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	// if (explore_node != NULL) {
	if (explore_node != NULL
			&& explore_node->type == NODE_TYPE_OBS) {
		BranchExperiment* new_experiment = new BranchExperiment(
			explore_node->parent,
			explore_node,
			explore_is_branch,
			wrapper);
		wrapper->curr_branch_experiment = new_experiment;
	}
}
