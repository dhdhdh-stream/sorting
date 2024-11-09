#include "solution_helpers.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_IMPACT_ITERS = 10;
#else
const int UPDATE_IMPACT_ITERS = 1000;
#endif /* MDEBUG */

void update_impact() {
	for (int iter_index = 0; iter_index < UPDATE_IMPACT_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> existing_context;
		solution->scopes[0]->activate(
				problem,
				existing_context,
				run_helper);

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		} else {
			target_val = -1.0;
		}
		run_helper.result = target_val;

		Problem* copy_problem = problem->copy_and_reset();

		run_helper.exceeded_limit = false;

		run_helper.num_analyze = 0;
		run_helper.num_actions = 0;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> flip_context;
		solution->scopes[0]->flip_activate(
				copy_problem,
				flip_context,
				run_helper);

		delete copy_problem;

		delete problem;
	}
}
