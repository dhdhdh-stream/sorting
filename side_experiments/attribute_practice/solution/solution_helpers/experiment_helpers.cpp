#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
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
void gather_helper(ScopeHistory* scope_history,
				   int curr_depth,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch,
				   int& explore_depth) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_ACTION:
		case NODE_TYPE_OBS:
			if (node->experiment == NULL) {
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					explore_node = node;
					explore_is_branch = false;
					explore_depth = curr_depth;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				gather_helper(scope_node_history->scope_history,
							  curr_depth + 1,
							  node_count,
							  explore_node,
							  explore_is_branch,
							  explore_depth);

				if (node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
						explore_depth = curr_depth;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			if (node->experiment == NULL) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = true;
						explore_depth = curr_depth;
					}
				} else {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
						explore_depth = curr_depth;
					}
				}
			}
			break;
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	int explore_depth = -1;
	gather_helper(scope_history,
				  0,
				  node_count,
				  explore_node,
				  explore_is_branch,
				  explore_depth);

	if (explore_node != NULL) {
		geometric_distribution<int> depth_distribution(0.3);
		int signal_depth;
		while (true) {
			signal_depth = depth_distribution(generator);
			if (signal_depth == explore_depth + 1) {
				signal_depth = -1;
				break;
			} else if (signal_depth <= explore_depth) {
				break;
			}
		}

		bool average_signals;
		if (signal_depth == -1) {
			average_signals = false;
		} else {
			uniform_int_distribution<int> average_signals_distribution(0, 1);
			average_signals = average_signals_distribution(generator);
		}

		Experiment* new_experiment = new Experiment(
			explore_node->parent,
			explore_node,
			explore_is_branch,
			signal_depth,
			average_signals);
		wrapper->curr_experiment = new_experiment;

		// temp
		cout << "Experiment" << endl;
	}
}
