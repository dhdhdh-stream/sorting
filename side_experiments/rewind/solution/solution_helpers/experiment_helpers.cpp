#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "repetition_experiment.h"
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
	if (scope_history->scope->is_outer) {
		return;
	}

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
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				gather_helper(scope_node_history->scope_history,
							  node_count,
							  explore_node,
							  explore_is_branch);

				if (node->experiment == NULL) {
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
			if (node->experiment == NULL) {
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

/**
 * - simply don't end on BranchNode
 */
void repetition_gather_helper(ScopeHistory* scope_history,
							  int& scope_count,
							  AbstractNode*& start_node,
							  AbstractNode*& end_node) {
	vector<AbstractNode*> nodes(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		nodes[h_it->second->index] = h_it->second->node;
	}

	for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
		switch (nodes[n_index]->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_BRANCH:
		case NODE_TYPE_OBS:
			nodes.erase(nodes.begin() + n_index);
			break;
		}
	}

	if (nodes.size() >= NEW_SCOPE_MIN_NUM_NODES) {
		uniform_int_distribution<int> select_distribution(0, scope_count);
		if (select_distribution(generator) == 0) {
			uniform_int_distribution<int> node_distribution(0, nodes.size()-1);
			while (true) {
				int start_index = node_distribution(generator);
				int end_index = node_distribution(generator);
				if (start_index > end_index) {
					int temp = start_index;
					start_index = end_index;
					end_index = temp;
				}
				if (end_index - start_index >= NEW_SCOPE_MIN_NUM_NODES-1) {
					start_node = nodes[start_index];
					end_node = nodes[end_index];
					break;
				}
			}
		}
		scope_count++;
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			repetition_gather_helper(scope_node_history->scope_history,
									 scope_count,
									 start_node,
									 end_node);
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	uniform_int_distribution<int> repetition_distribution(0, 1);
	if (repetition_distribution(generator)) {
		int scope_count = 0;
		AbstractNode* start_node;
		AbstractNode* end_node;
		repetition_gather_helper(scope_history,
								 scope_count,
								 start_node,
								 end_node);

		if (scope_count != 0) {
			Scope* new_scope;
			create_new_scope(start_node,
							 end_node,
							 new_scope);

			RepetitionExperiment* experiment = new RepetitionExperiment(
				end_node,
				new_scope);
			wrapper->curr_experiment = experiment;

			return;
		}
	}

	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		vector<AbstractNode*> possible_exits;

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
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)explore_node;
				starting_node = scope_node->next_node;
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
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)explore_node;
				starting_node = obs_node->next_node;
			}
			break;
		}

		explore_node->parent->random_exit_activate(
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

		Experiment* new_experiment = new Experiment(
			explore_node->parent,
			explore_node,
			explore_is_branch,
			exit_next_node);
		wrapper->curr_experiment = new_experiment;

		wrapper->solution->num_experiments++;
	}
}
