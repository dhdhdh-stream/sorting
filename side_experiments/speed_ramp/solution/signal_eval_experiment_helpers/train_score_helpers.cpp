#include "signal_eval_experiment.h"

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

void SignalEvalExperiment::train_score(vector<vector<vector<double>>>& pre_obs,
									   vector<vector<vector<double>>>& post_obs,
									   vector<double>& scores,
									   vector<bool>& new_score_input_is_pre,
									   vector<int>& new_score_input_indexes,
									   vector<int>& new_score_input_obs_indexes,
									   SignalNetwork*& new_score_network) {
	vector<pair<bool,pair<int,int>>> possible_inputs;
	for (int i_index = 0; i_index < (int)pre_obs[0].size(); i_index++) {
		for (int o_index = 0; o_index < (int)pre_obs[0][i_index].size(); o_index++) {
			possible_inputs.push_back({true, {i_index, o_index}});
		}
	}
	for (int i_index = 0; i_index < (int)post_obs[0].size(); i_index++) {
		for (int o_index = 0; o_index < (int)post_obs[0][i_index].size(); o_index++) {
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

	uniform_int_distribution<int> random_distribution(0, pre_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int h_index = random_distribution(generator);

		vector<double> inputs(new_score_input_is_pre.size());
		for (int i_index = 0; i_index < (int)new_score_input_is_pre.size(); i_index++) {
			if (new_score_input_is_pre[i_index]) {
				inputs[i_index] = pre_obs[h_index][
					new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
			} else {
				inputs[i_index] = post_obs[h_index][
					new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
			}
		}

		new_score_network->activate(inputs);

		double error = scores[h_index] - new_score_network->output->acti_vals[0];

		new_score_network->backprop(error);
	}
}
