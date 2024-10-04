#include "solution_helpers.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void get_existing_result(Problem* original_problem,
						 double& result,
						 bool& hit_subproblem) {
	Problem* copy_problem = original_problem->copy_and_reset();

	RunHelper run_helper;

	#if defined(MDEBUG) && MDEBUG
	run_helper.starting_run_seed = run_index;
	run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
	#endif /* MDEBUG */

	vector<ContextLayer> context;
	solution->scopes[0]->result_activate(
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

	result = target_val;
	hit_subproblem = run_helper.hit_subproblem;
}
