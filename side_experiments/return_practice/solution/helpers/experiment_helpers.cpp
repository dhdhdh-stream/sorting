#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "crazy.h"
#include "experiment.h"
#include "experiment_run.h"
#include "globals.h"
#include "solution.h"
#include "start_node.h"
#include "wrapper.h"

using namespace std;

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
				node_count++;
				if (start_node->experiment != NULL) {
					eval_count++;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				node_count++;
				if (action_node->experiment != NULL) {
					eval_count++;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					node_count++;
					if (branch_node->branch_experiment != NULL) {
						eval_count++;
					}
				} else {
					node_count++;
					if (branch_node->original_experiment != NULL) {
						eval_count++;
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
				if (start_node->experiment == NULL) {
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
				if (action_node->experiment == NULL) {
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
					if (branch_node->branch_experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = branch_node;
							explore_is_branch = true;
						}
					}
				} else {
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
		AbstractNode* starting_node;
		switch (explore_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)explore_node;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)explore_node;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)explore_node;
				if (explore_is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		}
		vector<AbstractNode*> possible_exits;
		wrapper->solution->random_exit_activate(
			starting_node,
			possible_exits);
		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		AbstractNode* exit_next_node = possible_exits[random_index];

		Experiment* new_experiment = new Experiment(explore_node,
													explore_is_branch,
													exit_next_node,
													wrapper);

		// temp
		cout << "new_experiment" << endl;

		switch (explore_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)explore_node;
				start_node->experiment = new_experiment;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)explore_node;
				action_node->experiment = new_experiment;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)explore_node;
				if (explore_is_branch) {
					branch_node->branch_experiment = new_experiment;
				} else {
					branch_node->original_experiment = new_experiment;
				}
			}
			break;
		}
	}
}

void create_crazy(ExperimentRun* run,
				  Wrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch;
	gather_start_helper(run,
						node_count,
						explore_node,
						explore_is_branch);

	if (explore_node != NULL) {
		AbstractNode* starting_node;
		switch (explore_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)explore_node;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)explore_node;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)explore_node;
				if (explore_is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		}
		vector<AbstractNode*> possible_exits;
		wrapper->solution->random_exit_activate(
			starting_node,
			possible_exits);
		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		AbstractNode* exit_next_node = possible_exits[random_index];

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		vector<int> actions;
		uniform_int_distribution<int> action_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			actions.push_back(action_distribution(generator));
		}

		Crazy* crazy = new Crazy();

		crazy->node_context = explore_node;
		crazy->is_branch = explore_is_branch;

		crazy->actions = actions;
		crazy->exit_next_node = exit_next_node;

		switch (explore_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)explore_node;
				start_node->experiment = crazy;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)explore_node;
				action_node->experiment = crazy;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)explore_node;
				if (explore_is_branch) {
					branch_node->branch_experiment = crazy;
				} else {
					branch_node->original_experiment = crazy;
				}
			}
			break;
		}

		wrapper->crazy = crazy;
		wrapper->hit_crazy = false;
	}
}
