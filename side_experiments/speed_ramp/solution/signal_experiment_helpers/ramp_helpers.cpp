#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "scope.h"
#include "signal.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void SignalExperiment::ramp_backprop(double target_val,
									 SignalExperimentHistory* history,
									 SolutionWrapper* wrapper) {
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

		double new_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_score += this->new_scores[h_index];
		}
		double new_score_average = new_sum_score / (double)this->new_scores.size();

		this->existing_scores.clear();
		this->new_scores.clear();

		this->state_iter = 0;

		#if defined(MDEBUG) && MDEBUG
		if (new_score_average >= existing_score_average || rand()%3 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
			this->curr_ramp++;
			this->num_fail = 0;

			if (this->curr_ramp == EVAL_GEAR) {
				this->curr_ramp--;

				if ((int)this->existing_explore_pre_obs.size() >= EXPLORE_SAMPLES
						&& (int)this->new_explore_pre_obs.size() >= EXPLORE_SAMPLES) {
					cout << "SignalExperiment" << endl;
					cout << "this->scope_context->id: " << this->scope_context->id << endl;

					DefaultSignal* new_pre_default_signal = train_new_default(true);

					vector<Signal*> new_pre_signals;
					double new_pre_misguess_average;
					create_reward_signal_helper(this->new_explore_pre_obs,
												this->new_explore_post_obs,
												this->new_explore_scores,
												true,
												new_pre_default_signal,
												this->adjusted_previous_pre_signals,
												new_pre_signals,
												new_pre_misguess_average);

					DefaultSignal* new_post_default_signal = train_new_default(false);

					vector<Signal*> new_post_signals;
					double new_post_misguess_average;
					create_reward_signal_helper(this->new_explore_pre_obs,
												this->new_explore_post_obs,
												this->new_explore_scores,
												false,
												new_post_default_signal,
												this->adjusted_previous_post_signals,
												new_post_signals,
												new_post_misguess_average);

					if (this->scope_context->pre_default_signal != NULL) {
						DefaultSignal* existing_pre_default_signal = train_existing_default(true);

						vector<Signal*> existing_previous_pre_signals;
						for (int s_index = 0; s_index < (int)this->scope_context->pre_signals.size(); s_index++) {
							existing_previous_pre_signals.push_back(new Signal(this->scope_context->pre_signals[s_index]));
						}

						vector<Signal*> existing_pre_signals;
						double existing_pre_misguess_average;
						create_reward_signal_helper(this->existing_explore_pre_obs,
													this->existing_explore_post_obs,
													this->existing_explore_scores,
													true,
													existing_pre_default_signal,
													existing_previous_pre_signals,
													existing_pre_signals,
													existing_pre_misguess_average);

						DefaultSignal* existing_post_default_signal = train_existing_default(false);

						vector<Signal*> existing_previous_post_signals;
						for (int s_index = 0; s_index < (int)this->scope_context->post_signals.size(); s_index++) {
							existing_previous_post_signals.push_back(new Signal(this->scope_context->post_signals[s_index]));
						}

						vector<Signal*> existing_post_signals;
						double existing_post_misguess_average;
						create_reward_signal_helper(this->existing_explore_pre_obs,
													this->existing_explore_post_obs,
													this->existing_explore_scores,
													false,
													existing_post_default_signal,
													existing_previous_post_signals,
													existing_post_signals,
													existing_post_misguess_average);

						if (existing_pre_misguess_average + existing_post_misguess_average
								< new_pre_misguess_average + new_post_misguess_average) {
							for (int s_index = 0; s_index < (int)this->scope_context->pre_signals.size(); s_index++) {
								delete this->scope_context->pre_signals[s_index];
							}
							this->scope_context->pre_signals = existing_pre_signals;
							existing_pre_signals.clear();
							if (this->scope_context->pre_default_signal != NULL) {
								delete this->scope_context->pre_default_signal;
							}
							this->scope_context->pre_default_signal = existing_pre_default_signal;
							existing_pre_default_signal = NULL;
							for (int s_index = 0; s_index < (int)this->scope_context->post_signals.size(); s_index++) {
								delete this->scope_context->post_signals[s_index];
							}
							this->scope_context->post_signals = existing_post_signals;
							existing_post_signals.clear();
							if (this->scope_context->post_default_signal != NULL) {
								delete this->scope_context->post_default_signal;
							}
							this->scope_context->post_default_signal = existing_post_default_signal;
							existing_post_default_signal = NULL;
						} else {
							vector<int> temp_pre_actions = this->pre_actions;
							vector<int> temp_post_actions = this->post_actions;
							this->pre_actions = this->scope_context->signal_pre_actions;
							this->post_actions = this->scope_context->signal_post_actions;
							this->scope_context->signal_pre_actions = temp_pre_actions;
							this->scope_context->signal_post_actions = temp_post_actions;

							this->pre_action_initialized = vector<bool>(this->pre_actions.size(), true);
							this->post_action_initialized = vector<bool>(this->post_actions.size(), true);

							for (int s_index = 0; s_index < (int)this->scope_context->pre_signals.size(); s_index++) {
								delete this->scope_context->pre_signals[s_index];
							}
							this->scope_context->pre_signals = new_pre_signals;
							new_pre_signals.clear();
							if (this->scope_context->pre_default_signal != NULL) {
								delete this->scope_context->pre_default_signal;
							}
							this->scope_context->pre_default_signal = new_pre_default_signal;
							new_pre_default_signal = NULL;
							for (int s_index = 0; s_index < (int)this->scope_context->post_signals.size(); s_index++) {
								delete this->scope_context->post_signals[s_index];
							}
							this->scope_context->post_signals = new_post_signals;
							new_post_signals.clear();
							if (this->scope_context->post_default_signal != NULL) {
								delete this->scope_context->post_default_signal;
							}
							this->scope_context->post_default_signal = new_post_default_signal;
							new_post_default_signal = NULL;
						}

						for (int s_index = 0; s_index < (int)existing_previous_pre_signals.size(); s_index++) {
							delete existing_previous_pre_signals[s_index];
						}
						for (int s_index = 0; s_index < (int)existing_pre_signals.size(); s_index++) {
							delete existing_pre_signals[s_index];
						}
						if (existing_pre_default_signal != NULL) {
							delete existing_pre_default_signal;
						}
						for (int s_index = 0; s_index < (int)existing_previous_post_signals.size(); s_index++) {
							delete existing_previous_post_signals[s_index];
						}
						for (int s_index = 0; s_index < (int)existing_post_signals.size(); s_index++) {
							delete existing_post_signals[s_index];
						}
						if (existing_post_default_signal != NULL) {
							delete existing_post_default_signal;
						}

						wrapper->solution->timestamp++;
					} else {
						vector<int> temp_pre_actions = this->pre_actions;
						vector<int> temp_post_actions = this->post_actions;
						this->pre_actions = this->scope_context->signal_pre_actions;
						this->post_actions = this->scope_context->signal_post_actions;
						this->scope_context->signal_pre_actions = temp_pre_actions;
						this->scope_context->signal_post_actions = temp_post_actions;

						this->pre_action_initialized = vector<bool>(this->pre_actions.size(), true);
						this->post_action_initialized = vector<bool>(this->post_actions.size(), true);

						for (int s_index = 0; s_index < (int)this->scope_context->pre_signals.size(); s_index++) {
							delete this->scope_context->pre_signals[s_index];
						}
						this->scope_context->pre_signals = new_pre_signals;
						new_pre_signals.clear();
						if (this->scope_context->pre_default_signal != NULL) {
							delete this->scope_context->pre_default_signal;
						}
						this->scope_context->pre_default_signal = new_pre_default_signal;
						new_pre_default_signal = NULL;
						for (int s_index = 0; s_index < (int)this->scope_context->post_signals.size(); s_index++) {
							delete this->scope_context->post_signals[s_index];
						}
						this->scope_context->post_signals = new_post_signals;
						new_post_signals.clear();
						if (this->scope_context->post_default_signal != NULL) {
							delete this->scope_context->post_default_signal;
						}
						this->scope_context->post_default_signal = new_post_default_signal;
						new_post_default_signal = NULL;

						wrapper->solution->timestamp++;
					}

					for (int s_index = 0; s_index < (int)new_pre_signals.size(); s_index++) {
						delete new_pre_signals[s_index];
					}
					if (new_pre_default_signal != NULL) {
						delete new_pre_default_signal;
					}
					for (int s_index = 0; s_index < (int)new_post_signals.size(); s_index++) {
						delete new_post_signals[s_index];
					}
					if (new_post_default_signal != NULL) {
						delete new_post_default_signal;
					}

					this->curr_ramp--;

					this->state = SIGNAL_EXPERIMENT_STATE_WRAPUP;
					this->state_iter = 0;
				}
			}
		} else {
			this->num_fail++;
			if (this->num_fail >= 2) {
				this->curr_ramp--;
				this->num_fail = 0;

				if (this->curr_ramp < 0) {
					for (int s_index = 0; s_index < (int)this->adjusted_previous_pre_signals.size(); s_index++) {
						delete this->adjusted_previous_pre_signals[s_index];
					}
					this->adjusted_previous_pre_signals.clear();
					for (int s_index = 0; s_index < (int)this->adjusted_previous_post_signals.size(); s_index++) {
						delete this->adjusted_previous_post_signals[s_index];
					}
					this->adjusted_previous_post_signals.clear();

					this->existing_scores.clear();
					this->new_scores.clear();

					this->existing_explore_pre_obs.clear();
					this->existing_explore_post_obs.clear();
					this->existing_explore_scores.clear();

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
}
