#include "solution_helpers.h"

#include <iostream>

#include "hit_all.h"
// #include "minesweeper.h"
#include "run.h"
#include "state_network.h"
// #include "test_indirect.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 40;
#else
const int MEASURE_NUM_ITERS = 4000;
#endif /* MDEBUG */

void measure_helper(Wrapper* wrapper,
					double& score_average,
					double& misguess_average) {
	// ProblemType* problem_type = new TypeTestIndirect();
	// ProblemType* problem_type = new TypeMinesweeper();
	ProblemType* problem_type = new TypeHitAll();

	double sum_scores = 0.0;
	double sum_misguess = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_NUM_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		Run* run = new Run();

		wrapper->init(run);

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs,
												run);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		sum_scores += target_val;

		wrapper->world_model->curr_final_network->activate(run->state);
		double predicted = wrapper->world_model->curr_final_network->output->acti_vals[0];

		sum_misguess += (target_val - predicted) * (target_val - predicted);

		delete run;

		delete problem;
	}

	score_average = sum_scores / MEASURE_NUM_ITERS;
	misguess_average = sum_misguess / MEASURE_NUM_ITERS;;

	delete problem_type;
}
