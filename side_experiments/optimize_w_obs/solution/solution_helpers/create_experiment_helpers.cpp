#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void even_distribution_helper(ScopeHistory* scope_history,
							  map<pair<AbstractNode*,bool>, pair<int,pair<ScopeHistory*,int>>>& counts) {
	for (map<int, AbstractNodeHistory*>::iterator history_it = scope_history->node_histories.begin();
			history_it != scope_history->node_histories.end(); history_it++) {
		if (history_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history_it->second;
			even_distribution_helper(scope_node_history->scope_history,
									 counts);
		}

		AbstractNode* curr_node = history_it->second->node;
		bool curr_is_branch;
		switch (history_it->second->node->type) {
		case NODE_TYPE_ACTION:
		case NODE_TYPE_SCOPE:
			curr_is_branch = false;
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history_it->second;
				curr_is_branch = branch_node_history->is_branch;
			}
			break;
		}

		map<pair<AbstractNode*,bool>, pair<int,pair<ScopeHistory*,int>>>::iterator count_it =
			counts.find({curr_node, curr_is_branch});
		if (count_it == counts.end()) {
			counts[{curr_node, curr_is_branch}] = {1, {scope_history, history_it->second->index}};
		} else {
			uniform_int_distribution<int> select_distribution(0, count_it->second.first);
			if (select_distribution(generator) == 0) {
				count_it->second.second = {scope_history, history_it->second->index};
			}

			count_it->second.first++;
		}
	}
}

void frequency_distribution_helper(ScopeHistory* scope_history,
								   int& count,
								   AbstractNode*& explore_node,
								   bool& explore_is_branch,
								   ScopeHistory*& explore_scope_history,
								   int& explore_index) {
	for (map<int, AbstractNodeHistory*>::iterator history_it = scope_history->node_histories.begin();
			history_it != scope_history->node_histories.end(); history_it++) {
		if (history_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history_it->second;
			frequency_distribution_helper(
				scope_node_history->scope_history,
				count,
				explore_node,
				explore_is_branch,
				explore_scope_history,
				explore_index);
		}

		uniform_int_distribution<int> select_distribution(0, count);
		if (select_distribution(generator) == 0) {
			explore_node = history_it->second->node;
			switch (history_it->second->node->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				explore_is_branch = false;
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history_it->second;
					explore_is_branch = branch_node_history->is_branch;
				}
				break;
			}
			explore_scope_history = scope_history;
			explore_index = history_it->second->index;
		}

		count++;
	}
}

void create_experiment(ScopeHistory* scope_history) {
	AbstractNode* explore_node;
	bool explore_is_branch;
	ScopeHistory* explore_scope_history;
	int explore_index;
	uniform_int_distribution<int> even_distribution(0, 1);
	if (even_distribution(generator) == 0) {
		map<pair<AbstractNode*,bool>, pair<int,pair<ScopeHistory*,int>>> counts;
		even_distribution_helper(scope_history,
								 counts);

		uniform_int_distribution<int> explore_node_distribution(0, counts.size()-1);
		map<pair<AbstractNode*,bool>, pair<int,pair<ScopeHistory*,int>>>::iterator it = next(counts.begin(), explore_node_distribution(generator));
		explore_node = it->first.first;
		explore_is_branch = it->first.second;
		explore_scope_history = it->second.second.first;
		explore_index = it->second.second.second;
	} else {
		int count = 0;
		frequency_distribution_helper(scope_history,
									  count,
									  explore_node,
									  explore_is_branch,
									  explore_scope_history,
									  explore_index);
	}
	/**
	 * - don't weigh based on number of nodes within scope
	 *   - can get trapped by small useless scopes
	 *     - may be good for certain decision heavy scopes to have lots of nodes
	 */

	Scope* explore_scope = (Scope*)explore_node->parent;

	/**
	 * - don't focus on generalization/reuse
	 *   - may block progress if incompatible spots are grouped together
	 *   - (though may also greatly speed up future progress of course)
	 */
	#if defined(MDEBUG) && MDEBUG
	uniform_int_distribution<int> non_new_distribution(0, 1);
	#else
	uniform_int_distribution<int> non_new_distribution(0, 9);
	#endif /* MDEBUG */
	if (explore_scope->new_scope_experiment == NULL
			&& explore_node->parent->nodes.size() > 10
			&& non_new_distribution(generator) != 0) {
		NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
			explore_node->parent,
			explore_node,
			explore_is_branch);

		if (new_scope_experiment->result == EXPERIMENT_RESULT_FAIL) {
			delete new_scope_experiment;
		} else {
			explore_scope->new_scope_experiment = new_scope_experiment;
			explore_node->experiments.push_back(new_scope_experiment);
		}
	} else {
		/**
		 * - weigh towards PassThroughExperiments as cheaper and potentially just as effective
		 *   - solutions are often made of relatively few distinct decisions, but applied such that has good coverage
		 *     - like tessellation, but have to get both the shape and the pattern correct
		 *       - and PassThroughExperiments help with both
		 */
		uniform_int_distribution<int> pass_through_distribution(0, 3);
		if (pass_through_distribution(generator) == 0) {
			PassThroughExperiment* new_experiment = new PassThroughExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch);

			explore_node->experiments.push_back(new_experiment);
		} else {
			BranchExperiment* new_experiment = new BranchExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch);

			explore_node->experiments.push_back(new_experiment);
		}
	}
}
