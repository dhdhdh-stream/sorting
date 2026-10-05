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
				   map<Scope*, ExploreContext>& explore_contexts,
				   SolutionWrapper* wrapper) {
	if (scope_history->scope != wrapper->solution->outer_scope) {
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
					uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
					context_it->second.node_count++;
					if (select_distribution(generator) == 0) {
						context_it->second.explore_node = node;
						context_it->second.explore_is_branch = false;
						context_it->second.scope_history = scope_history;
						context_it->second.explore_index = h_index;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node;
					if (!action_node->is_generic) {
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
								  explore_contexts,
								  wrapper);

					if (!scope_node->is_generic) {
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
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[h_index];
					if (branch_node_history->is_branch) {
						uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
						context_it->second.node_count++;
						if (select_distribution(generator) == 0) {
							context_it->second.explore_node = node;
							context_it->second.explore_is_branch = true;
							context_it->second.scope_history = scope_history;
							context_it->second.explore_index = h_index;
						}
					} else {
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
			}
		}
	} else {
		for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
			AbstractNode* node = scope_history->node_histories[h_index]->node;
			switch (node->type) {
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];
					gather_helper(scope_node_history->scope_history,
								  explore_contexts,
								  wrapper);
				}
				break;
			}
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	map<Scope*, ExploreContext> explore_contexts;
	gather_helper(scope_history,
				  explore_contexts,
				  wrapper);

	map<Scope*, ExploreContext>::iterator context_it;
	uniform_int_distribution<int> top_distribution(0, 3);
	if (top_distribution(generator) == 0) {
		context_it = explore_contexts.find(wrapper->solution->top_scope);
	} else {
		uniform_int_distribution<int> scope_distribution(0, explore_contexts.size()-1);
		context_it = next(explore_contexts.begin(), scope_distribution(generator));
	}
	if (context_it->second.explore_node != NULL) {
		geometric_distribution<int> exit_distribution(0.1);
		AbstractNode* exit_next_node;
		while (true) {
			int random_index = context_it->second.explore_index + 1 + exit_distribution(generator);
			if (random_index >= (int)context_it->second.scope_history->node_histories.size()) {
				exit_next_node = NULL;
				break;
			} else {
				exit_next_node = context_it->second.scope_history->node_histories[random_index]->node;
				bool is_generic = false;
				switch (exit_next_node->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)exit_next_node;
						if (action_node->is_generic) {
							is_generic = true;
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)exit_next_node;
						if (scope_node->is_generic) {
							is_generic = true;
						}
					}
					break;
				}
				if (!is_generic) {
					break;
				}
			}
		}

		ExploreExperiment* new_experiment = new ExploreExperiment(
			wrapper,
			context_it->second.explore_node->parent,
			context_it->second.explore_node,
			context_it->second.explore_is_branch,
			exit_next_node);
		switch (context_it->second.explore_node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)context_it->second.explore_node;
				noop_node->experiments.push_back(new_experiment);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)context_it->second.explore_node;
				action_node->experiments.push_back(new_experiment);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)context_it->second.explore_node;
				scope_node->experiments.push_back(new_experiment);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)context_it->second.explore_node;
				if (context_it->second.explore_is_branch) {
					branch_node->branch_experiments.push_back(new_experiment);
				} else {
					branch_node->original_experiments.push_back(new_experiment);
				}
			}
			break;
		}
	}
}
