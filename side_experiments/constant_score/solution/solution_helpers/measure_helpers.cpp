#include "solution_helpers.h"

#include "globals.h"
#include "minesweeper.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 20;
#else
const int MEASURE_NUM_ITERS = 10000;
#endif /* MDEBUG */

double measure_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	double sum_vals = 0.0;
	for (int i_index = 0; i_index < MEASURE_NUM_ITERS; i_index++) {
		Problem* problem = problem_type->get_problem();
		wrapper->problem = problem;

		vector<double> obs = problem->get_observations();

		wrapper->init(RUN_TYPE_EXISTING,
					  obs);

		while (true) {
			pair<bool,int> next = wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}

			obs = problem->get_observations();
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * wrapper->num_actions;

		wrapper->end();

		sum_vals += target_val;

		delete problem;
	}

	delete problem_type;

	return sum_vals/MEASURE_NUM_ITERS;
}
