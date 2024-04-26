#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void generalize_helper(Problem* problem,
					   RunHelper& run_helper) {
	solution->num_actions_until_experiment = -1;

	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = solution->scopes.back();
	context.back().node = NULL;

	ScopeHistory* root_history = new ScopeHistory(solution->scopes.back());
	context.back().scope_history = root_history;

	solution->scopes.back()->activate(
		problem,
		context,
		run_helper,
		root_history);

	if (!run_helper.eval_experiment) {
		if (run_helper.experiments_seen_order.size() == 0) {
			if (!run_helper.exceeded_limit) {
				if (rand()%2 == 0) {
					create_experiment(root_history);
				}
			}
		}
	}

	delete root_history;

	uniform_int_distribution<int> next_distribution(0, (int)(solution->average_num_actions/2.0));
	solution->num_actions_until_experiment = 1 + next_distribution(generator);
	geometric_distribution<int> skip_distribution(1.0/solution->average_num_actions * 8.0);
	run_helper.num_actions_after_experiment_to_skip = skip_distribution(generator);
}
