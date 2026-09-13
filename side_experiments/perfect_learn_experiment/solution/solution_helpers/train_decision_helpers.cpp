#include "solution_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"

using namespace std;

void train_decision_helper(ProblemType* problem_type,
						   vector<int>& actions_1,
						   vector<int>& actions_2) {
	Network* network_1;
	{
		vector<vector<double>> obs_histories;
		vector<double> target_val_histories;
		for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
			Problem* problem = problem_type->get_problem();

			vector<double> obs = problem->get_observations();
			obs_histories.push_back(obs);

			for (int a_index = 0; a_index < (int)actions_1.size(); a_index++) {
				problem->perform_action(actions_1[a_index]);
			}

			double target_val = problem->score_result();
			target_val_histories.push_back(target_val);

			delete problem;
		}

		network_1 = new Network(obs_histories[0].size());
		double hidden_1_average_max_update = 0.0;
		double hidden_2_average_max_update = 0.0;
		double hidden_3_average_max_update = 0.0;
		double output_average_max_update = 0.0;

		uniform_int_distribution<int> new_train_distribution(0, obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = new_train_distribution(generator);

			network_1->activate(obs_histories[rand_index]);

			double error = target_val_histories[rand_index] - network_1->output->acti_vals[0];

			network_1->init_backprop(error,
									 hidden_1_average_max_update,
									 hidden_2_average_max_update,
									 hidden_3_average_max_update,
									 output_average_max_update);
		}
	}

	Network* network_2;
	{
		vector<vector<double>> obs_histories;
		vector<double> target_val_histories;
		for (int i_index = 0; i_index < NUM_TRAIN_SAMPLES; i_index++) {
			Problem* problem = problem_type->get_problem();

			vector<double> obs = problem->get_observations();
			obs_histories.push_back(obs);

			for (int a_index = 0; a_index < (int)actions_2.size(); a_index++) {
				problem->perform_action(actions_2[a_index]);
			}

			double target_val = problem->score_result();
			target_val_histories.push_back(target_val);

			delete problem;
		}

		network_2 = new Network(obs_histories[0].size());
		double hidden_1_average_max_update = 0.0;
		double hidden_2_average_max_update = 0.0;
		double hidden_3_average_max_update = 0.0;
		double output_average_max_update = 0.0;

		uniform_int_distribution<int> new_train_distribution(0, obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = new_train_distribution(generator);

			network_2->activate(obs_histories[rand_index]);

			double error = target_val_histories[rand_index] - network_2->output->acti_vals[0];

			network_2->init_backprop(error,
									 hidden_1_average_max_update,
									 hidden_2_average_max_update,
									 hidden_3_average_max_update,
									 output_average_max_update);
		}
	}

	double sum_scores = 0.0;
	for (int iter_index = 0; iter_index < NUM_MEASURE_SAMPLES; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> obs = problem->get_observations();
		network_1->activate(obs);
		network_2->activate(obs);

		if (network_1->output->acti_vals(0) > network_2->output->acti_vals(0)) {
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
	cout << "train decision: " << average_score << endl;

	delete network_1;
	delete network_2;
}
