#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "candidate_experiment.h"
#include "chase_experiment.h"
#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(bool in_tunnel,
				   ScopeHistory* scope_history,
				   SolutionWrapper* wrapper,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_ACTION:
		case NODE_TYPE_OBS:
			if (in_tunnel
					&& node->experiment == NULL) {
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					explore_node = node;
					explore_is_branch = false;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				if (scope_history->scope == wrapper->curr_tunnel_parent) {
					gather_helper(true,
								  scope_node_history->scope_history,
								  wrapper,
								  node_count,
								  explore_node,
								  explore_is_branch);
				} else {
					gather_helper(in_tunnel,
								  scope_node_history->scope_history,
								  wrapper,
								  node_count,
								  explore_node,
								  explore_is_branch);
				}

				if (in_tunnel
						&& node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			if (in_tunnel
					&& node->experiment == NULL) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = true;
					}
				} else {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	bool in_tunnel;
	if (wrapper->curr_tunnel_parent == NULL) {
		in_tunnel = true;
	} else {
		in_tunnel = false;
	}
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(in_tunnel,
				  scope_history,
				  wrapper,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		if (wrapper->tunnel_iter == 0) {
			Experiment* new_experiment = new Experiment(
				explore_node->parent,
				explore_node,
				explore_is_branch);
			wrapper->curr_experiment = new_experiment;
		} else {
			if (wrapper->curr_tunnel_parent == NULL) {
				CandidateExperiment* new_experiment = new CandidateExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);
				wrapper->curr_experiment = new_experiment;
			} else {
				ChaseExperiment* new_experiment = new ChaseExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);
				wrapper->curr_experiment = new_experiment;
			}
		}
	}
}
