#include "nn_helpers.h"

#include <iostream>
#include <vector>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"

#if defined(MDEBUG) && MDEBUG
const int NUM_EPOCHS = 2;
const int NUM_SAMPLES = 10;
const int TRAIN_ITERS = 200;
#else
const int NUM_EPOCHS = 15;
const int NUM_SAMPLES = 1000;
const int TRAIN_ITERS = 100000;
#endif /* MDEBUG */

using namespace std;

void train_rl_helper(vector<int>& actions,
					 vector<vector<double>>& obs_vals,
					 double target_val,
					 vector<Network*>& networks) {
	for (int s_index = 0; s_index < (int)actions.size(); s_index++) {
		networks[actions[s_index] + 1]->backprop(
			obs_vals[s_index],
			target_val);
	}
}

void train_rl(vector<Network*>& networks) {
	/**
	 * - occasional random is better than temperature
	 *   - explore should be evaluated given other actions are correct
	 *     - vs. the random to random from temperature
	 */
	uniform_int_distribution<int> random_distribution(0, 4);
	uniform_int_distribution<int> random_action_distribution(-1, problem_type->num_possible_actions()-1);
	uniform_int_distribution<int> sample_distribution(0, NUM_SAMPLES-1);
	for (int epoch_index = 0; epoch_index < NUM_EPOCHS; epoch_index++) {
		vector<vector<int>> actions;
		vector<vector<vector<double>>> obs_vals;
		vector<double> target_vals;
		for (int gather_index = 0; gather_index < NUM_SAMPLES; gather_index++) {
			vector<int> curr_actions;
			vector<vector<double>> curr_obs_vals;

			Problem* problem = problem_type->get_problem();

			int iter_index = 0;
			while (true) {
				vector<double> obs;
				vector<vector<int>> locations;
				problem->get_observations(obs,
										  locations);

				int action;
				if (epoch_index == 0
						|| random_distribution(generator) == 0) {
					action = random_action_distribution(generator);
				} else {
					int best_index = -1;
					double best_val = numeric_limits<double>::lowest();
					for (int a_index = 0; a_index < (int)networks.size(); a_index++) {
						networks[a_index]->activate(obs);
						if (networks[a_index]->output->acti_vals[0] > best_val) {
							best_index = a_index;
							best_val = networks[a_index]->output->acti_vals[0];
						}
					}
					action = best_index - 1;

					// // temp
					// if (gather_index%100 == 0) {
					// 	cout << iter_index << endl;
					// 	cout << "obs[0]: " << obs[0] << endl;
					// 	for (int a_index = 0; a_index < (int)networks.size(); a_index++) {
					// 		cout << a_index << ": " << networks[a_index]->output->acti_vals[0] << endl;
					// 	}
					// 	cout << endl;
					// }
				}

				curr_actions.push_back(action);
				curr_obs_vals.push_back(obs);

				if (action == ACTION_TERMINATE) {
					break;
				}

				problem->perform_action(Action(action));

				iter_index++;
				if (iter_index >= RL_MAX_ITERS) {
					break;
				}
			}

			double target_val = problem->score_result(iter_index);

			actions.push_back(curr_actions);
			obs_vals.push_back(curr_obs_vals);
			target_vals.push_back(target_val);

			delete problem;
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)target_vals.size(); h_index++) {
			sum_vals += target_vals[h_index];
		}
		double average_val = sum_vals / (int)target_vals.size();
		cout << "average_val: " << average_val << endl;
		for (int h_index = 0; h_index < (int)target_vals.size(); h_index++) {
			target_vals[h_index] -= average_val;
		}

		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			train_rl_helper(actions[rand_index],
							obs_vals[rand_index],
							target_vals[rand_index],
							networks);
		}
	}
}
