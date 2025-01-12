#include "solution_helpers.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

double get_existing_result(Problem* original_problem) {
	Problem* copy_problem = original_problem->copy_and_reset();

	RunHelper run_helper;

	#if defined(MDEBUG) && MDEBUG
	run_helper.starting_run_seed = run_index;
	run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
	#endif /* MDEBUG */

	vector<ContextLayer> context;
	solution->scopes[0]->activate(
			copy_problem,
			context,
			run_helper);

	double target_val = copy_problem->score_result();
	target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
	target_val -= run_helper.num_analyze * solution->curr_time_penalty;

	delete copy_problem;

	return target_val;
}
