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
#include "scope.h"
#include "signal_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SPLIT_NUM_TRIES = 3;
#else
const int SPLIT_NUM_TRIES = 10;
#endif /* MDEBUG */

const double MIN_MATCH_RATIO = 0.02;

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

void SignalExperiment::create_reward_signal_helper(SolutionWrapper* wrapper) {
	double curr_misguess_average;
	double curr_misguess_standard_deviation;
	if (this->scope_context->signals.size() == 0) {
		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
			sum_vals += this->target_val_histories[h_index];
		}
		double average_val = sum_vals / (double)this->target_val_histories.size();

		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
			sum_misguess += (this->target_val_histories[h_index] - average_val)
				* (this->target_val_histories[h_index] - average_val);
		}
		curr_misguess_average = sum_misguess / (double)this->target_val_histories.size();

		double sum_misguess_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
			double curr_misguess = (this->target_val_histories[h_index] - average_val)
				* (this->target_val_histories[h_index] - average_val);
			sum_misguess_variance += (curr_misguess - curr_misguess_average)
				* (curr_misguess - curr_misguess_average);
		}
		curr_misguess_standard_deviation = sqrt(sum_misguess_variance / (double)this->target_val_histories.size());
	} else {
		curr_misguess_average = this->scope_context->signal_misguess_average;
		curr_misguess_standard_deviation = this->scope_context->signal_misguess_standard_deviation;
	}

	bool is_success = false;
	int num_min_match = MIN_MATCH_RATIO * (double)this->pre_obs_histories.size();
	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<bool> new_match_input_is_pre;
		vector<int> new_match_input_indexes;
		vector<int> new_match_input_obs_indexes;
		SignalNetwork* new_match_network = NULL;
		bool split_is_success = split_helper(new_match_input_is_pre,
											 new_match_input_indexes,
											 new_match_input_obs_indexes,
											 new_match_network);

		if (split_is_success) {
			vector<vector<vector<double>>> positive_match_pre_obs;
			vector<vector<vector<double>>> positive_match_post_obs;
			vector<double> positive_match_target_vals;
			for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->positive_pre_obs_histories[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->positive_post_obs_histories[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					positive_match_pre_obs.push_back(this->positive_pre_obs_histories[h_index]);
					positive_match_post_obs.push_back(this->positive_post_obs_histories[h_index]);
					positive_match_target_vals.push_back(this->positive_target_val_histories[h_index]);
				}
			}

			vector<vector<vector<double>>> match_pre_obs;
			vector<vector<vector<double>>> match_post_obs;
			vector<double> match_target_vals;
			for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->pre_obs_histories[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->post_obs_histories[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(this->pre_obs_histories[h_index]);
					match_post_obs.push_back(this->post_obs_histories[h_index]);
					match_target_vals.push_back(this->target_val_histories[h_index]);
				}
			}

			if ((int)match_pre_obs.size() >= num_min_match) {
				vector<bool> new_score_input_is_pre;
				vector<int> new_score_input_indexes;
				vector<int> new_score_input_obs_indexes;
				SignalNetwork* new_score_network = NULL;
				train_score(positive_match_pre_obs,
							positive_match_post_obs,
							positive_match_target_vals,
							match_pre_obs,
							match_post_obs,
							match_target_vals,
							new_score_input_is_pre,
							new_score_input_indexes,
							new_score_input_obs_indexes,
							new_score_network);

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

				vector<Signal*> potential_signals = this->signals;
				potential_signals.push_back(new_signal);

				double potential_miss_average_guess = calc_miss_average_guess(
					this->pre_obs_histories,
					this->post_obs_histories,
					this->target_val_histories,
					potential_signals);

				vector<double> potential_vals(this->pre_obs_histories.size());
				for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
					potential_vals[h_index] = calc_signal(this->pre_obs_histories[h_index],
														  this->post_obs_histories[h_index],
														  potential_signals,
														  potential_miss_average_guess);
				}

				double sum_potential_misguess = 0.0;
				for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
					sum_potential_misguess += (this->target_val_histories[h_index] - potential_vals[h_index])
						* (this->target_val_histories[h_index] - potential_vals[h_index]);
				}
				double potential_misguess_average = sum_potential_misguess / (double)this->pre_obs_histories.size();

				double sum_potential_misguess_variance = 0.0;
				for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
					double curr_misguess = (this->target_val_histories[h_index] - potential_vals[h_index])
						* (this->target_val_histories[h_index] - potential_vals[h_index]);
					sum_potential_misguess_variance += (curr_misguess - potential_misguess_average)
						* (curr_misguess - potential_misguess_average);
				}
				double potential_misguess_standard_deviation = sqrt(sum_potential_misguess_variance / (double)this->pre_obs_histories.size());

				double misguess_improvement = curr_misguess_average - potential_misguess_average;
				double min_standard_deviation = min(curr_misguess_standard_deviation, potential_misguess_standard_deviation);
				double t_score = misguess_improvement / (min_standard_deviation / sqrt((double)this->pre_obs_histories.size()));

				// temp
				cout << "curr_misguess_average: " << curr_misguess_average << endl;
				cout << "potential_misguess_average: " << potential_misguess_average << endl;
				cout << "measure t_score: " << t_score << endl;

				#if defined(MDEBUG) && MDEBUG
				if (t_score >= 1.282 || rand()%2 == 0) {
				#else
				if (t_score >= 1.282) {
				#endif /* MDEBUG */
					this->signals = potential_signals;
					this->miss_average_guess = potential_miss_average_guess;

					curr_misguess_average = potential_misguess_average;
					curr_misguess_standard_deviation = potential_misguess_standard_deviation;

					is_success = true;
				} else {
					delete new_signal;
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

	if (is_success) {
		cout << "SignalExperiment success" << endl;

		this->scope_context->signal_pre_actions = this->pre_actions;
		this->scope_context->signal_post_actions = this->post_actions;
		for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
			delete this->scope_context->signals[s_index];
		}
		this->scope_context->signals = this->signals;
		this->signals.clear();
		this->scope_context->miss_average_guess = this->miss_average_guess;

		this->scope_context->signal_misguess_average = curr_misguess_average;
		this->scope_context->signal_misguess_standard_deviation = curr_misguess_standard_deviation;

		for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
			delete wrapper->solution->existing_scope_histories[h_index];
		}
		wrapper->solution->existing_scope_histories.clear();
		wrapper->solution->existing_target_val_histories.clear();

		wrapper->solution->existing_scope_histories = this->new_scope_histories;
		this->new_scope_histories.clear();
		wrapper->solution->existing_target_val_histories = this->new_target_val_histories;

		wrapper->solution->clean();

		if (wrapper->solution->existing_scope_histories.size() >= MEASURE_ITERS) {
			wrapper->solution->measure_update();
		}
	}
}
