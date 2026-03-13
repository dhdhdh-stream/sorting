#include "helpers.h"

using namespace std;

#include "minesweeper.h"
#include "solution_wrapper.h"

double result_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	double sum_vals = 0.0;

	#if defined(MDEBUG) && MDEBUG
	for (int i_index = 0; i_index < 10; i_index++) {
	#else
	for (int i_index = 0; i_index < 4000; i_index++) {
	#endif /* MDEBUG */
		Problem* problem = problem_type->get_problem();

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

		sum_vals += target_val;

		delete problem;
	}

	delete problem_type;

	return sum_vals/4000.0;
}
