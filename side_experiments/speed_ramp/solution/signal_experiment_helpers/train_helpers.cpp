#include "signal_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "factor.h"
#include "globals.h"
#include "scope.h"
#include "signal_network.h"

using namespace std;

const int SCORE_NUM_INPUTS = 10;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_ITERS = 10;
const int TRAIN_NEW_ITERS = 30;
#else
const int TRAIN_EXISTING_ITERS = 100000;
const int TRAIN_NEW_ITERS = 300000;
#endif /* MDEBUG */

DefaultSignal* SignalExperiment::train_existing_default(bool is_pre) {
	DefaultSignal* default_signal;
	if (is_pre) {
		default_signal = new DefaultSignal(this->scope_context->pre_default_signal);
	} else {
		default_signal = new DefaultSignal(this->scope_context->post_default_signal);
	}

	uniform_int_distribution<int> random_distribution(0, this->existing_explore_pre_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_EXISTING_ITERS; iter_index++) {
		int h_index = random_distribution(generator);

		vector<double> inputs(default_signal->score_input_is_pre.size());
		for (int i_index = 0; i_index < (int)default_signal->score_input_is_pre.size(); i_index++) {
			if (default_signal->score_input_is_pre[i_index]) {
				inputs[i_index] = this->existing_explore_pre_obs[h_index][
					default_signal->score_input_indexes[i_index]][default_signal->score_input_obs_indexes[i_index]];
			} else {
				inputs[i_index] = this->existing_explore_post_obs[h_index][
					default_signal->score_input_indexes[i_index]][default_signal->score_input_obs_indexes[i_index]];
			}
		}

		default_signal->score_network->activate(inputs);

		double error = this->existing_explore_scores[h_index] - default_signal->score_network->output->acti_vals[0];

		default_signal->score_network->backprop(error);
	}

	return default_signal;
}

DefaultSignal* SignalExperiment::train_new_default(bool is_pre) {
	vector<bool> new_score_input_is_pre;
	vector<int> new_score_input_indexes;
	vector<int> new_score_input_obs_indexes;
	SignalNetwork* new_score_network;
	train_score(this->new_explore_pre_obs,
				this->new_explore_post_obs,
				this->new_explore_scores,
				is_pre,
				new_score_input_is_pre,
				new_score_input_indexes,
				new_score_input_obs_indexes,
				new_score_network);

	DefaultSignal* default_signal = new DefaultSignal();
	default_signal->score_input_is_pre = new_score_input_is_pre;
	default_signal->score_input_indexes = new_score_input_indexes;
	default_signal->score_input_obs_indexes = new_score_input_obs_indexes;
	default_signal->score_network = new_score_network;

	return default_signal;
}

void SignalExperiment::train_score(vector<vector<vector<double>>>& pre_obs,
								   vector<vector<vector<double>>>& post_obs,
								   vector<double>& scores,
								   bool is_pre,
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
	if (!is_pre) {
		for (int i_index = 0; i_index < (int)post_obs[0].size(); i_index++) {
			for (int o_index = 0; o_index < (int)post_obs[0][i_index].size(); o_index++) {
				possible_inputs.push_back({false, {i_index, o_index}});
			}
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
	for (int iter_index = 0; iter_index < TRAIN_NEW_ITERS; iter_index++) {
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
