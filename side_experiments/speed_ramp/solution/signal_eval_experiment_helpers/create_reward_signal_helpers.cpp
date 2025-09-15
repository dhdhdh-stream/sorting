#include "signal_eval_experiment.h"

#include <cmath>
#include <iostream>

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

void SignalEvalExperiment::create_reward_signal_helper() {
	{
		vector<vector<vector<double>>> combined_pre_obs;
		combined_pre_obs.insert(combined_pre_obs.end(),
			pre_obs.begin(), pre_obs.end());
		combined_pre_obs.insert(combined_pre_obs.end(),
			explore_pre_obs.begin(), explore_pre_obs.end());
		vector<vector<vector<double>>> combined_post_obs;
		combined_post_obs.insert(combined_post_obs.end(),
			post_obs.begin(), post_obs.end());
		combined_post_obs.insert(combined_post_obs.end(),
			explore_post_obs.begin(), explore_post_obs.end());
		vector<double> combined_scores;
		combined_scores.insert(combined_scores.end(),
			scores.begin(), scores.end());
		combined_scores.insert(combined_scores.end(),
			explore_scores.begin(), explore_scores.end());

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

		this->default_signal = new DefaultSignal();
		this->default_signal->score_input_is_pre = new_score_input_is_pre;
		this->default_signal->score_input_indexes = new_score_input_indexes;
		this->default_signal->score_input_obs_indexes = new_score_input_obs_indexes;
		this->default_signal->score_network = new_score_network;
	}

	vector<double> predicted(this->pre_obs.size());
	for (int h_index = 0; h_index < (int)this->pre_obs.size(); h_index++) {
		predicted[h_index] = this->default_signal->calc(
			this->pre_obs[h_index],
			this->post_obs[h_index]);
	}

	vector<double> explore_predicted(this->explore_pre_obs.size());
	for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
		explore_predicted[h_index] = this->default_signal->calc(
			this->explore_pre_obs[h_index],
			this->explore_post_obs[h_index]);
	}

	for (int s_index = this->previous_signals.size()-1; s_index >= 0; s_index--) {
		vector<vector<vector<double>>> match_pre_obs;
		vector<vector<vector<double>>> match_post_obs;
		vector<double> match_target_vals;
		vector<double> match_curr_predicted;
		vector<double> match_new_predicted;
		vector<int> match_type;
		vector<int> match_index;

		for (int h_index = 0; h_index < (int)this->pre_obs.size(); h_index++) {
			bool is_match;
			double val;
			this->previous_signals[s_index]->calc(this->pre_obs[h_index],
												  this->post_obs[h_index],
												  is_match,
												  val);
			if (is_match) {
				match_pre_obs.push_back(this->pre_obs[h_index]);
				match_post_obs.push_back(this->post_obs[h_index]);
				match_target_vals.push_back(this->scores[h_index]);
				match_curr_predicted.push_back(predicted[h_index]);
				match_new_predicted.push_back(val);
				match_type.push_back(MATCH_TYPE_CURRENT);
				match_index.push_back(h_index);
			}
		}

		for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
			bool is_match;
			double val;
			this->previous_signals[s_index]->calc(this->explore_pre_obs[h_index],
												  this->explore_post_obs[h_index],
												  is_match,
												  val);
			if (is_match) {
				match_pre_obs.push_back(this->explore_pre_obs[h_index]);
				match_post_obs.push_back(this->explore_post_obs[h_index]);
				match_target_vals.push_back(this->explore_scores[h_index]);
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
			this->signals.insert(this->signals.begin(), this->previous_signals[s_index]);
			this->previous_signals.erase(this->previous_signals.begin() + s_index);

			for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
				switch (match_type[m_index]) {
				case MATCH_TYPE_CURRENT:
					predicted[match_index[m_index]] = match_new_predicted[m_index];
					break;
				case MATCH_TYPE_EXPLORE:
					explore_predicted[match_index[m_index]] = match_new_predicted[m_index];
					break;
				}
			}
		}
	}

	int num_min_match = MIN_MATCH_RATIO * (double)(this->pre_obs.size() + this->explore_pre_obs.size());
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
			vector<vector<vector<double>>> match_pre_obs;
			vector<vector<vector<double>>> match_post_obs;
			vector<double> match_target_vals;
			vector<double> match_curr_predicted;
			vector<int> match_type;
			vector<int> match_index;

			for (int h_index = 0; h_index < (int)this->pre_obs.size(); h_index++) {
				vector<double> input_vals(new_match_input_is_pre.size());
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						input_vals[i_index] = this->pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						input_vals[i_index] = this->post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
				new_match_network->activate(input_vals);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > MATCH_WEIGHT) {
				#endif /* MDEBUG */
					match_pre_obs.push_back(this->pre_obs[h_index]);
					match_post_obs.push_back(this->post_obs[h_index]);
					match_target_vals.push_back(this->scores[h_index]);
					match_curr_predicted.push_back(predicted[h_index]);
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
					this->signals.push_back(new_signal);

					for (int m_index = 0; m_index < (int)match_pre_obs.size(); m_index++) {
						switch (match_type[m_index]) {
						case MATCH_TYPE_CURRENT:
							predicted[match_index[m_index]] = match_new_predicted[m_index];
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
	for (int h_index = 0; h_index < (int)this->pre_obs.size(); h_index++) {
		sum_misguess += (this->scores[h_index] - predicted[h_index])
			* (this->scores[h_index] - predicted[h_index]);
	}
	for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
		sum_misguess += (this->explore_scores[h_index] - explore_predicted[h_index])
			* (this->explore_scores[h_index] - explore_predicted[h_index]);
	}
	this->misguess_average = sum_misguess / (double)(this->pre_obs.size() + this->explore_pre_obs.size());

	cout << "this->misguess_average: " << this->misguess_average << endl;
}
