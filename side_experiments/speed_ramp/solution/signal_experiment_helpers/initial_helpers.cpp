#include "signal_experiment.h"

#include "globals.h"
#include "signal.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_MIN_EXISTING_NUM_SAMPLES = 10;
const int INITIAL_C1_NUM_SAMPLES = 1;
const int INITIAL_C2_NUM_SAMPLES = 2;
const int INITIAL_C3_NUM_SAMPLES = 3;
const int INITIAL_C4_NUM_SAMPLES = 4;
#else
const int INITIAL_MIN_EXISTING_NUM_SAMPLES = 100;
const int INITIAL_C1_NUM_SAMPLES = 2;
const int INITIAL_C2_NUM_SAMPLES = 10;
const int INITIAL_C3_NUM_SAMPLES = 50;
const int INITIAL_C4_NUM_SAMPLES = 200;
#endif /* MDEBUG */

void SignalExperiment::initial_backprop(double target_val,
										SignalExperimentHistory* history,
										SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->pre_obs.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				if (history->inner_is_explore[i_index] == history->outer_is_explore[i_index]) {
					if (history->inner_is_explore[i_index]) {
						if (this->new_explore_pre_obs.size() >= NEW_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_explore_pre_obs[index] = history->pre_obs[i_index];
							this->new_explore_post_obs[index] = history->post_obs[i_index];
							this->new_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->new_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_explore_post_obs.push_back(history->post_obs[i_index]);
							this->new_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->new_current_pre_obs.size() >= NEW_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_current_pre_obs[index] = history->pre_obs[i_index];
							this->new_current_post_obs[index] = history->post_obs[i_index];
							this->new_current_scores[index] = history->signal_vals[i_index];
						} else {
							this->new_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_current_post_obs.push_back(history->post_obs[i_index]);
							this->new_current_scores.push_back(history->signal_vals[i_index]);
						}
					}
				}
			} else {
				if (history->inner_is_explore[i_index] == wrapper->has_explore) {
					if (history->inner_is_explore[i_index]) {
						if (this->new_explore_pre_obs.size() >= NEW_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_explore_pre_obs[index] = history->pre_obs[i_index];
							this->new_explore_post_obs[index] = history->post_obs[i_index];
							this->new_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->new_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_explore_post_obs.push_back(history->post_obs[i_index]);
							this->new_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->new_current_pre_obs.size() >= NEW_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_current_pre_obs[index] = history->pre_obs[i_index];
							this->new_current_post_obs[index] = history->post_obs[i_index];
							this->new_current_scores[index] = target_val;
						} else {
							this->new_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_current_post_obs.push_back(history->post_obs[i_index]);
							this->new_current_scores.push_back(target_val);
						}
					}
				}
			}
		}
	} else {
		this->existing_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->pre_obs.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				if (history->inner_is_explore[i_index] == history->outer_is_explore[i_index]) {
					if (history->inner_is_explore[i_index]) {
						if (this->existing_explore_pre_obs.size() >= EXISTING_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_explore_pre_obs[index] = history->pre_obs[i_index];
							this->existing_explore_post_obs[index] = history->post_obs[i_index];
							this->existing_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->existing_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_explore_post_obs.push_back(history->post_obs[i_index]);
							this->existing_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->existing_current_pre_obs.size() >= EXISTING_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_current_pre_obs[index] = history->pre_obs[i_index];
							this->existing_current_post_obs[index] = history->post_obs[i_index];
							this->existing_current_scores[index] = history->signal_vals[i_index];
						} else {
							this->existing_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_current_post_obs.push_back(history->post_obs[i_index]);
							this->existing_current_scores.push_back(history->signal_vals[i_index]);
						}
					}
				}
			} else {
				if (history->inner_is_explore[i_index] == wrapper->has_explore) {
					if (history->inner_is_explore[i_index]) {
						if (this->existing_explore_pre_obs.size() >= EXISTING_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_explore_pre_obs[index] = history->pre_obs[i_index];
							this->existing_explore_post_obs[index] = history->post_obs[i_index];
							this->existing_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->existing_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_explore_post_obs.push_back(history->post_obs[i_index]);
							this->existing_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->existing_current_pre_obs.size() >= EXISTING_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_current_pre_obs[index] = history->pre_obs[i_index];
							this->existing_current_post_obs[index] = history->post_obs[i_index];
							this->existing_current_scores[index] = target_val;
						} else {
							this->existing_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_current_post_obs.push_back(history->post_obs[i_index]);
							this->existing_current_scores.push_back(target_val);
						}
					}
				}
			}
		}
	}

	bool is_eval = false;
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
		if (this->new_scores.size() >= INITIAL_C1_NUM_SAMPLES
				&& this->existing_scores.size() >= INITIAL_MIN_EXISTING_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
		if (this->new_scores.size() >= INITIAL_C2_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
		if (this->new_scores.size() >= INITIAL_C3_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
		if (this->new_scores.size() >= INITIAL_C4_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	}

	if (is_eval) {
		double existing_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			existing_sum_score += this->existing_scores[h_index];
		}
		double existing_score_average = existing_sum_score / (double)this->existing_scores.size();

		double existing_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			existing_sum_variance += (this->existing_scores[h_index] - existing_score_average)
				* (this->existing_scores[h_index] - existing_score_average);
		}
		double existing_score_standard_deviation = sqrt(existing_sum_variance / (double)this->existing_scores.size());

		double new_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_score += this->new_scores[h_index];
		}
		double new_score_average = new_sum_score / (double)this->new_scores.size();

		double new_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_variance += (this->new_scores[h_index] - new_score_average)
				* (this->new_scores[h_index] - new_score_average);
		}
		double new_score_standard_deviation = sqrt(new_sum_variance / (double)this->new_scores.size());

		double score_improvement = new_score_average - existing_score_average;
		double existing_score_standard_error = existing_score_standard_deviation / sqrt((double)this->existing_scores.size());
		double new_score_standard_error = new_score_standard_deviation / sqrt((double)this->new_scores.size());
		double score_t_score = score_improvement / sqrt(
			existing_score_standard_error * existing_score_standard_error
				+ new_score_standard_error * new_score_standard_error);

		#if defined(MDEBUG) && MDEBUG
		if (score_t_score >= -0.674 || rand()%3 != 0) {
		#else
		if (score_t_score >= -0.674) {
		#endif /* MDEBUG */
			switch (this->state) {
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
				break;
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
				break;
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
				this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C4;
				break;
			case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
				this->existing_scores.clear();
				this->new_scores.clear();

				this->curr_ramp = 0;

				this->state = SIGNAL_EXPERIMENT_STATE_RAMP;
				this->state_iter = 0;

				break;
			}
		} else {
			for (int s_index = 0; s_index < (int)this->adjusted_previous_signals.size(); s_index++) {
				delete this->adjusted_previous_signals[s_index];
			}
			this->adjusted_previous_signals.clear();

			this->existing_scores.clear();
			this->new_scores.clear();

			this->new_current_pre_obs.clear();
			this->new_current_post_obs.clear();
			this->new_current_scores.clear();

			this->new_explore_pre_obs.clear();
			this->new_explore_post_obs.clear();
			this->new_explore_scores.clear();

			set_actions();

			this->state = SIGNAL_EXPERIMENT_STATE_INITIAL_C1;
			this->state_iter = 0;
		}
	}
}
