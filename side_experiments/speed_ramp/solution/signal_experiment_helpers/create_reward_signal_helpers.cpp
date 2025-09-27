#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "signal.h"
#include "signal_network.h"

using namespace std;

const int MATCH_TYPE_CURRENT = 0;
const int MATCH_TYPE_EXPLORE = 1;

#if defined(MDEBUG) && MDEBUG
const int SPLIT_NUM_TRIES = 4;
#else
const int SPLIT_NUM_TRIES = 20;
#endif /* MDEBUG */

const double MIN_MATCH_RATIO = 0.04;

const double CHECK_MIN_MATCH_RATIO = 0.02;

void SignalExperiment::create_reward_signal_helper(vector<vector<vector<double>>>& current_pre_obs,
												   vector<vector<vector<double>>>& current_post_obs,
												   vector<double>& current_scores,
												   vector<vector<vector<double>>>& explore_pre_obs,
												   vector<vector<vector<double>>>& explore_post_obs,
												   vector<double>& explore_scores,
												   DefaultSignal* default_signal,
												   vector<Signal*>& previous_signals,
												   vector<Signal*>& signals,
												   double& misguess_average,
												   bool& better_than_default) {
	vector<double> current_predicted(current_pre_obs.size());
	for (int h_index = 0; h_index < (int)current_pre_obs.size(); h_index++) {
		current_predicted[h_index] = default_signal->calc(
			current_pre_obs[h_index],
			current_post_obs[h_index]);
	}

	vector<double> explore_predicted(explore_pre_obs.size());
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		explore_predicted[h_index] = default_signal->calc(
			explore_pre_obs[h_index],
			explore_post_obs[h_index]);
	}

	for (int s_index = previous_signals.size()-1; s_index >= 0; s_index--) {
		vector<vector<vector<double>>> match_pre_obs;
		vector<vector<vector<double>>> match_post_obs;
		vector<double> match_target_vals;
		vector<double> match_curr_predicted;
		vector<double> match_new_predicted;
		vector<int> match_type;
		vector<int> match_index;

		for (int h_index = 0; h_index < (int)current_pre_obs.size(); h_index++) {
			bool is_match;
			double val;
			previous_signals[s_index]->calc(current_pre_obs[h_index],
											current_post_obs[h_index],
											is_match,
											val);
			if (is_match) {
				match_pre_obs.push_back(current_pre_obs[h_index]);
				match_post_obs.push_back(current_post_obs[h_index]);
				match_target_vals.push_back(current_scores[h_index]);
				match_curr_predicted.push_back(current_predicted[h_index]);
				match_new_predicted.push_back(val);
				match_type.push_back(MATCH_TYPE_CURRENT);
				match_index.push_back(h_index);
			}
		}

		for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
			bool is_match;
			double val;
			previous_signals[s_index]->calc(explore_pre_obs[h_index],
											explore_post_obs[h_index],
											is_match,
											val);
			if (is_match) {
				match_pre_obs.push_back(explore_pre_obs[h_index]);
				match_post_obs.push_back(explore_post_obs[h_index]);
				match_target_vals.push_back(explore_scores[h_index]);
				match_curr_predicted.push_back(explore_predicted[h_index]);
				match_new_predicted.push_back(val);
				match_type.push_back(MATCH_TYPE_EXPLORE);
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
				switch (match_type[m_index]) {
				case MATCH_TYPE_CURRENT:
					current_predicted[match_index[m_index]] = match_new_predicted[m_index];
					break;
				case MATCH_TYPE_EXPLORE:
					explore_predicted[match_index[m_index]] = match_new_predicted[m_index];
					break;
				}
			}
		}
	}

	int num_min_match = MIN_MATCH_RATIO * (double)(current_pre_obs.size() + explore_pre_obs.size());
	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<bool> new_match_input_is_pre;
		vector<int> new_match_input_indexes;
		vector<int> new_match_input_obs_indexes;
		SignalNetwork* new_match_network = NULL;
		bool split_is_success = split_helper(current_pre_obs,
											 current_post_obs,
											 explore_pre_obs,
											 explore_post_obs,
											 new_match_input_is_pre,
											 new_match_input_indexes,
											 new_match_input_obs_indexes,
											 new_match_network);

		if (split_is_success) {
			vector<vector<vector<double>>> match_pre_obs;
			vector<vector<vector<double>>> match_post_obs;
			vector<double> match_target_vals;
			vector<double> match_curr_predicted;
			vector<int> match_type;
			vector<int> match_index;

			for (int h_index = 0; h_index < (int)current_pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = current_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = current_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(current_pre_obs[h_index]);
					match_post_obs.push_back(current_post_obs[h_index]);
					match_target_vals.push_back(current_scores[h_index]);
					match_curr_predicted.push_back(current_predicted[h_index]);
					match_type.push_back(MATCH_TYPE_CURRENT);
					match_index.push_back(h_index);
				}
			}

			for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = explore_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = explore_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(explore_pre_obs[h_index]);
					match_post_obs.push_back(explore_post_obs[h_index]);
					match_target_vals.push_back(explore_scores[h_index]);
					match_curr_predicted.push_back(explore_predicted[h_index]);
					match_type.push_back(MATCH_TYPE_EXPLORE);
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
						switch (match_type[m_index]) {
						case MATCH_TYPE_CURRENT:
							current_predicted[match_index[m_index]] = match_new_predicted[m_index];
							break;
						case MATCH_TYPE_EXPLORE:
							explore_predicted[match_index[m_index]] = match_new_predicted[m_index];
							break;
						}
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
	for (int h_index = 0; h_index < (int)current_pre_obs.size(); h_index++) {
		sum_misguess += (current_scores[h_index] - current_predicted[h_index])
			* (current_scores[h_index] - current_predicted[h_index]);
	}
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		sum_misguess += (explore_scores[h_index] - explore_predicted[h_index])
			* (explore_scores[h_index] - explore_predicted[h_index]);
	}
	misguess_average = sum_misguess / (double)(current_pre_obs.size() + explore_pre_obs.size());

	cout << "misguess_average: " << misguess_average << endl;

	double sum_explore_scores = 0.0;
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		sum_explore_scores += explore_scores[h_index];
	}
	double explore_score_average = sum_explore_scores / (double)explore_pre_obs.size();

	double sum_default_misguess = 0.0;
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		sum_default_misguess += (explore_scores[h_index] - explore_score_average)
			* (explore_scores[h_index] - explore_score_average);
	}
	double default_misguess_average = sum_default_misguess / (double)explore_pre_obs.size();

	double sum_default_misguess_variance = 0.0;
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		double curr_misguess = (explore_scores[h_index] - explore_score_average)
			* (explore_scores[h_index] - explore_score_average);
		sum_default_misguess_variance += (curr_misguess - default_misguess_average)
			* (curr_misguess - default_misguess_average);
	}
	double default_misguess_standard_deviation = sqrt(sum_default_misguess_variance / (double)explore_pre_obs.size());

	double sum_explore_misguess = 0.0;
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		sum_explore_misguess += (explore_scores[h_index] - explore_predicted[h_index])
			* (explore_scores[h_index] - explore_predicted[h_index]);
	}
	double explore_misguess_average = sum_explore_misguess / (double)explore_pre_obs.size();

	double sum_explore_misguess_variance = 0.0;
	for (int h_index = 0; h_index < (int)explore_pre_obs.size(); h_index++) {
		double curr_misguess = (explore_scores[h_index] - explore_predicted[h_index])
			* (explore_scores[h_index] - explore_predicted[h_index]);
		sum_explore_misguess_variance += (curr_misguess - explore_misguess_average)
			* (curr_misguess - explore_misguess_average);
	}
	double explore_misguess_standard_deviation = sqrt(sum_explore_misguess_variance / (double)explore_pre_obs.size());

	double signal_improvement = default_misguess_average - explore_misguess_average;
	double min_standard_deviation = min(default_misguess_standard_deviation, explore_misguess_standard_deviation);
	double signal_improvement_t_score = signal_improvement / (min_standard_deviation / sqrt((double)explore_pre_obs.size()));

	cout << "default_misguess_average: " << default_misguess_average << endl;
	cout << "explore_misguess_average: " << explore_misguess_average << endl;
	cout << "signal_improvement_t_score: " << signal_improvement_t_score << endl;

	if (signal_improvement_t_score >= 2.326) {
		better_than_default = true;
	} else {
		better_than_default = false;
	}
}
