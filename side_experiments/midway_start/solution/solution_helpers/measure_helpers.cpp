#include "solution_helpers.h"

#include "constants.h"
#include "minesweeper.h"
#include "solution_wrapper.h"

using namespace std;

double measure_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	double sum_vals = 0.0;
	for (int iter_index = 0; iter_index < EXPERIMENT_NUM_DATAPOINTS; iter_index++) {
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

		sum_vals += target_val;

		delete problem;
	}

	delete problem_type;

	return sum_vals / EXPERIMENT_NUM_DATAPOINTS;
}
