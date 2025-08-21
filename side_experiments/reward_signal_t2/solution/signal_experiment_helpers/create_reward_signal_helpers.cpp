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

const double CHECK_MIN_MATCH_RATIO = 0.01;

bool check_signal_still_good(vector<vector<vector<double>>>& pre_obs_histories,
							 vector<vector<vector<double>>>& post_obs_histories,
							 vector<double>& target_val_histories,
							 Signal* signal) {
	vector<double> match_target_vals;
	vector<double> match_predicted_vals;
	for (int h_index = 0; h_index < (int)pre_obs_histories.size(); h_index++) {
		vector<double> input_vals(signal->match_input_is_pre.size());
		for (int i_index = 0; i_index < (int)signal->match_input_is_pre.size(); i_index++) {
			if (signal->match_input_is_pre[i_index]) {
				input_vals[i_index] = pre_obs_histories[h_index][
					signal->match_input_indexes[i_index]][signal->match_input_obs_indexes[i_index]];
			} else {
				input_vals[i_index] = post_obs_histories[h_index][
					signal->match_input_indexes[i_index]][signal->match_input_obs_indexes[i_index]];
			}
		}
		signal->match_network->activate(input_vals);
		#if defined(MDEBUG) && MDEBUG
		if (rand()%3 == 0) {
		#else
		if (signal->match_network->output->acti_vals[0] >= MATCH_WEIGHT) {
		#endif /* MDEBUG */
			match_target_vals.push_back(target_val_histories[h_index]);

			vector<double> input_vals(signal->score_input_is_pre.size());
			for (int i_index = 0; i_index < (int)signal->score_input_is_pre.size(); i_index++) {
				if (signal->score_input_is_pre[i_index]) {
					input_vals[i_index] = pre_obs_histories[h_index][
						signal->score_input_indexes[i_index]][signal->score_input_obs_indexes[i_index]];
				} else {
					input_vals[i_index] = post_obs_histories[h_index][
						signal->score_input_indexes[i_index]][signal->score_input_obs_indexes[i_index]];
				}
			}
			signal->score_network->activate(input_vals);
			match_predicted_vals.push_back(signal->score_network->output->acti_vals[0]);
		}
	}

	if (match_target_vals.size() >= CHECK_MIN_MATCH_RATIO * (double)pre_obs_histories.size()) {
		double sum_vals = 0.0;
		for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
			sum_vals += match_target_vals[m_index];
		}
		double val_average = sum_vals / (double)match_target_vals.size();

		double default_sum_misguess = 0.0;
		for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
			default_sum_misguess += (match_target_vals[m_index] - val_average)
				* (match_target_vals[m_index] - val_average);
		}

		double signal_sum_misguess = 0.0;
		for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
			signal_sum_misguess += (match_target_vals[m_index] - match_predicted_vals[m_index])
				* (match_target_vals[m_index] - match_predicted_vals[m_index]);
		}

		#if defined(MDEBUG) && MDEBUG
		if (signal_sum_misguess < default_sum_misguess || rand()%2 == 0) {
		#else
		if (signal_sum_misguess < default_sum_misguess) {
		#endif /* MDEBUG */
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

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
	for (int s_index = (int)this->signals.size()-1; s_index >= 0; s_index--) {
		bool positive_still_good = check_signal_still_good(
			this->positive_pre_obs_histories,
			this->positive_post_obs_histories,
			this->positive_target_val_histories,
			this->signals[s_index]);
		bool still_good = check_signal_still_good(
			this->pre_obs_histories,
			this->post_obs_histories,
			this->target_val_histories,
			this->signals[s_index]);
		if (!positive_still_good || !still_good) {
			delete this->signals[s_index];
			this->signals.erase(this->signals.begin() + s_index);
		}
	}

	double curr_positive_misguess_average;
	double curr_positive_misguess_standard_deviation;
	double curr_misguess_average;
	double curr_misguess_standard_deviation;
	if (this->signals.size() > 0) {
		this->miss_average_guess = calc_miss_average_guess(
			this->pre_obs_histories,
			this->post_obs_histories,
			this->target_val_histories,
			this->signals);

		vector<double> positive_signal_vals(this->positive_pre_obs_histories.size());
		for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
			positive_signal_vals[h_index] = calc_signal(this->positive_pre_obs_histories[h_index],
														this->positive_post_obs_histories[h_index],
														this->signals,
														this->miss_average_guess);
		}

		double positive_sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
			positive_sum_misguess += (this->positive_target_val_histories[h_index] - positive_signal_vals[h_index])
				* (this->positive_target_val_histories[h_index] - positive_signal_vals[h_index]);
		}
		curr_positive_misguess_average = positive_sum_misguess / (double)this->positive_pre_obs_histories.size();

		double positive_sum_misguess_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
			double curr_misguess = (this->positive_target_val_histories[h_index] - positive_signal_vals[h_index])
				* (this->positive_target_val_histories[h_index] - positive_signal_vals[h_index]);
			positive_sum_misguess_variance += (curr_misguess - curr_positive_misguess_average)
				* (curr_misguess - curr_positive_misguess_average);
		}
		curr_positive_misguess_standard_deviation = sqrt(positive_sum_misguess_variance / (double)this->positive_pre_obs_histories.size());

		vector<double> signal_vals(this->pre_obs_histories.size());
		for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
			signal_vals[h_index] = calc_signal(this->pre_obs_histories[h_index],
											   this->post_obs_histories[h_index],
											   this->signals,
											   this->miss_average_guess);
		}

		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
			sum_misguess += (this->target_val_histories[h_index] - signal_vals[h_index])
				* (this->target_val_histories[h_index] - signal_vals[h_index]);
		}
		curr_misguess_average = sum_misguess / (double)this->pre_obs_histories.size();

		double sum_misguess_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->pre_obs_histories.size(); h_index++) {
			double curr_misguess = (this->target_val_histories[h_index] - signal_vals[h_index])
				* (this->target_val_histories[h_index] - signal_vals[h_index]);
			sum_misguess_variance += (curr_misguess - curr_misguess_average)
				* (curr_misguess - curr_misguess_average);
		}
		curr_misguess_standard_deviation = sqrt(sum_misguess_variance / (double)this->pre_obs_histories.size());
	} else {
		double positive_sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->positive_target_val_histories.size(); h_index++) {
			positive_sum_vals += this->positive_target_val_histories[h_index];
		}
		double positive_average_val = positive_sum_vals / (double)this->positive_target_val_histories.size();

		double positive_sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)this->positive_target_val_histories.size(); h_index++) {
			positive_sum_misguess += (this->positive_target_val_histories[h_index] - positive_average_val)
				* (this->positive_target_val_histories[h_index] - positive_average_val);
		}
		curr_positive_misguess_average = positive_sum_misguess / (double)this->positive_target_val_histories.size();

		double positive_sum_misguess_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->positive_target_val_histories.size(); h_index++) {
			double curr_misguess = (this->positive_target_val_histories[h_index] - positive_average_val)
				* (this->positive_target_val_histories[h_index] - positive_average_val);
			positive_sum_misguess_variance += (curr_misguess - curr_positive_misguess_average)
				* (curr_misguess - curr_positive_misguess_average);
		}
		curr_positive_misguess_standard_deviation = sqrt(positive_sum_misguess_variance / (double)this->positive_target_val_histories.size());

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
	}

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

				vector<double> positive_potential_vals(this->positive_pre_obs_histories.size());
				for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
					positive_potential_vals[h_index] = calc_signal(
						this->positive_pre_obs_histories[h_index],
						this->positive_post_obs_histories[h_index],
						potential_signals,
						potential_miss_average_guess);
				}

				double positive_sum_potential_misguess = 0.0;
				for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
					positive_sum_potential_misguess += (this->positive_target_val_histories[h_index] - positive_potential_vals[h_index])
						* (this->positive_target_val_histories[h_index] - positive_potential_vals[h_index]);
				}
				double positive_potential_misguess_average = positive_sum_potential_misguess / (double)this->positive_pre_obs_histories.size();

				double positive_sum_potential_misguess_variance = 0.0;
				for (int h_index = 0; h_index < (int)this->positive_pre_obs_histories.size(); h_index++) {
					double curr_misguess = (this->positive_target_val_histories[h_index] - positive_potential_vals[h_index])
						* (this->positive_target_val_histories[h_index] - positive_potential_vals[h_index]);
					positive_sum_potential_misguess_variance += (curr_misguess - positive_potential_misguess_average)
						* (curr_misguess - positive_potential_misguess_average);
				}
				double positive_potential_misguess_standard_deviation = sqrt(positive_sum_potential_misguess_variance / (double)this->positive_pre_obs_histories.size());

				double positive_misguess_improvement = curr_positive_misguess_average - positive_potential_misguess_average;
				double positive_min_standard_deviation = min(curr_positive_misguess_standard_deviation, positive_potential_misguess_standard_deviation);
				double positive_t_score = positive_misguess_improvement / (positive_min_standard_deviation / sqrt((double)this->positive_pre_obs_histories.size()));

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
				cout << "curr_positive_misguess_average: " << curr_positive_misguess_average << endl;
				cout << "positive_potential_misguess_average: " << positive_potential_misguess_average << endl;
				cout << "measure positive_t_score: " << positive_t_score << endl;
				cout << "curr_misguess_average: " << curr_misguess_average << endl;
				cout << "potential_misguess_average: " << potential_misguess_average << endl;
				cout << "measure t_score: " << t_score << endl;

				#if defined(MDEBUG) && MDEBUG
				if ((positive_t_score >= 1.282 && t_score >= -0.674) || rand()%2 == 0) {
				#else
				if (positive_t_score >= 1.282 && t_score >= -0.674) {
				#endif /* MDEBUG */
					this->signals = potential_signals;
					this->miss_average_guess = potential_miss_average_guess;

					curr_positive_misguess_average = positive_potential_misguess_average;
					curr_positive_misguess_standard_deviation = positive_potential_misguess_standard_deviation;
					curr_misguess_average = potential_misguess_average;
					curr_misguess_standard_deviation = potential_misguess_standard_deviation;
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

	bool is_success = false;
	if (this->scope_context->signals.size() == 0) {
		if (this->signals.size() > 0) {
			is_success = true;
		}
	} else {
		double misguess_improvement = this->scope_context->signal_misguess_average - curr_misguess_average;
		double min_standard_deviation = min(this->scope_context->signal_misguess_standard_deviation, curr_misguess_standard_deviation);
		double t_score = misguess_improvement / (min_standard_deviation / sqrt((double)this->pre_obs_histories.size()));

		#if defined(MDEBUG) && MDEBUG
		if ((curr_positive_misguess_average < this->scope_context->signal_positive_misguess_average
					&& t_score > -0.674)
				|| (this->signals.size() > 0 && rand()%2 == 0)) {
		#else
		if (curr_positive_misguess_average < this->scope_context->signal_positive_misguess_average
				&& t_score > -0.674) {
		#endif /* MDEBUG */
			is_success = true;
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

		this->scope_context->signal_positive_misguess_average = curr_positive_misguess_average;
		this->scope_context->signal_positive_misguess_standard_deviation = curr_positive_misguess_standard_deviation;
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
