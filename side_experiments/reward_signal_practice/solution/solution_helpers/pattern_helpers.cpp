#include "solution_helpers.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "problem.h"

using namespace std;

const int MEASURE_ITERS = 1000;

const double FACTOR_MAX_AVERAGE_T_SCORE = 1.0;

void measure_keypoints(vector<int>& full_sequence,
					   int pattern_start_index,
					   vector<int>& keypoints,
					   vector<double>& keypoint_averages,
					   vector<double>& keypoint_standard_deviations) {
	vector<vector<double>> keypoint_obs(keypoints.size());
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		for (int a_index = 0; a_index < (int)full_sequence.size(); a_index++) {
			problem->perform_action(full_sequence[a_index]);

			for (int k_index = 0; k_index < (int)keypoints.size(); k_index++) {
				if (a_index == pattern_start_index + keypoints[k_index]) {
					vector<double> obs = problem->get_observations();
					keypoint_obs[k_index].push_back(obs[0]);
				}
			}
		}

		delete problem;
	}

	for (int k_index = 0; k_index < (int)keypoints.size(); k_index++) {
		double sum_vals = 0.0;
		for (int h_index = 0; h_index < MEASURE_ITERS; h_index++) {
			sum_vals += keypoint_obs[k_index][h_index];
		}
		double average = sum_vals / (double)MEASURE_ITERS;

		double sum_variances = 0.0;
		for (int h_index = 0; h_index < MEASURE_ITERS; h_index++) {
			sum_variances += (keypoint_obs[k_index][h_index] - average)
				* (keypoint_obs[k_index][h_index] - average);
		}
		double standard_deviation = sqrt(sum_variances / (double)MEASURE_ITERS);
		if (standard_deviation < MIN_STANDARD_DEVIATION) {
			standard_deviation = MIN_STANDARD_DEVIATION;
		}

		keypoint_averages.push_back(average);
		keypoint_standard_deviations.push_back(standard_deviation);
	}
}

void train_pattern_network(vector<int>& actions,
						   vector<int>& keypoints,
						   vector<double>& keypoint_averages,
						   vector<double>& keypoint_standard_deviations,
						   vector<int>& inputs,
						   Network*& network,
						   double& average_misguess,
						   double& misguess_standard_deviation) {
	vector<vector<double>> input_vals;
	vector<vector<bool>> input_is_on;
	vector<double> target_vals;
	int count = 0;
	uniform_int_distribution<int> action_distribution(0, 1);
	uniform_int_distribution<int> num_random_distribution(0, 99);
	while (true) {
		count++;

		Problem* problem = problem_type->get_problem();

		int num_random = num_random_distribution(generator);
		for (int r_index = 0; r_index < num_random; r_index++) {
			problem->perform_action(action_distribution(generator));
		}

		vector<double> t_scores(keypoints.size());
		vector<double> curr_input_vals(inputs.size());
		vector<bool> curr_input_is_on(inputs.size());
		for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
			problem->perform_action(actions[a_index]);

			for (int k_index = 0; k_index < (int)keypoints.size(); k_index++) {
				if (a_index == keypoints[k_index]) {
					vector<double> obs = problem->get_observations();
					double t_score = (obs[0] - keypoint_averages[k_index]) / keypoint_standard_deviations[k_index];
					t_scores[k_index] = t_score;
				}
			}

			for (int i_index = 0; i_index < (int)inputs.size(); i_index++) {
				if (a_index == inputs[i_index]) {
					vector<double> obs = problem->get_observations();
					curr_input_vals[i_index] = obs[0];
					curr_input_is_on[i_index] = true;
				}
			}
		}

		double target_val = problem->score_result();

		delete problem;

		double sum_t_scores = 0.0;
		for (int k_index = 0; k_index < (int)keypoints.size(); k_index++) {
			sum_t_scores += abs(t_scores[k_index]);
		}
		double average_t_score = sum_t_scores / (double)keypoints.size();
		if (average_t_score <= FACTOR_MAX_AVERAGE_T_SCORE) {
			input_vals.push_back(curr_input_vals);
			input_is_on.push_back(curr_input_is_on);

			target_vals.push_back(target_val);

			if (input_vals.size() >= MEASURE_ITERS) {
				break;
			}
		}
	}

	double hit_percentage = (double)MEASURE_ITERS / (double)count;
	cout << "hit_percentage: " << hit_percentage << endl;

	network = new Network((int)inputs.size(),
						  input_vals,
						  input_is_on);

	train_network(input_vals,
				  input_is_on,
				  target_vals,
				  network);	

	measure_network(input_vals,
					input_is_on,
					target_vals,
					network,
					average_misguess,
					misguess_standard_deviation);
}
