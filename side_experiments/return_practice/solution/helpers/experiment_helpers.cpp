#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment.h"
#include "experiment_run.h"
#include "force_experiment.h"
#include "globals.h"
#include "solution.h"
#include "start_node.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPERIMENT_MIN_NUM_STATES = 10;
#else
const int EXPERIMENT_MIN_NUM_STATES = 100;
#endif /* MDEBUG */

const int FORCE_EXPERIMENT_ITER = 10;

void count_eval_helper(ExperimentRun* run,
					   int& node_count,
					   int& eval_count) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)node;
				if (start_node->state_history.size() >= EXPERIMENT_MIN_NUM_STATES) {
					node_count++;
					if (start_node->experiment != NULL) {
						eval_count++;
					}
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->state_history.size() >= EXPERIMENT_MIN_NUM_STATES) {
					node_count++;
					if (action_node->experiment != NULL) {
						eval_count++;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_state_history.size() >= EXPERIMENT_MIN_NUM_STATES) {
						node_count++;
						if (branch_node->branch_experiment != NULL) {
							eval_count++;
						}
					}
				} else {
					if (branch_node->original_state_history.size() >= EXPERIMENT_MIN_NUM_STATES) {
						node_count++;
						if (branch_node->original_experiment != NULL) {
							eval_count++;
						}
					}
				}
			}
			break;
		}
	}
}

void gather_start_helper(ExperimentRun* run,
						 int& node_count,
						 AbstractNode*& explore_node,
						 bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)node;
				if (start_node->state_history.size() >= EXPERIMENT_MIN_NUM_STATES
						&& start_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->state_history.size() >= EXPERIMENT_MIN_NUM_STATES
						&& action_node->experiment == NULL) {
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
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_state_history.size() >= EXPERIMENT_MIN_NUM_STATES
							&& branch_node->branch_experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = branch_node;
							explore_is_branch = true;
						}
					}
				} else {
					if (branch_node->original_state_history.size() >= EXPERIMENT_MIN_NUM_STATES
							&& branch_node->original_experiment == NULL) {
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
		}
	}
}

void create_experiment(ExperimentRun* run,
					   Wrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch;
	gather_start_helper(run,
						node_count,
						explore_node,
						explore_is_branch);

	if (explore_node != NULL) {
		wrapper->experiment_iter++;
		if (wrapper->experiment_iter % FORCE_EXPERIMENT_ITER == 0) {
			init_force_experiment_helper(explore_node,
										 explore_is_branch,
										 wrapper);
		} else {
			init_experiment_helper(explore_node,
								   explore_is_branch,
								   wrapper);
		}
	}
}
