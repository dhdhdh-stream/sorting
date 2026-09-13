#include "solution_helpers.h"

#include <iostream>

#include "action_network.h"
#include "constants.h"
#include "obs_network.h"
#include "problem.h"
#include "score_network.h"
#include "solution.h"

using namespace std;

void predict_decision_helper(ProblemType* problem_type,
							 vector<int>& actions_1,
							 vector<int>& actions_2,
							 Solution* solution) {
	double sum_scores = 0.0;
	for (int iter_index = 0; iter_index < NUM_MEASURE_SAMPLES; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> obs = problem->get_observations();

		double predicted_1;
		{
			Eigen::VectorXf state;
			state.resize(NUM_STATES);
			state.setConstant(0.0);

			solution->obs_network->activate(state,
											obs);

			for (int a_index = 0; a_index < (int)actions_1.size(); a_index++) {
				solution->action_networks[actions_1[a_index]]->activate(state);
			}

			solution->score_network->activate(state);
			predicted_1 = solution->score_network->output->acti_vals(0);
		}

		double predicted_2;
		{
			Eigen::VectorXf state;
			state.resize(NUM_STATES);
			state.setConstant(0.0);

			solution->obs_network->activate(state,
											obs);

			for (int a_index = 0; a_index < (int)actions_2.size(); a_index++) {
				solution->action_networks[actions_2[a_index]]->activate(state);
			}

			solution->score_network->activate(state);
			predicted_2 = solution->score_network->output->acti_vals(0);
		}

		if (predicted_1 > predicted_2) {
			for (int a_index = 0; a_index < (int)actions_1.size(); a_index++) {
				problem->perform_action(actions_1[a_index]);
			}
		} else {
			for (int a_index = 0; a_index < (int)actions_2.size(); a_index++) {
				problem->perform_action(actions_2[a_index]);
			}
		}

		double target_val = problem->score_result();
		sum_scores += target_val;

		delete problem;
	}
	double average_score = sum_scores / NUM_MEASURE_SAMPLES;
	cout << "predict decision: " << average_score << endl;
}
