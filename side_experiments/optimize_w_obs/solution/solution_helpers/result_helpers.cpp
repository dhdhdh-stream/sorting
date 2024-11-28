#include "solution_helpers.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void get_existing_result(Problem* original_problem,
						 RunHelper& run_helper) {
	Problem* copy_problem = original_problem->copy_and_reset();

	#if defined(MDEBUG) && MDEBUG
	run_helper.starting_run_seed = run_index;
	run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
	#endif /* MDEBUG */

	vector<ContextLayer> context;
	solution->scopes[0]->result_activate(
			copy_problem,
			context,
			run_helper);

	if (run_helper.experiments_seen_order.size() == 0
			&& run_helper.nodes_seen.size() != 0) {
		create_experiment(run_helper);
	}

	if (run_helper.experiment_histories.size() == 0) {
		for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
			AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
			experiment->average_remaining_experiments_from_start =
				0.9 * experiment->average_remaining_experiments_from_start
				+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
		}
	}

	double target_val = copy_problem->score_result();
	target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
	target_val -= run_helper.num_analyze * solution->curr_time_penalty;
	run_helper.result = target_val;

	delete copy_problem;
}
