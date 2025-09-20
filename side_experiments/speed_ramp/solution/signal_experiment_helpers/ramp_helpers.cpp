#include "signal_experiment.h"

#include "default_signal.h"
#include "globals.h"
#include "scope.h"
#include "signal.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int RAMP_EPOCH_NUM_ITERS = 20;

const int EVAL_GEAR = 2;
#else
const int RAMP_EPOCH_NUM_ITERS = 8000;

const int EVAL_GEAR = 5;
#endif /* MDEBUG */

void SignalExperiment::ramp_backprop(double target_val,
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

	this->state_iter++;
	#if defined(MDEBUG) && MDEBUG
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS
			&& this->existing_scores.size() >= 2
			&& this->new_scores.size() >= 2) {
	#else
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
	#endif /* MDEBUG */
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

		this->existing_scores.clear();
		this->new_scores.clear();

		this->state_iter = 0;

		#if defined(MDEBUG) && MDEBUG
		if (score_t_score >= -0.674 || rand()%3 != 0) {
		#else
		if (score_t_score >= -0.674) {
		#endif /* MDEBUG */
			this->curr_ramp++;
			if (this->curr_ramp == EVAL_GEAR) {
				DefaultSignal* new_default_signal = train_new_default();

				vector<Signal*> new_signals;
				double new_misguess_average;
				create_reward_signal_helper(this->new_current_pre_obs,
											this->new_current_post_obs,
											this->new_current_scores,
											this->new_explore_pre_obs,
											this->new_explore_post_obs,
											this->new_explore_scores,
											new_default_signal,
											this->adjusted_previous_signals,
											new_signals,
											new_misguess_average);

				if (this->scope_context->default_signal != NULL) {
					DefaultSignal* existing_default_signal = train_existing_default();

					vector<Signal*> existing_previous_signals;
					for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
						existing_previous_signals.push_back(this->scope_context->signals[s_index]);
					}

					vector<Signal*> existing_signals;
					double existing_misguess_average;
					create_reward_signal_helper(this->existing_current_pre_obs,
												this->existing_current_post_obs,
												this->existing_current_scores,
												this->existing_explore_pre_obs,
												this->existing_explore_post_obs,
												this->existing_explore_scores,
												existing_default_signal,
												existing_previous_signals,
												existing_signals,
												existing_misguess_average);

					if (existing_misguess_average < new_misguess_average) {
						for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
							delete this->scope_context->signals[s_index];
						}
						this->scope_context->signals = existing_signals;
						existing_signals.clear();
						if (this->scope_context->default_signal != NULL) {
							delete this->scope_context->default_signal;
						}
						this->scope_context->default_signal = existing_default_signal;
						existing_default_signal = NULL;
					} else {
						vector<int> temp_pre_actions = this->pre_actions;
						vector<int> temp_post_actions = this->post_actions;
						this->pre_actions = this->scope_context->signal_pre_actions;
						this->post_actions = this->scope_context->signal_post_actions;
						this->scope_context->signal_pre_actions = temp_pre_actions;
						this->scope_context->signal_post_actions = temp_post_actions;

						this->pre_action_initialized = vector<bool>(this->pre_actions.size(), true);
						this->post_action_initialized = vector<bool>(this->post_actions.size(), true);

						for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
							delete this->scope_context->signals[s_index];
						}
						this->scope_context->signals = new_signals;
						new_signals.clear();
						if (this->scope_context->default_signal != NULL) {
							delete this->scope_context->default_signal;
						}
						this->scope_context->default_signal = new_default_signal;
						new_default_signal = NULL;
					}

					for (int s_index = 0; s_index < (int)existing_previous_signals.size(); s_index++) {
						delete existing_previous_signals[s_index];
					}
					for (int s_index = 0; s_index < (int)existing_signals.size(); s_index++) {
						delete existing_signals[s_index];
					}
					if (existing_default_signal != NULL) {
						delete existing_default_signal;
					}
				} else {
					vector<int> temp_pre_actions = this->pre_actions;
					vector<int> temp_post_actions = this->post_actions;
					this->pre_actions = this->scope_context->signal_pre_actions;
					this->post_actions = this->scope_context->signal_post_actions;
					this->scope_context->signal_pre_actions = temp_pre_actions;
					this->scope_context->signal_post_actions = temp_post_actions;

					this->pre_action_initialized = vector<bool>(this->pre_actions.size(), true);
					this->post_action_initialized = vector<bool>(this->post_actions.size(), true);

					for (int s_index = 0; s_index < (int)this->scope_context->signals.size(); s_index++) {
						delete this->scope_context->signals[s_index];
					}
					this->scope_context->signals = new_signals;
					new_signals.clear();
					if (this->scope_context->default_signal != NULL) {
						delete this->scope_context->default_signal;
					}
					this->scope_context->default_signal = new_default_signal;
					new_default_signal = NULL;
				}

				for (int s_index = 0; s_index < (int)new_signals.size(); s_index++) {
					delete new_signals[s_index];
				}
				if (new_default_signal != NULL) {
					delete new_default_signal;
				}

				this->curr_ramp = EVAL_GEAR-2;

				this->state = SIGNAL_EXPERIMENT_STATE_RAMP;
				this->state_iter = 0;
			}
		} else {
			this->curr_ramp--;
			if (this->curr_ramp < 0) {
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
}
