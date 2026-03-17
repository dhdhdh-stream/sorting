#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment.h"
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
	Scope* scope = scope_history->scope;

	if (scope->is_outer) {
		return;
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (scope->nodes.find(h_it->first) == scope->nodes.end()) {
			continue;
		}

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

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		vector<AbstractNode*> possible_pres;
		explore_node->parent->random_pre(
			explore_node,
			possible_pres);

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

		vector<AbstractNode*> possible_exits;
		explore_node->parent->random_exit(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> pre_distribution(0, possible_pres.size()-1);
		int pre_index = pre_distribution(generator);
		AbstractNode* pre_node = possible_pres[pre_index];
		uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
		int exit_index = exit_distribution(generator);
		AbstractNode* exit_node = possible_exits[exit_index];

		// // temp
		// cout << "possible_pres.size(): " << possible_pres.size() << endl;
		// cout << "possible_exits.size(): " << possible_exits.size() << endl;

		bool can_protect = check_can_protect(pre_node,
											 exit_node);
		if (can_protect) {
			// // temp
			// for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
			// 	Scope* scope = wrapper->solution->scopes[s_index];

			// 	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			// 			it != scope->nodes.end(); it++) {
			// 		if (it->second->protect_type != PROTECT_TYPE_NA) {
			// 			throw invalid_argument("it->second->protect_type != PROTECT_TYPE_NA");
			// 		}
			// 	}

			// 	if (scope->is_protect_end) {
			// 		throw invalid_argument("scope->is_protect_end");
			// 	}
			// }

			pre_node->protect_type = PROTECT_TYPE_START;
			if (exit_node == NULL) {
				explore_node->parent->is_protect_end = true;
			} else {
				exit_node->protect_type = PROTECT_TYPE_END;
			}

			geometric_distribution<int> exit_distribution(0.1);
			int random_index;
			while (true) {
				random_index = exit_distribution(generator);
				if (random_index <= exit_index) {
					break;
				}
			}
			AbstractNode* exit_next_node = possible_exits[random_index];

			// // temp
			// cout << "explore_node->parent->id: " << explore_node->parent->id << endl;
			// cout << "pre_index: " << pre_index << endl;
			// cout << "pre_node->id: " << pre_node->id << endl;
			// cout << "exit_index: " << exit_index << endl;
			// if (exit_node == NULL) {
			// 	cout << "exit_node->id: -1" << endl;
			// } else {
			// 	cout << "exit_node->id: " << exit_node->id << endl;
			// }
			// cout << "random_index: " << random_index << endl;
			// if (exit_next_node == NULL) {
			// 	cout << "exit_next_node->id: -1" << endl;
			// } else {
			// 	cout << "exit_next_node->id: " << exit_next_node->id << endl;
			// }

			Experiment* new_experiment = new Experiment(
				explore_node->parent,
				explore_node,
				explore_is_branch,
				exit_next_node);
			wrapper->curr_experiment = new_experiment;

			new_experiment->protect_pre_node = pre_node;
			new_experiment->protect_exit_node = exit_node;

			wrapper->solution->num_experiments++;
		}
	}
}
