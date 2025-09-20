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

DefaultSignal* SignalExperiment::train_existing_default() {
	DefaultSignal* default_signal = new DefaultSignal(this->scope_context->default_signal);

	vector<vector<vector<double>>> combined_pre_obs;
	combined_pre_obs.insert(combined_pre_obs.end(),
		this->existing_current_pre_obs.begin(), this->existing_current_pre_obs.end());
	combined_pre_obs.insert(combined_pre_obs.end(),
		this->existing_explore_pre_obs.begin(), this->existing_explore_pre_obs.end());
	vector<vector<vector<double>>> combined_post_obs;
	combined_post_obs.insert(combined_post_obs.end(),
		this->existing_current_post_obs.begin(), this->existing_current_post_obs.end());
	combined_post_obs.insert(combined_post_obs.end(),
		this->existing_explore_post_obs.begin(), this->existing_explore_post_obs.end());
	vector<double> combined_scores;
	combined_scores.insert(combined_scores.end(),
		this->existing_current_scores.begin(), this->existing_current_scores.end());
	combined_scores.insert(combined_scores.end(),
		this->existing_explore_scores.begin(), this->existing_explore_scores.end());

	uniform_int_distribution<int> random_distribution(0, combined_pre_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_NEW_ITERS; iter_index++) {
		int h_index = random_distribution(generator);

		vector<double> inputs(default_signal->score_input_is_pre.size());
		for (int i_index = 0; i_index < (int)default_signal->score_input_is_pre.size(); i_index++) {
			if (default_signal->score_input_is_pre[i_index]) {
				inputs[i_index] = combined_pre_obs[h_index][
					default_signal->score_input_indexes[i_index]][default_signal->score_input_obs_indexes[i_index]];
			} else {
				inputs[i_index] = combined_post_obs[h_index][
					default_signal->score_input_indexes[i_index]][default_signal->score_input_obs_indexes[i_index]];
			}
		}

		default_signal->score_network->activate(inputs);

		double error = combined_scores[h_index] - default_signal->score_network->output->acti_vals[0];

		default_signal->score_network->backprop(error);
	}

	return default_signal;
}

DefaultSignal* SignalExperiment::train_new_default() {
	vector<vector<vector<double>>> combined_pre_obs;
	combined_pre_obs.insert(combined_pre_obs.end(),
		this->new_current_pre_obs.begin(), this->new_current_pre_obs.end());
	combined_pre_obs.insert(combined_pre_obs.end(),
		this->new_explore_pre_obs.begin(), this->new_explore_pre_obs.end());
	vector<vector<vector<double>>> combined_post_obs;
	combined_post_obs.insert(combined_post_obs.end(),
		this->new_current_post_obs.begin(), this->new_current_post_obs.end());
	combined_post_obs.insert(combined_post_obs.end(),
		this->new_explore_post_obs.begin(), this->new_explore_post_obs.end());
	vector<double> combined_scores;
	combined_scores.insert(combined_scores.end(),
		this->new_current_scores.begin(), this->new_current_scores.end());
	combined_scores.insert(combined_scores.end(),
		this->new_explore_scores.begin(), this->new_explore_scores.end());

	vector<bool> new_score_input_is_pre;
	vector<int> new_score_input_indexes;
	vector<int> new_score_input_obs_indexes;
	SignalNetwork* new_score_network;
	train_score(combined_pre_obs,
				combined_post_obs,
				combined_scores,
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
