#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
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

const int FORCE_EXPERIMENT_ITER = 4;

void count_eval_helper(ExperimentRun* run,
					   int& node_count,
					   int& eval_count) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		if (node->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)node;
			if (branch_node->ramp < RAMP_NUM_GEARS) {
				eval_count++;
			}
		}

		node_count++;
	}
}

void gather_start_helper(ExperimentRun* run,
						 int& node_count,
						 AbstractNode*& explore_node,
						 bool& explore_is_branch,
						 vector<AbstractNode*>& explore_node_histories,
						 int& explore_index) {
	vector<AbstractNode*> curr_node_histories(run->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		curr_node_histories[h_it->second->index] = h_it->second->node;
	}

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
						explore_node_histories = curr_node_histories;
						explore_index = h_it->second->index;
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
						explore_node_histories = curr_node_histories;
						explore_index = h_it->second->index;
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
							explore_node_histories = curr_node_histories;
							explore_index = h_it->second->index;
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
							explore_node_histories = curr_node_histories;
							explore_index = h_it->second->index;
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
	vector<AbstractNode*> explore_node_histories;
	int explore_index;
	gather_start_helper(run,
						node_count,
						explore_node,
						explore_is_branch,
						explore_node_histories,
						explore_index);

	if (explore_node != NULL) {
		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = explore_index + 1 + exit_distribution(generator);
			if (random_index < (int)explore_node_histories.size()) {
				break;
			}
		}
		AbstractNode* exit_next_node = explore_node_histories[random_index];

		init_experiment_helper(explore_node,
							   explore_is_branch,
							   exit_next_node,
							   wrapper);
	}
}

void create_force_experiment(ExperimentRun* run,
							 Wrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch;
	vector<AbstractNode*> explore_node_histories;
	int explore_index;
	gather_start_helper(run,
						node_count,
						explore_node,
						explore_is_branch,
						explore_node_histories,
						explore_index);

	if (explore_node != NULL) {
		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = explore_index + 1 + exit_distribution(generator);
			if (random_index < (int)explore_node_histories.size()) {
				break;
			}
		}
		AbstractNode* exit_next_node = explore_node_histories[random_index];

		init_force_experiment_helper(explore_node,
									 explore_is_branch,
									 exit_next_node,
									 wrapper);
	}
}
