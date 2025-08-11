#include "signal_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"

using namespace std;

const int SCORE_NUM_INPUTS = 10;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

bool SignalExperiment::train_score(vector<vector<vector<double>>>& pre_obs_histories,
								   vector<vector<vector<double>>>& post_obs_histories,
								   vector<double>& target_val_histories,
								   vector<bool>& new_score_input_is_pre,
								   vector<int>& new_score_input_indexes,
								   vector<int>& new_score_input_obs_indexes,
								   Network*& new_score_network) {
	vector<pair<bool,pair<int,int>>> possible_inputs;
	for (int i_index = 0; i_index < (int)pre_obs_histories[0].size(); i_index++) {
		for (int o_index = 0; o_index < (int)pre_obs_histories[0][i_index].size(); o_index++) {
			possible_inputs.push_back({true, {i_index, o_index}});
		}
	}
	for (int i_index = 0; i_index < (int)post_obs_histories[0].size(); i_index++) {
		for (int o_index = 0; o_index < (int)post_obs_histories[0][i_index].size(); o_index++) {
			possible_inputs.push_back({false, {i_index, o_index}});
		}
	}

	while (possible_inputs.size() > 0) {
		uniform_int_distribution<int> input_distribution(0, possible_inputs.size()-1);
		int input_index = input_distribution(generator);

		new_score_input_is_pre.push_back(possible_inputs[input_index].first);
		new_score_input_indexes.push_back(possible_inputs[input_index].second.first);
		new_score_input_obs_indexes.push_back(possible_inputs[input_index].second.second);

		if (new_score_input_is_pre.size() >= SCORE_NUM_INPUTS) {
			break;
		}
	}

	vector<double> input_averages(new_score_input_is_pre.size(), 0.0);
	vector<double> input_standard_deviations(new_score_input_is_pre.size(), 0.0);
	/**
	 * -unused
	 */
	new_score_network = new Network((int)new_score_input_is_pre.size(),
									input_averages,
									input_standard_deviations);

	uniform_int_distribution<int> distribution(0, pre_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = distribution(generator);

		vector<double> inputs(new_score_input_is_pre.size());
		for (int i_index = 0; i_index < (int)new_score_input_is_pre.size(); i_index++) {
			if (new_score_input_is_pre[i_index]) {
				inputs[i_index] = pre_obs_histories[rand_index][
					new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
			} else {
				inputs[i_index] = post_obs_histories[rand_index][
					new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
			}
		}

		new_score_network->activate(inputs);

		double error = target_val_histories[rand_index] - new_score_network->output->acti_vals[0];

		new_score_network->backprop(error);
	}

	vector<double> network_vals(pre_obs_histories.size());
	for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
		vector<double> inputs(new_score_input_is_pre.size());
		for (int i_index = 0; i_index < (int)new_score_input_is_pre.size(); i_index++) {
			if (new_score_input_is_pre[i_index]) {
				inputs[i_index] = pre_obs_histories[h_index][
					new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
			} else {
				inputs[i_index] = post_obs_histories[h_index][
					new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
			}
		}

		new_score_network->activate(inputs);

		network_vals[h_index] = new_score_network->output->acti_vals[0];
	}

	double sum_target_vals = 0.0;
	for (int i_index = 0; i_index < (int)target_val_histories.size(); i_index++) {
		sum_target_vals += target_val_histories[i_index];
	}
	double average_target_val = sum_target_vals / (double)target_val_histories.size();

	double sum_base_misguess = 0.0;
	for (int i_index = 0; i_index < (int)target_val_histories.size(); i_index++) {
		sum_base_misguess += (target_val_histories[i_index] - average_target_val)
			* (target_val_histories[i_index] - average_target_val);
	}
	double average_base_misguess = sum_base_misguess / (double)target_val_histories.size();

	double sum_base_misguess_variance = 0.0;
	for (int i_index = 0; i_index < (int)target_val_histories.size(); i_index++) {
		double curr_misguess = (target_val_histories[i_index] - average_target_val)
			* (target_val_histories[i_index] - average_target_val);
		sum_base_misguess_variance += (curr_misguess - average_base_misguess) * (curr_misguess - average_base_misguess);
	}
	double base_misguess_standard_deviation = sqrt(sum_base_misguess_variance / (double)target_val_histories.size());
	if (base_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		base_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	double sum_signal_misguess = 0.0;
	for (int i_index = 0; i_index < (int)target_val_histories.size(); i_index++) {
		sum_signal_misguess += (target_val_histories[i_index] - network_vals[i_index])
			* (target_val_histories[i_index] - network_vals[i_index]);
	}
	double average_signal_misguess = sum_signal_misguess / (double)target_val_histories.size();

	double sum_signal_misguess_variance = 0.0;
	for (int i_index = 0; i_index < (int)target_val_histories.size(); i_index++) {
		double curr_misguess = (target_val_histories[i_index] - network_vals[i_index])
			* (target_val_histories[i_index] - network_vals[i_index]);
		sum_signal_misguess_variance += (curr_misguess - average_signal_misguess) * (curr_misguess - average_signal_misguess);
	}
	double signal_misguess_standard_deviation = sqrt(sum_signal_misguess_variance / (double)target_val_histories.size());
	if (signal_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		signal_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	double signal_improvement = average_base_misguess - average_signal_misguess;
	double min_standard_deviation = min(base_misguess_standard_deviation, signal_misguess_standard_deviation);
	double t_score = signal_improvement / (min_standard_deviation / sqrt((double)target_val_histories.size()));

	#if defined(MDEBUG) && MDEBUG
	if (t_score < 2.326 || rand()%2 == 0) {
	#else
	if (t_score < 2.326) {
	#endif /* MDEBUG */
		return false;
	}

	return true;
}
