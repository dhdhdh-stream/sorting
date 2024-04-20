#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void inner_experiment(Problem* problem,
					  RunHelper& run_helper) {
	num_actions_until_experiment = -1;

	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = solution->scopes[2];
	context.back().node = NULL;

	ScopeHistory* root_history = new ScopeHistory(solution->scopes[2]);
	context.back().scope_history = root_history;

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	solution->scopes[2]->activate(
		problem,
		context,
		exit_depth,
		exit_node,
		run_helper,
		root_history);

	if (!eval_experiment) {
		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				if (rand()%2 == 0) {
					create_experiment(root_history);
				}
			}
		}
	}

	delete root_history;

	uniform_int_distribution<int> next_distribution(0, (int)solution->average_num_actions);
	num_actions_until_experiment = 1 + next_distribution(generator);
	geometric_distribution<int> skip_distribution(1.0/solution->average_num_actions * 4.0);
	num_actions_after_experiment_to_skip = skip_distribution(generator);
}
