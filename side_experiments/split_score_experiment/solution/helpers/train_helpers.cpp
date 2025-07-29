#include "helpers.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

const double SEED_RATIO = 0.1;

const int MAX_EPOCHS = 30;
const int ITERS_PER_EPOCH = 10000;
const double MAX_AVERAGE_ERROR = 0.1;

const int TRAIN_ITERS = 300000;

bool split_helper(vector<vector<double>>& existing_vals,
				  vector<vector<double>>& explore_vals,
				  Network* match_network) {
	int num_seeds = SEED_RATIO * (double)existing_vals.size();

	vector<int> positive_seeds;
	{
		vector<int> initial_possible_indexes(existing_vals.size());
		for (int i_index = 0; i_index < (int)existing_vals.size(); i_index++) {
			initial_possible_indexes[i_index] = i_index;
		}
		for (int s_index = 0; s_index < num_seeds; s_index++) {
			uniform_int_distribution<int> possible_distribution(0, initial_possible_indexes.size()-1);
			int random_index = possible_distribution(generator);
			positive_seeds.push_back(initial_possible_indexes[random_index]);
			initial_possible_indexes.erase(initial_possible_indexes.begin() + random_index);
		}
	}

	vector<int> negative_seeds;
	{
		vector<int> initial_possible_indexes(explore_vals.size());
		for (int i_index = 0; i_index < (int)explore_vals.size(); i_index++) {
			initial_possible_indexes[i_index] = i_index;
		}
		for (int s_index = 0; s_index < num_seeds; s_index++) {
			uniform_int_distribution<int> possible_distribution(0, initial_possible_indexes.size()-1);
			int random_index = possible_distribution(generator);
			negative_seeds.push_back(initial_possible_indexes[random_index]);
			initial_possible_indexes.erase(initial_possible_indexes.begin() + random_index);
		}
	}

	uniform_int_distribution<int> is_positive_distribution(0, 1);
	uniform_int_distribution<int> seed_distribution(0, num_seeds-1);
	int e_index = 0;
	while (true) {
		double sum_errors = 0.0;
		for (int iter_index = 0; iter_index < ITERS_PER_EPOCH; iter_index++) {
			bool is_positive = is_positive_distribution(generator) == 0;

			vector<double> inputs;
			if (is_positive) {
				int random_index = seed_distribution(generator);
				int existing_index = positive_seeds[random_index];
				inputs = existing_vals[existing_index];
			} else {
				int random_index = seed_distribution(generator);
				int explore_index = negative_seeds[random_index];
				inputs = explore_vals[explore_index];
			}

			match_network->activate(inputs);

			double error;
			if (is_positive) {
				if (match_network->output->acti_vals[0] < 1.0) {
					error = 1.0 - match_network->output->acti_vals[0];
				}
			} else {
				if (match_network->output->acti_vals[0] > -1.0) {
					error = -1.0 - match_network->output->acti_vals[0];
				}
			}

			match_network->backprop(error);

			sum_errors += abs(error);
		}

		double average_error = sum_errors / ITERS_PER_EPOCH;
		if (average_error <= MAX_AVERAGE_ERROR) {
			return true;
		}

		e_index++;
		if (e_index >= MAX_EPOCHS) {
			return false;
		}

		vector<pair<double,int>> existing_acti_vals(existing_vals.size());
		for (int h_index = 0; h_index < (int)existing_vals.size(); h_index++) {
			match_network->activate(existing_vals[h_index]);

			existing_acti_vals[h_index] = {match_network->output->acti_vals[0], h_index};
		}
		sort(existing_acti_vals.begin(), existing_acti_vals.end());
		for (int s_index = 0; s_index < num_seeds; s_index++) {
			positive_seeds[s_index] = existing_acti_vals[existing_acti_vals.size() - 1 - s_index].second;
		}

		vector<pair<double,int>> explore_acti_vals(explore_vals.size());
		for (int h_index = 0; h_index < (int)explore_vals.size(); h_index++) {
			match_network->activate(explore_vals[h_index]);

			explore_acti_vals[h_index] = {match_network->output->acti_vals[0], h_index};
		}
		sort(explore_acti_vals.begin(), explore_acti_vals.end());
		for (int s_index = 0; s_index < num_seeds; s_index++) {
			negative_seeds[s_index] = explore_acti_vals[s_index].second;
		}
	}

	return true;
}

bool train_score(vector<vector<double>>& vals,
				 vector<double>& target_vals,
				 Network* network) {
	uniform_int_distribution<int> distribution(0, vals.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = distribution(generator);
		network->activate(vals[rand_index]);

		double error = target_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}

	vector<double> network_vals(vals.size());
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		network->activate(vals[i_index]);
		network_vals[i_index] = network->output->acti_vals[0];
	}

	double sum_target_vals = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		sum_target_vals += target_vals[i_index];
	}
	double average_target_val = sum_target_vals / (double)vals.size();

	double sum_base_misguess = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		sum_base_misguess += (target_vals[i_index] - average_target_val)
			* (target_vals[i_index] - average_target_val);
	}
	double average_base_misguess = sum_base_misguess / (double)vals.size();

	double sum_base_misguess_variance = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		double curr_misguess = (target_vals[i_index] - average_target_val)
			* (target_vals[i_index] - average_target_val);
		sum_base_misguess_variance += (curr_misguess - average_base_misguess) * (curr_misguess - average_base_misguess);
	}
	double base_misguess_standard_deviation = sqrt(sum_base_misguess_variance / (double)vals.size());
	if (base_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		base_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	double sum_signal_misguess = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		sum_signal_misguess += (target_vals[i_index] - network_vals[i_index])
			* (target_vals[i_index] - network_vals[i_index]);
	}
	double average_signal_misguess = sum_signal_misguess / (double)vals.size();

	double sum_signal_misguess_variance = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		double curr_misguess = (target_vals[i_index] - network_vals[i_index])
			* (target_vals[i_index] - network_vals[i_index]);
		sum_signal_misguess_variance += (curr_misguess - average_signal_misguess) * (curr_misguess - average_signal_misguess);
	}
	double signal_misguess_standard_deviation = sqrt(sum_signal_misguess_variance / (double)vals.size());
	if (signal_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		signal_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	double signal_improvement = average_base_misguess - average_signal_misguess;
	double min_standard_deviation = min(base_misguess_standard_deviation, signal_misguess_standard_deviation);
	double t_score = signal_improvement / (min_standard_deviation / sqrt((double)vals.size()));

	cout << "t_score: " << t_score << endl;

	if (t_score < 2.326) {
		return false;
	}

	return true;
}
