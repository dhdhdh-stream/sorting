#include "signal_experiment.h"

#include <iostream>

#include "default_signal.h"
#include "scope.h"
#include "signal.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void SignalExperiment::gather_backprop(double target_val,
									   SignalExperimentHistory* history,
									   SolutionWrapper* wrapper) {
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
