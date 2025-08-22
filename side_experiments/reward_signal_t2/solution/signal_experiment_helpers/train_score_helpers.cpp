#include "signal_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "signal_network.h"

using namespace std;

const int SCORE_NUM_INPUTS = 10;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

void SignalExperiment::train_score(vector<vector<vector<double>>>& positive_pre_obs_histories,
								   vector<vector<vector<double>>>& positive_post_obs_histories,
								   vector<double>& positive_target_val_histories,
								   vector<vector<vector<double>>>& pre_obs_histories,
								   vector<vector<vector<double>>>& post_obs_histories,
								   vector<double>& target_val_histories,
								   vector<bool>& new_score_input_is_pre,
								   vector<int>& new_score_input_indexes,
								   vector<int>& new_score_input_obs_indexes,
								   SignalNetwork*& new_score_network) {
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

	new_score_network = new SignalNetwork((int)new_score_input_is_pre.size());

	uniform_int_distribution<int> is_positive_distribution(0, 1);
	uniform_int_distribution<int> positive_distribution(0, positive_pre_obs_histories.size()-1);
	uniform_int_distribution<int> negative_distribution(0, pre_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		vector<double> inputs(new_score_input_is_pre.size());
		double target_val;

		bool is_positive = is_positive_distribution(generator) == 0;

		if (is_positive) {
			int h_index = positive_distribution(generator);
			for (int i_index = 0; i_index < (int)new_score_input_is_pre.size(); i_index++) {
				if (new_score_input_is_pre[i_index]) {
					inputs[i_index] = positive_pre_obs_histories[h_index][
						new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
				} else {
					inputs[i_index] = positive_post_obs_histories[h_index][
						new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
				}
			}
			target_val = positive_target_val_histories[h_index];
		} else {
			int h_index = negative_distribution(generator);
			for (int i_index = 0; i_index < (int)new_score_input_is_pre.size(); i_index++) {
				if (new_score_input_is_pre[i_index]) {
					inputs[i_index] = pre_obs_histories[h_index][
						new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
				} else {
					inputs[i_index] = post_obs_histories[h_index][
						new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
				}
			}
			target_val = target_val_histories[h_index];
		}

		new_score_network->activate(inputs);

		double error = target_val - new_score_network->output->acti_vals[0];

		new_score_network->backprop(error);
	}
}
