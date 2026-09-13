#include "solution_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"

using namespace std;

void compare(ProblemType* problem_type,
			 vector<int>& actions) {
	vector<vector<double>> obs_histories;
	vector<double> target_val_histories;
	for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> obs = problem->get_observations();
		obs_histories.push_back(obs);

		for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
			problem->perform_action(actions[a_index]);
		}

		double target_val = problem->score_result();
		target_val_histories.push_back(target_val);

		delete problem;
	}

	Network* network = new Network(obs_histories[0].size());
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double hidden_3_average_max_update = 0.0;
	double output_average_max_update = 0.0;

	uniform_int_distribution<int> new_train_distribution(0, obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = new_train_distribution(generator);

		network->activate(obs_histories[rand_index]);

		double error = target_val_histories[rand_index] - network->output->acti_vals[0];

		network->init_backprop(error,
							   hidden_1_average_max_update,
							   hidden_2_average_max_update,
							   hidden_3_average_max_update,
							   output_average_max_update);
	}

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)obs_histories.size(); h_index++) {
		network->activate(obs_histories[h_index]);
		double predicted = network->output->acti_vals(0);
		sum_misguess += (target_val_histories[h_index] - predicted)
			* (target_val_histories[h_index] - predicted);
	}
	double misguess_average = sum_misguess / (double)obs_histories.size();
	cout << "compare: " << misguess_average << endl;

	delete network;
}
