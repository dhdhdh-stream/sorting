#include "solution_helpers.h"

#include "run.h"
#include "test_indirect.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 40;
#else
const int MEASURE_NUM_ITERS = 4000;
#endif /* MDEBUG */

double measure_helper(Wrapper* wrapper) {
	ProblemType* problem_type = new TypeTestIndirect();

	double sum_scores = 0.0;
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

		delete run;

		delete problem;
	}

	double score_average = sum_scores / MEASURE_NUM_ITERS;

	delete problem_type;

	return score_average;
}
