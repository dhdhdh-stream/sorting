/**
 * - reward signal is as much about preserving existing, as making improvements
 *   - as signal only valid under conditions
 * 
 * - there can be multiple options for "consistency" that make sense
 *   - e.g., train at a train station
 *     - could be correct to follow train or follow train station
 * - the option that should be chosen is the one that produces the best signal
 */

#include "signal_experiment.h"

#include <cmath>
#include <iostream>
#include <limits>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SPLIT_NUM_TRIES = 2;
#else
const int SPLIT_NUM_TRIES = 20;
#endif /* MDEBUG */

double calc_miss_average_guess(vector<vector<vector<double>>>& pre_obs_histories,
							   vector<vector<vector<double>>>& post_obs_histories,
							   vector<double>& target_val_histories,
							   vector<Signal*>& potential_signals) {
	double sum_target_vals = 0.0;
	int sum_count = 0;
	for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
		bool has_match = false;
		for (int s_index = 0; s_index < (int)potential_signals.size(); s_index++) {
			vector<double> input_vals(potential_signals[s_index]->match_input_is_pre.size());
			for (int i_index = 0; i_index < (int)potential_signals[s_index]->match_input_is_pre.size(); i_index++) {
				if (potential_signals[s_index]->match_input_is_pre[i_index]) {
					input_vals[i_index] = pre_obs_histories[h_index][
						potential_signals[s_index]->match_input_indexes[i_index]][potential_signals[s_index]->match_input_obs_indexes[i_index]];
				} else {
					input_vals[i_index] = post_obs_histories[h_index][
						potential_signals[s_index]->match_input_indexes[i_index]][potential_signals[s_index]->match_input_obs_indexes[i_index]];
				}
			}
			potential_signals[s_index]->match_network->activate(input_vals);
			#if defined(MDEBUG) && MDEBUG
			if (rand()%3 == 0) {
			#else
			if (potential_signals[s_index]->match_network->output->acti_vals[0] >= MATCH_WEIGHT) {
			#endif /* MDEBUG */
				has_match = true;
				break;
			}
		}

		if (!has_match) {
			sum_target_vals += target_val_histories[h_index];
			sum_count++;
		}
	}

	if (sum_count > 0) {
		return sum_target_vals / (double)sum_count;
	} else {
		return 0.0;
	}
}

void SignalExperiment::create_reward_signal_helper(
		vector<vector<vector<double>>>& pre_obs_histories,
		vector<vector<vector<double>>>& post_obs_histories,
		vector<double>& target_val_histories,
		vector<Signal*>& signals,
		double& miss_average_guess) {
	cout << "create_reward_signal_helper" << endl;

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)target_val_histories.size(); h_index++) {
		sum_vals += target_val_histories[h_index];
	}
	double average_val = sum_vals / (double)target_val_histories.size();

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)target_val_histories.size(); h_index++) {
		sum_misguess += (target_val_histories[h_index] - average_val)
			* (target_val_histories[h_index] - average_val);
	}
	double curr_misguess_average = sum_misguess / (double)target_val_histories.size();

	double sum_misguess_variance = 0.0;
	for (int h_index = 0; h_index < (int)target_val_histories.size(); h_index++) {
		double curr_misguess = (target_val_histories[h_index] - average_val)
			* (target_val_histories[h_index] - average_val);
		sum_misguess_variance += (curr_misguess - curr_misguess_average)
			* (curr_misguess - curr_misguess_average);
	}
	double curr_misguess_standard_deviation = sqrt(sum_misguess_variance / (double)target_val_histories.size());

	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<bool> new_match_input_is_pre;
		vector<int> new_match_input_indexes;
		vector<int> new_match_input_obs_indexes;
		Network* new_match_network = NULL;
		bool split_is_success = split_helper(pre_obs_histories,
											 post_obs_histories,
											 new_match_input_is_pre,
											 new_match_input_indexes,
											 new_match_input_obs_indexes,
											 new_match_network);

		if (split_is_success) {
			vector<vector<vector<double>>> match_pre_obs;
			vector<vector<vector<double>>> match_post_obs;
			vector<double> match_target_vals;
			for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = pre_obs_histories[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = post_obs_histories[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(pre_obs_histories[h_index]);
					match_post_obs.push_back(post_obs_histories[h_index]);
					match_target_vals.push_back(target_val_histories[h_index]);
				}
			}

			vector<bool> new_score_input_is_pre;
			vector<int> new_score_input_indexes;
			vector<int> new_score_input_obs_indexes;
			Network* new_score_network = NULL;
			bool is_success = train_score(match_pre_obs,
										  match_post_obs,
										  match_target_vals,
										  new_score_input_is_pre,
										  new_score_input_indexes,
										  new_score_input_obs_indexes,
										  new_score_network);
			if (is_success) {
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

				vector<Signal*> potential_signals = signals;
				potential_signals.push_back(new_signal);

				double potential_miss_average_guess = calc_miss_average_guess(
					pre_obs_histories,
					post_obs_histories,
					target_val_histories,
					potential_signals);

				vector<double> potential_vals(pre_obs_histories.size());
				for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
					potential_vals[h_index] = calc_signal(pre_obs_histories[h_index],
														  post_obs_histories[h_index],
														  potential_signals,
														  potential_miss_average_guess);
				}

				double sum_potential_misguess = 0.0;
				for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
					sum_potential_misguess += (target_val_histories[h_index] - potential_vals[h_index])
						* (target_val_histories[h_index] - potential_vals[h_index]);
				}
				double potential_misguess_average = sum_potential_misguess / (double)pre_obs_histories.size();

				double sum_potential_misguess_variance = 0.0;
				for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
					double curr_misguess = (target_val_histories[h_index] - potential_vals[h_index])
						* (target_val_histories[h_index] - potential_vals[h_index]);
					sum_potential_misguess_variance += (curr_misguess - potential_misguess_average)
						* (curr_misguess - potential_misguess_average);
				}
				double potential_misguess_standard_deviation = sqrt(sum_potential_misguess_variance / (double)pre_obs_histories.size());

				double misguess_improvement = curr_misguess_average - potential_misguess_average;
				double min_standard_deviation = min(curr_misguess_standard_deviation, potential_misguess_standard_deviation);
				double t_score = misguess_improvement / (min_standard_deviation / sqrt((double)pre_obs_histories.size()));

				cout << "measure t_score: " << t_score << endl;

				#if defined(MDEBUG) && MDEBUG
				if (t_score >= 1.282 || rand()%2 == 0) {
				#else
				if (t_score >= 1.282) {
				#endif /* MDEBUG */
					signals = potential_signals;
					miss_average_guess = potential_miss_average_guess;

					curr_misguess_average = potential_misguess_average;
					curr_misguess_standard_deviation = potential_misguess_standard_deviation;
				} else {
					delete new_signal;
				}
			}

			if (new_score_network != NULL) {
				delete new_score_network;
			}
		}

		if (new_match_network != NULL) {
			delete new_match_network;
		}
	}
}
