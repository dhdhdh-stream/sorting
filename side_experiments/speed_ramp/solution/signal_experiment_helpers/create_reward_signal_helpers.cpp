#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "signal.h"
#include "signal_network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SPLIT_NUM_TRIES = 4;
#else
const int SPLIT_NUM_TRIES = 10;
#endif /* MDEBUG */

const double MIN_MATCH_RATIO = 0.1;

const double CHECK_MIN_MATCH_RATIO = 0.05;

void SignalExperiment::create_reward_signal_helper(vector<vector<vector<double>>>& pre_obs,
												   vector<vector<vector<double>>>& post_obs,
												   vector<double>& scores,
												   DefaultSignal* default_signal,
												   vector<Signal*>& previous_signals,
												   vector<Signal*>& signals,
												   double& misguess_average) {
	vector<double> predicted(pre_obs.size());
	for (int h_index = 0; h_index < (int)pre_obs.size(); h_index++) {
		predicted[h_index] = default_signal->calc(
			pre_obs[h_index],
			post_obs[h_index]);
	}

	for (int s_index = previous_signals.size()-1; s_index >= 0; s_index--) {
		vector<vector<vector<double>>> match_pre_obs;
		vector<vector<vector<double>>> match_post_obs;
		vector<double> match_target_vals;
		vector<double> match_curr_predicted;
		vector<double> match_new_predicted;
		vector<int> match_index;

		for (int h_index = 0; h_index < (int)pre_obs.size(); h_index++) {
			bool is_match;
			double val;
			previous_signals[s_index]->calc(pre_obs[h_index],
											post_obs[h_index],
											is_match,
											val);
			if (is_match) {
				match_pre_obs.push_back(pre_obs[h_index]);
				match_post_obs.push_back(post_obs[h_index]);
				match_target_vals.push_back(scores[h_index]);
				match_curr_predicted.push_back(predicted[h_index]);
				match_new_predicted.push_back(val);
				match_index.push_back(h_index);
			}
		}

		double curr_sum_misguess = 0.0;
		for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
			curr_sum_misguess += (match_target_vals[m_index] - match_curr_predicted[m_index])
				* (match_target_vals[m_index] - match_curr_predicted[m_index]);
		}
		double curr_misguess_average = curr_sum_misguess / (double)match_pre_obs.size();

		double new_sum_misguess = 0.0;
		for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
			new_sum_misguess += (match_target_vals[m_index] - match_new_predicted[m_index])
				* (match_target_vals[m_index] - match_new_predicted[m_index]);
		}
		double new_misguess_average = new_sum_misguess / (double)match_pre_obs.size();

		#if defined(MDEBUG) && MDEBUG
		if (new_misguess_average < curr_misguess_average || rand()%2 == 0) {
		#else
		if (new_misguess_average < curr_misguess_average) {
		#endif /* MDEBUG */
			signals.insert(signals.begin(), previous_signals[s_index]);
			previous_signals.erase(previous_signals.begin() + s_index);

			for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
				predicted[match_index[m_index]] = match_new_predicted[m_index];
			}
		}
	}

	int num_min_match = MIN_MATCH_RATIO * (double)pre_obs.size();
	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<bool> new_match_input_is_pre;
		vector<int> new_match_input_indexes;
		vector<int> new_match_input_obs_indexes;
		SignalNetwork* new_match_network = NULL;
		bool split_is_success = split_helper(pre_obs,
											 post_obs,
											 new_match_input_is_pre,
											 new_match_input_indexes,
											 new_match_input_obs_indexes,
											 new_match_network);

		if (split_is_success) {
			vector<vector<vector<double>>> match_pre_obs;
			vector<vector<vector<double>>> match_post_obs;
			vector<double> match_target_vals;
			vector<double> match_curr_predicted;
			vector<int> match_index;

			for (int h_index = 0; h_index < (int)pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(pre_obs[h_index]);
					match_post_obs.push_back(post_obs[h_index]);
					match_target_vals.push_back(scores[h_index]);
					match_curr_predicted.push_back(predicted[h_index]);
					match_index.push_back(h_index);
				}
			}

			if ((int)match_target_vals.size() >= num_min_match) {
				vector<bool> new_score_input_is_pre;
				vector<int> new_score_input_indexes;
				vector<int> new_score_input_obs_indexes;
				SignalNetwork* new_score_network = NULL;
				train_score(match_pre_obs,
							match_post_obs,
							match_target_vals,
							new_score_input_is_pre,
							new_score_input_indexes,
							new_score_input_obs_indexes,
							new_score_network);

				vector<double> match_new_predicted(match_pre_obs.size());
				for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
					vector<double> input_vals(new_score_input_is_pre.size());
					for (int i_index = 0; i_index < (int)new_score_input_is_pre.size(); i_index++) {
						if (new_score_input_is_pre[i_index]) {
							input_vals[i_index] = match_pre_obs[m_index][
								new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
						} else {
							input_vals[i_index] = match_post_obs[m_index][
								new_score_input_indexes[i_index]][new_score_input_obs_indexes[i_index]];
						}
					}
					new_score_network->activate(input_vals);
					match_new_predicted[m_index] = new_score_network->output->acti_vals[0];
				}

				double curr_sum_misguess = 0.0;
				for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
					curr_sum_misguess += (match_target_vals[m_index] - match_curr_predicted[m_index])
						* (match_target_vals[m_index] - match_curr_predicted[m_index]);
				}
				double curr_misguess_average = curr_sum_misguess / (double)match_pre_obs.size();

				double curr_sum_misguess_variance = 0.0;
				for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
					double curr_misguess = (match_target_vals[m_index] - match_curr_predicted[m_index])
						* (match_target_vals[m_index] - match_curr_predicted[m_index]);
					curr_sum_misguess_variance += (curr_misguess - curr_misguess_average)
						* (curr_misguess - curr_misguess_average);
				}
				double curr_misguess_standard_deviation = sqrt(curr_sum_misguess_variance / (double)match_pre_obs.size());

				double new_sum_misguess = 0.0;
				for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
					new_sum_misguess += (match_target_vals[m_index] - match_new_predicted[m_index])
						* (match_target_vals[m_index] - match_new_predicted[m_index]);
				}
				double new_misguess_average = new_sum_misguess / (double)match_pre_obs.size();

				double new_sum_misguess_variance = 0.0;
				for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
					double new_misguess = (match_target_vals[m_index] - match_new_predicted[m_index])
						* (match_target_vals[m_index] - match_new_predicted[m_index]);
					new_sum_misguess_variance += (new_misguess - new_misguess_average)
						* (new_misguess - new_misguess_average);
				}
				double new_misguess_standard_deviation = sqrt(new_sum_misguess_variance / (double)match_pre_obs.size());

				double misguess_improvement = curr_misguess_average - new_misguess_average;
				double min_standard_deviation = min(curr_misguess_standard_deviation, new_misguess_standard_deviation);
				double t_score = misguess_improvement / (min_standard_deviation / sqrt((double)match_pre_obs.size()));

				#if defined(MDEBUG) && MDEBUG
				if (t_score >= 1.282 || rand()%2 == 0) {
				#else
				if (t_score >= 1.282) {
				#endif /* MDEBUG */
					Signal* new_signal = new Signal();
					new_signal->match_input_is_pre = new_match_input_is_pre;
					new_signal->match_input_indexes = new_match_input_indexes;
					new_signal->match_input_obs_indexes = new_match_input_obs_indexes;
					new_signal->match_network = new_match_network;
					new_match_network = NULL;
					new_signal->score_input_is_pre = new_score_input_is_pre;
					new_signal->score_input_indexes = new_score_input_indexes;
					new_signal->score_input_obs_indexes = new_score_input_obs_indexes;
					new_signal->score_network = new_score_network;
					new_score_network = NULL;
					signals.push_back(new_signal);

					for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
						predicted[match_index[m_index]] = match_new_predicted[m_index];
					}
				}

				if (new_score_network != NULL) {
					delete new_score_network;
				}
			}
		}

		if (new_match_network != NULL) {
			delete new_match_network;
		}
	}

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)pre_obs.size(); h_index++) {
		sum_misguess += (scores[h_index] - predicted[h_index])
			* (scores[h_index] - predicted[h_index]);
	}
	misguess_average = sum_misguess / (double)pre_obs.size();

	cout << "misguess_average: " << misguess_average << endl;
}
