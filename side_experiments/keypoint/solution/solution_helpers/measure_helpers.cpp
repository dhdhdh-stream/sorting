#include "solution_helpers.h"

using namespace std;

const int KEYPOINT_EXPERIMENTS_PER_MEASURE = 10;

void measure() {
	double sum_score = 0.0;
	vector<ScopeHistory*> scope_histories;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		run_index++;

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(
			problem,
			run_helper,
			scope_history);
		
		double target_val = problem->score_result();
		sum_score += target_val;

		update_scores(scope_history,
					  target_val);

		scope_histories.push_back(scope_history);

		delete problem;
	}

	solution->measure_update();

	solution->curr_score = sum_score / MEASURE_ITERS;

	for (int k_index = 0; k_index < KEYPOINT_EXPERIMENTS_PER_MEASURE; k_index++) {
		keypoint_experiment(scope_histories);
	}

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		delete scope_histories[h_index];
	}
}
