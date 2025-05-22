#include "solution_helpers.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void result_helper(Problem* original_problem,
				   RunHelper& run_helper,
				   AbstractExperiment*& curr_experiment) {
	Problem* copy_problem = original_problem->copy_and_reset();

	ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
	solution->scopes[0]->result_activate(
			copy_problem,
			run_helper,
			scope_history);

	double target_val = copy_problem->score_result();
	target_val -= run_helper.num_actions * 0.0001;

	if (curr_experiment == NULL) {
		create_experiment(scope_history,
						  curr_experiment);
	}

	delete scope_history;
	delete copy_problem;

	run_helper.result = target_val;

	run_helper.num_actions = 0;
}
