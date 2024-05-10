#include "new_action_helpers.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "new_action_tracker.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void new_action_activate(AbstractNode* experiment_node,
						 bool is_branch,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper) {
	map<AbstractNode*, NewActionNodeTracker*>::iterator it =
		solution->new_action_tracker->node_trackers.find(experiment_node);
	if (it != solution->new_action_tracker->node_trackers.end()) {
		if (it->second->is_branch == is_branch) {
			uniform_int_distribution<int> select_distribution(0, 1);
			if (run_helper.is_always_select
					|| select_distribution(generator) == 0) {
				run_helper.new_action_history->new_path_taken.push_back(experiment_node);

				vector<ContextLayer> inner_context;
				inner_context.push_back(ContextLayer());

				inner_context.back().scope = solution->scopes.back();
				inner_context.back().node = NULL;

				ScopeHistory* generalize_history = new ScopeHistory(solution->scopes.back());
				inner_context.back().scope_history = generalize_history;

				solution->scopes.back()->activate(
					problem,
					inner_context,
					run_helper,
					generalize_history);

				if (run_helper.experiments_seen_order.size() == 0) {
					if (!run_helper.exceeded_limit) {
						uniform_int_distribution<int> target_distribution(0, 1);
						if (target_distribution(generator) == 0) {
							create_experiment(generalize_history);
						}
					}
				}

				delete generalize_history;

				curr_node = it->second->exit_next_node;
			} else {
				run_helper.new_action_history->existing_path_taken.push_back(experiment_node);
			}
		}
	}
}
