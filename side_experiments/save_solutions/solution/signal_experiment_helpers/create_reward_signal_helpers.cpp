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

#include "constants.h"
#include "helpers.h"
#include "signal_instance.h"
#include "signal_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int MATCH_TYPE_POSITIVE = 0;
const int MATCH_TYPE_TRAP = 1;
const int MATCH_TYPE_CURRENT = 2;
const int MATCH_TYPE_EXPLORE = 3;

#if defined(MDEBUG) && MDEBUG
const int SPLIT_NUM_TRIES = 3;
#else
const int SPLIT_NUM_TRIES = 20;
#endif /* MDEBUG */

const double MIN_MATCH_RATIO = 0.04;

const double CHECK_MIN_MATCH_RATIO = 0.02;

bool SignalExperiment::check_instance_still_good(SignalInstance* instance,
												 SolutionWrapper* wrapper) {
	vector<double> match_target_vals;
	vector<double> match_predicted_vals;

	double curr_val_average = wrapper->solution->curr_val_average;

	for (int h_index = 0; h_index < (int)this->positive_pre_obs.size(); h_index++) {
		if (instance->is_match(this->positive_pre_obs[h_index],
							   this->positive_post_obs[h_index])) {
			match_target_vals.push_back(this->positive_scores[h_index] - curr_val_average);
			match_predicted_vals.push_back(instance->calc_score(
				this->positive_pre_obs[h_index],
				this->positive_post_obs[h_index]));
		}
	}

	for (int h_index = 0; h_index < (int)this->trap_pre_obs.size(); h_index++) {
		if (instance->is_match(this->trap_pre_obs[h_index],
							   this->trap_post_obs[h_index])) {
			match_target_vals.push_back(this->trap_scores[h_index] - curr_val_average);
			match_predicted_vals.push_back(instance->calc_score(
				this->trap_pre_obs[h_index],
				this->trap_post_obs[h_index]));
		}
	}

	for (int h_index = 0; h_index < (int)this->current_pre_obs.size(); h_index++) {
		if (instance->is_match(this->current_pre_obs[h_index],
							   this->current_post_obs[h_index])) {
			match_target_vals.push_back(this->current_scores[h_index] - curr_val_average);
			match_predicted_vals.push_back(instance->calc_score(
				this->current_pre_obs[h_index],
				this->current_post_obs[h_index]));
		}
	}

	for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
		if (instance->is_match(this->explore_pre_obs[h_index],
							   this->explore_post_obs[h_index])) {
			match_target_vals.push_back(this->explore_scores[h_index] - curr_val_average);
			match_predicted_vals.push_back(instance->calc_score(
				this->explore_pre_obs[h_index],
				this->explore_post_obs[h_index]));
		}
	}
	/**
	 * - OK to include explore even if extremes
	 *   - will still be better than default
	 */

	int num_min_match = CHECK_MIN_MATCH_RATIO * (double)(this->positive_pre_obs.size()
		+ this->trap_pre_obs.size() + this->current_pre_obs.size() + this->explore_pre_obs.size());
	if ((int)match_target_vals.size() >= num_min_match) {
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

		double instance_sum_misguess = 0.0;
		for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
			instance_sum_misguess += (match_target_vals[m_index] - match_predicted_vals[m_index])
				* (match_target_vals[m_index] - match_predicted_vals[m_index]);
		}

		#if defined(MDEBUG) && MDEBUG
		if (instance_sum_misguess < default_sum_misguess || rand()%2 == 0) {
		#else
		if (instance_sum_misguess < default_sum_misguess) {
		#endif /* MDEBUG */
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void SignalExperiment::create_reward_signal_helper(SolutionWrapper* wrapper) {
	double sum_explore_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->explore_scores.size(); h_index++) {
		sum_explore_vals += this->explore_scores[h_index];
	}
	double new_default_guess = sum_explore_vals / (double)this->explore_scores.size();

	vector<double> positive_predicted(this->positive_pre_obs.size());
	vector<bool> positive_has_match(this->positive_pre_obs.size(), false);
	for (int h_index = 0; h_index < (int)this->positive_pre_obs.size(); h_index++) {
		bool is_match;
		double val;
		calc_signal(this->positive_pre_obs[h_index],
					this->positive_post_obs[h_index],
					this->instances,
					is_match,
					val);
		if (is_match) {
			positive_predicted[h_index] = val;
			positive_has_match[h_index] = true;
		} else {
			positive_predicted[h_index] = new_default_guess;
		}
	}

	vector<double> trap_predicted(this->trap_pre_obs.size());
	for (int h_index = 0; h_index < (int)this->trap_pre_obs.size(); h_index++) {
		trap_predicted[h_index] = calc_signal(this->trap_pre_obs[h_index],
											  this->trap_post_obs[h_index],
											  this->instances,
											  new_default_guess);
	}

	vector<double> current_predicted(this->current_pre_obs.size());
	vector<bool> current_has_match(this->current_pre_obs.size());
	for (int h_index = 0; h_index < (int)this->current_pre_obs.size(); h_index++) {
		bool is_match;
		double val;
		calc_signal(this->current_pre_obs[h_index],
					this->current_post_obs[h_index],
					this->instances,
					is_match,
					val);
		if (is_match) {
			current_predicted[h_index] = val;
			current_has_match[h_index] = true;
		} else {
			current_predicted[h_index] = new_default_guess;
		}
	}

	vector<double> explore_predicted(this->explore_pre_obs.size());
	for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
		explore_predicted[h_index] = calc_signal(this->explore_pre_obs[h_index],
												 this->explore_post_obs[h_index],
												 this->instances,
												 new_default_guess);
	}

	int num_min_match = MIN_MATCH_RATIO * (double)(this->positive_pre_obs.size()
		+ this->trap_pre_obs.size() + this->current_pre_obs.size() + this->explore_pre_obs.size());
	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<bool> new_match_input_is_pre;
		vector<int> new_match_input_indexes;
		vector<int> new_match_input_obs_indexes;
		SignalNetwork* new_match_network = NULL;
		bool split_is_success = split_helper(positive_has_match,
											 current_has_match,
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

			for (int h_index = 0; h_index < (int)this->positive_pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->positive_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->positive_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(this->positive_pre_obs[h_index]);
					match_post_obs.push_back(this->positive_post_obs[h_index]);
					match_target_vals.push_back(this->positive_scores[h_index]);
					match_curr_predicted.push_back(positive_predicted[h_index]);
					match_type.push_back(MATCH_TYPE_POSITIVE);
					match_index.push_back(h_index);
				}
			}

			for (int h_index = 0; h_index < (int)this->trap_pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->trap_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->trap_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(this->trap_pre_obs[h_index]);
					match_post_obs.push_back(this->trap_post_obs[h_index]);
					match_target_vals.push_back(this->trap_scores[h_index]);
					match_curr_predicted.push_back(trap_predicted[h_index]);
					match_type.push_back(MATCH_TYPE_TRAP);
					match_index.push_back(h_index);
				}
			}

			for (int h_index = 0; h_index < (int)this->current_pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->current_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->current_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(this->current_pre_obs[h_index]);
					match_post_obs.push_back(this->current_post_obs[h_index]);
					match_target_vals.push_back(this->current_scores[h_index]);
					match_curr_predicted.push_back(current_predicted[h_index]);
					match_type.push_back(MATCH_TYPE_CURRENT);
					match_index.push_back(h_index);
				}
			}

			for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->explore_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->explore_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(this->explore_pre_obs[h_index]);
					match_post_obs.push_back(this->explore_post_obs[h_index]);
					match_target_vals.push_back(this->explore_scores[h_index]);
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
					SignalInstance* new_instance = new SignalInstance();
					new_instance->match_input_is_pre = new_match_input_is_pre;
					new_instance->match_input_indexes = new_match_input_indexes;
					new_instance->match_input_obs_indexes = new_match_input_obs_indexes;
					new_instance->match_network = new_match_network;
					new_match_network = NULL;
					new_instance->score_input_is_pre = new_score_input_is_pre;
					new_instance->score_input_indexes = new_score_input_indexes;
					new_instance->score_input_obs_indexes = new_score_input_obs_indexes;
					new_instance->score_network = new_score_network;
					new_score_network = NULL;
					this->instances.push_back(new_instance);

					for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
						switch (match_type[m_index]) {
						case MATCH_TYPE_POSITIVE:
							positive_predicted[match_index[m_index]] = match_new_predicted[m_index];
							break;
						case MATCH_TYPE_TRAP:
							trap_predicted[match_index[m_index]] = match_new_predicted[m_index];
							break;
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
	for (int h_index = 0; h_index < (int)this->positive_pre_obs.size(); h_index++) {
		sum_misguess += (this->positive_scores[h_index] - positive_predicted[h_index])
			* (this->positive_scores[h_index] - positive_predicted[h_index]);
	}
	for (int h_index = 0; h_index < (int)this->trap_pre_obs.size(); h_index++) {
		sum_misguess += (this->trap_scores[h_index] - trap_predicted[h_index])
			* (this->trap_scores[h_index] - trap_predicted[h_index]);
	}
	for (int h_index = 0; h_index < (int)this->current_pre_obs.size(); h_index++) {
		sum_misguess += (this->current_scores[h_index] - current_predicted[h_index])
			* (this->current_scores[h_index] - current_predicted[h_index]);
	}
	for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
		sum_misguess += (this->explore_scores[h_index] - explore_predicted[h_index])
			* (this->explore_scores[h_index] - explore_predicted[h_index]);
	}
	this->misguess_average = sum_misguess / (double)(this->positive_pre_obs.size()
		+ this->trap_pre_obs.size() + this->current_pre_obs.size() + this->explore_pre_obs.size());
}
