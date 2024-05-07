#include "new_action_helpers.h"

#include "abstract_node.h"
#include "globals.h"
#include "new_action_tracker.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void new_action_activate(AbstractNode* experiment_node,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper) {
	map<AbstractNode*, NewActionNodeTracker*>::iterator it =
		solution->new_action_tracker->node_trackers.find(experiment_node);
	if (it != solution->new_action_tracker->node_trackers.end()) {
		if (solution->num_actions_until_experiment == 0) {
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

			solution->num_actions_until_experiment = 1 + solution->new_action_tracker->num_actions_until_distribution(generator);
		} else {
			run_helper.new_action_history->existing_path_taken.push_back(experiment_node);
		}
	}
}
