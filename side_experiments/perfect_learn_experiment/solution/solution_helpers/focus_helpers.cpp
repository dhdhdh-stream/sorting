#include "solution_helpers.h"

#include <iostream>

#include "action_network.h"
#include "constants.h"
#include "obs_network.h"
#include "problem.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_MEASURE_SAMPLES = 20;
#else
const int NUM_MEASURE_SAMPLES = 4000;
#endif /* MDEBUG */

void focus(ProblemType* problem_type,
		   vector<int>& actions,
		   Solution* solution) {
	double sum_misguess = 0.0;
	for (int i_index = 0; i_index < NUM_MEASURE_SAMPLES; i_index++) {
		Problem* problem = problem_type->get_problem();

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);

		vector<double> obs = problem->get_observations();
		solution->obs_network->activate(state,
										obs);

		for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
			problem->perform_action(actions[a_index]);

			solution->action_networks[actions[a_index]]->activate(state);
		}

		double target_val = problem->score_result();

		solution->score_network->activate(state);
		double predicted = solution->score_network->output->acti_vals(0);

		sum_misguess += (target_val - predicted) * (target_val - predicted);

		delete problem;
	}
	double misguess_average = sum_misguess / NUM_MEASURE_SAMPLES;
	cout << "focus: " << misguess_average << endl;
}
