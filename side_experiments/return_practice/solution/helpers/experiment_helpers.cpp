#include "solution_helpers.h"

#include "branch_node.h"
#include "constants.h"
#include "experiment.h"
#include "experiment_run.h"
#include "globals.h"
#include "obs_node.h"

using namespace std;

void count_eval_helper(ExperimentRun* run,
					   int& node_count,
					   int& eval_count) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->state_history.size() >= EXPERIMENT_MIN_NUM_STATE_HISTORY) {
					node_count++;
					if (obs_node->experiment != NULL) {
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
					if (branch_node->branch_state_history.size() >= EXPERIMENT_MIN_NUM_STATE_HISTORY) {
						node_count++;
						if (branch_node->branch_experiment != NULL) {
							eval_count++;
						}
					}
				} else {
					if (branch_node->original_state_history.size() >= EXPERIMENT_MIN_NUM_STATE_HISTORY) {
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

void gather_helper(ExperimentRun* run,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->state_history.size() >= EXPERIMENT_MIN_NUM_STATE_HISTORY) {
					if (obs_node->experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = obs_node;
							explore_is_branch = false;
						}
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_state_history.size() >= EXPERIMENT_MIN_NUM_STATE_HISTORY) {
						if (branch_node->branch_experiment == NULL) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = true;
							}
						}
					}
				} else {
					if (branch_node->original_state_history.size() >= EXPERIMENT_MIN_NUM_STATE_HISTORY) {
						if (branch_node->original_experiment == NULL) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = false;
							}
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
	gather_helper(run,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		init_experiment_helper(explore_node,
							   explore_is_branch,
							   wrapper);
	}
}
