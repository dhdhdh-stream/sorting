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

const int GATHER_DEPENDENCIES_NUM_TRIES = 10;
const int MAX_NUM_DEPENDENCIES = 4;

class ExploreContext {
public:
	int node_count;

	AbstractNode* explore_node;
	bool explore_is_branch;
	ScopeHistory* scope_history;
	int explore_index;
};

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   map<Scope*, ExploreContext>& explore_contexts) {
	map<Scope*, ExploreContext>::iterator context_it = explore_contexts.find(scope_history->scope);
	if (context_it == explore_contexts.end()) {
		context_it = explore_contexts.insert({scope_history->scope, ExploreContext()}).first;
		context_it->second.node_count = 0;
		context_it->second.explore_node = NULL;
	}

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)node;
				if (noop_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
					context_it->second.node_count++;
					if (select_distribution(generator) == 0) {
						context_it->second.explore_node = node;
						context_it->second.explore_is_branch = false;
						context_it->second.scope_history = scope_history;
						context_it->second.explore_index = h_index;
					}
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
					context_it->second.node_count++;
					if (select_distribution(generator) == 0) {
						context_it->second.explore_node = node;
						context_it->second.explore_is_branch = false;
						context_it->second.scope_history = scope_history;
						context_it->second.explore_index = h_index;
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];

				gather_helper(scope_node_history->scope_history,
							  explore_contexts);

				if (scope_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
					context_it->second.node_count++;
					if (select_distribution(generator) == 0) {
						context_it->second.explore_node = node;
						context_it->second.explore_is_branch = false;
						context_it->second.scope_history = scope_history;
						context_it->second.explore_index = h_index;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[h_index];
				if (branch_node_history->is_branch) {
					if (branch_node->branch_experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
						context_it->second.node_count++;
						if (select_distribution(generator) == 0) {
							context_it->second.explore_node = node;
							context_it->second.explore_is_branch = true;
							context_it->second.scope_history = scope_history;
							context_it->second.explore_index = h_index;
						}
					}
				} else {
					if (branch_node->original_experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
						context_it->second.node_count++;
						if (select_distribution(generator) == 0) {
							context_it->second.explore_node = node;
							context_it->second.explore_is_branch = false;
							context_it->second.scope_history = scope_history;
							context_it->second.explore_index = h_index;
						}
					}
				}
			}
			break;
		}
	}
}

void compare_index(vector<int>& left, vector<int>& right, bool& right_later) {
	int layer = 0;
	while (true) {
		if (layer >= (int)left.size()) {
			right_later = false;
			return;
		} else if (layer >= (int)right.size()) {
			right_later = true;
			return;
		} else if (left[layer] < right[layer]) {
			right_later = true;
			return;
		} else if (left[layer] > right[layer]) {
			right_later = false;
			return;
		} else {
			layer++;
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	map<Scope*, ExploreContext> explore_contexts;
	gather_helper(scope_history,
				  explore_contexts);

	map<Scope*, ExploreContext>::iterator context_it;
	uniform_int_distribution<int> root_distribution(0, 3);
	if (root_distribution(generator) == 0) {
		context_it = explore_contexts.find(wrapper->solution->starting_scope);
	} else {
		uniform_int_distribution<int> scope_distribution(0, explore_contexts.size()-1);
		context_it = next(explore_contexts.begin(), scope_distribution(generator));
	}
	if (context_it->second.explore_node != NULL) {
		ScopeHistory* explore_scope_history = context_it->second.scope_history;

		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = context_it->second.explore_index + 1 + exit_distribution(generator);
			if (random_index < (int)explore_scope_history->node_histories.size() + 1) {
				break;
			}
		}
		AbstractNode* exit_next_node;
		if (random_index >= (int)explore_scope_history->node_histories.size()) {
			exit_next_node = NULL;
		} else {
			exit_next_node = explore_scope_history->node_histories[random_index]->node;
		}

		vector<vector<int>> dependencies;
		vector<vector<int>> indexes;
		for (int try_index = 0; try_index < GATHER_DEPENDENCIES_NUM_TRIES; try_index++) {
			vector<int> curr_context;
			vector<int> curr_index;
			int count = 0;
			vector<int> dependency;
			vector<int> index;
			gather_dependencies_top_helper(explore_scope_history,
										   context_it->second.explore_index,
										   curr_context,
										   curr_index,
										   count,
										   dependency,
										   index);
			bool matches_existing = false;
			for (int d_index = 0; d_index < (int)dependencies.size(); d_index++) {
				if (dependency == dependencies[d_index]) {
					matches_existing = true;
					break;
				}
			}
			if (!matches_existing) {
				int insert_index = 0;
				for (int i_index = 0; i_index < (int)indexes.size(); i_index++) {
					bool existing_later;
					compare_index(index, indexes[i_index], existing_later);
					if (existing_later) {
						break;
					} else {
						insert_index++;
					}
				}
				indexes.insert(indexes.begin() + insert_index, index);
				dependencies.insert(dependencies.begin() + insert_index, dependency);

				if (dependencies.size() >= MAX_NUM_DEPENDENCIES) {
					break;
				}
			}
		}

		// uniform_int_distribution<int> use_signal_distribution(0, 1);
		// bool use_signal = use_signal_distribution(generator) == 0;
		bool use_signal = false;

		ExploreExperiment* new_experiment = new ExploreExperiment(
			wrapper->diversity_index,
			context_it->second.explore_node->parent,
			context_it->second.explore_node,
			context_it->second.explore_is_branch,
			exit_next_node,
			dependencies,
			use_signal,
			wrapper);
		switch (context_it->second.explore_node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)context_it->second.explore_node;
				noop_node->experiment = new_experiment;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)context_it->second.explore_node;
				action_node->experiment = new_experiment;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)context_it->second.explore_node;
				scope_node->experiment = new_experiment;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)context_it->second.explore_node;
				if (context_it->second.explore_is_branch) {
					branch_node->branch_experiment = new_experiment;
				} else {
					branch_node->original_experiment = new_experiment;
				}
			}
			break;
		}
	}
}
