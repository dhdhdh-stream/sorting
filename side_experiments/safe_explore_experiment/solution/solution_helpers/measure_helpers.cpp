#include "solution_helpers.h"

#include "minesweeper.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 20;
#else
const int MEASURE_NUM_ITERS = 2000;
#endif /* MDEBUG */

double measure_helper(SolutionWrapper* solution_wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	double sum_vals = 0.0;

	for (int i_index = 0; i_index < MEASURE_NUM_ITERS; i_index++) {
		Problem* problem = problem_type->get_problem();
		solution_wrapper->problem = problem;

		solution_wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		solution_wrapper->end();

		sum_vals += target_val;

		delete problem;
	}

	double average_score = sum_vals/MEASURE_NUM_ITERS;

	delete problem_type;

	return average_score;
}
