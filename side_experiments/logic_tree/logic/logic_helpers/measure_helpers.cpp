#include "logic_helpers.h"

#include "abstract_problem.h"
#include "constants.h"
#include "logic_helpers.h"
#include "logic_tree.h"
#include "logic_wrapper.h"

using namespace std;

double measure_helper(AbstractProblem* problem,
					  LogicWrapper* logic_wrapper) {
	double sum_misguess = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		vector<double> obs;
		double target_val;
		problem->get_test_instance(obs,
								   target_val);

		double eval = logic_eval_helper(logic_wrapper->logic_tree->root,
										obs);

		double misguess = (target_val - eval) * (target_val - eval);
		sum_misguess += misguess;
	}

	return sum_misguess / MEASURE_ITERS;
}
