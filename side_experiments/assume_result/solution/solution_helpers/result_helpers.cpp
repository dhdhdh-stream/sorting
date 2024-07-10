#include "solution_helpers.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"

using namespace std;

double get_existing_result(Problem* original_problem) {
	Problem* copy_problem = original_problem->copy_and_reset();

	RunHelper run_helper;

	vector<ContextLayer> context;
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
	solution->scopes[0]->measure_activate(
			copy_problem,
			context,
			run_helper);
	
	double target_val;
	if (!run_helper.exceeded_limit) {
		target_val = copy_problem->score_result(run_helper.num_analyze,
												run_helper.num_actions);
	} else {
		target_val = -1.0;
	}

	delete copy_problem;

	return target_val;
}
