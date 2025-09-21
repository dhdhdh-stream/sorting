#include "signal_experiment.h"

#include <iostream>

#include "default_signal.h"
#include "scope.h"
#include "signal.h"

using namespace std;

void SignalExperiment::gather_backprop(double target_val,
									   SignalExperimentHistory* history,
									   SolutionWrapper* wrapper) {
	if ((int)this->existing_current_pre_obs.size() >= EXISTING_CURRENT_SAMPLES
			&& (int)this->existing_explore_pre_obs.size() >= EXISTING_EXPLORE_SAMPLES
			&& (int)this->new_current_pre_obs.size() >= NEW_CURRENT_SAMPLES
			&& (int)this->new_explore_pre_obs.size() >= NEW_EXPLORE_SAMPLES) {
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
				existing_previous_signals.push_back(new Signal(this->scope_context->signals[s_index]));
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

		this->curr_ramp--;

		this->state = SIGNAL_EXPERIMENT_STATE_WRAPUP;
		this->state_iter = 0;
	}
}
