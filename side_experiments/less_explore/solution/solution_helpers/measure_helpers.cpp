#include "solution_helpers.h"

#include "minesweeper.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_MEASURE = 10;
#else
const int NUM_MEASURE = 2000;
#endif /* MDEBUG */

double measure_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	double sum_score = 0.0;
	for (int i_index = 0; i_index < 2000; i_index++) {
		Problem* problem = problem_type->get_problem();
		wrapper->problem = problem;

		wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * wrapper->num_actions;

		wrapper->end();

		sum_score += target_val;

		delete problem;
	}

	delete problem_type;

	return sum_score / NUM_MEASURE;
}
