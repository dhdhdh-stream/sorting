#include "solution_helpers.h"

#include "problem.h"
#include "solution_wrapper.h"

using namespace std;

double get_existing_result(SolutionWrapper* wrapper) {
	Problem* copy_problem = wrapper->problem->copy_and_reset();

	wrapper->init();

	while (true) {
		vector<double> obs = copy_problem->get_observations();

		pair<bool,int> next = wrapper->step(obs);
		if (next.first) {
			break;
		} else {
			copy_problem->perform_action(next.second);
		}
	}

	double target_val = copy_problem->score_result();
	target_val -= 0.0001 * wrapper->num_actions;

	wrapper->end();

	delete copy_problem;

	return target_val;
}
