#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch,
				   vector<AbstractNode*>& explore_node_histories,
				   int& explore_index,
				   SolutionWrapper* wrapper) {
	bool match_scope = false;
	if (wrapper->solution->cycle_index == -1
			&& scope_history->scope == wrapper->solution->starting_scope) {
		match_scope = true;
	} else if (scope_history->scope->id == wrapper->solution->iter_index) {
		match_scope = true;
	}

	if (match_scope) {
		vector<AbstractNode*> curr_node_histories(scope_history->node_histories.size());
		for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
				h_it != scope_history->node_histories.end(); h_it++) {
			curr_node_histories[h_it->second->index] = h_it->second->node;
		}
		curr_node_histories.push_back(NULL);

		for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
				h_it != scope_history->node_histories.end(); h_it++) {
			AbstractNode* node = h_it->second->node;
			switch (node->type) {
			case NODE_TYPE_NOOP:
			case NODE_TYPE_ACTION:
				{
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
						explore_node_histories = curr_node_histories;
						explore_index = h_it->second->index;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

					gather_helper(scope_node_history->scope_history,
								  node_count,
								  explore_node,
								  explore_is_branch,
								  explore_node_histories,
								  explore_index,
								  wrapper);

					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
						explore_node_histories = curr_node_histories;
						explore_index = h_it->second->index;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
					if (branch_node_history->is_branch) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = node;
							explore_is_branch = true;
							explore_node_histories = curr_node_histories;
							explore_index = h_it->second->index;
						}
					} else {
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
			}
		}
	} else {
		for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
				h_it != scope_history->node_histories.end(); h_it++) {
			AbstractNode* node = h_it->second->node;
			switch (node->type) {
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
					gather_helper(scope_node_history->scope_history,
								  node_count,
								  explore_node,
								  explore_is_branch,
								  explore_node_histories,
								  explore_index,
								  wrapper);
				}
				break;
			}
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   int diversity_index,
					   SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch;
	vector<AbstractNode*> explore_node_histories;
	int explore_index;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch,
				  explore_node_histories,
				  explore_index,
				  wrapper);

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

		ExploreExperiment* new_experiment = new ExploreExperiment(
			diversity_index,
			explore_node->parent,
			explore_node,
			explore_is_branch,
			exit_next_node,
			wrapper);
		switch (explore_node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)explore_node;
				noop_node->experiments.push_back(new_experiment);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)explore_node;
				action_node->experiments.push_back(new_experiment);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)explore_node;
				scope_node->experiments.push_back(new_experiment);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)explore_node;
				if (explore_is_branch) {
					branch_node->branch_experiments.push_back(new_experiment);
				} else {
					branch_node->original_experiments.push_back(new_experiment);
				}
			}
			break;
		}
	}
}
