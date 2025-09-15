#include "helpers.h"

#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "explore_experiment.h"
#include "scope.h"
#include "signal.h"
#include "signal_eval_experiment.h"
#include "signal_network.h"
#include "solution_wrapper.h"

using namespace std;

double calc_signal(ScopeHistory* signal_needed_from) {
	Scope* scope = signal_needed_from->scope;

	for (int s_index = 0; s_index < (int)scope->signals.size(); s_index++) {
		bool is_match;
		double val;
		scope->signals[s_index]->calc(signal_needed_from->signal_pre_obs,
									  signal_needed_from->signal_post_obs,
									  is_match,
									  val);
		if (is_match) {
			return val;
		}
	}

	return scope->default_signal->calc(signal_needed_from->signal_pre_obs,
									   signal_needed_from->signal_post_obs);
}

bool check_signal_activate(vector<double>& obs,
						   int& action,
						   bool& is_next,
						   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();
	Scope* scope = scope_history->scope;

	/**
	 * - check pre
	 */
	if (scope_history->node_histories.size() == 0) {
		scope_history->signal_pre_obs.push_back(obs);

		if (scope_history->signal_pre_obs.size() <= scope->signal_pre_actions.size()) {
			action = scope->signal_pre_actions[scope_history->signal_pre_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		}
	}

	/**
	 * - check post
	 */
	if (wrapper->node_context.back() == NULL) {
		scope_history->signal_post_obs.push_back(obs);

		if (scope_history->signal_post_obs.size() <= scope->signal_post_actions.size()) {
			action = scope->signal_post_actions[scope_history->signal_post_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		}
	}

	return false;
}

bool experiment_check_signal_activate(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();
	Scope* scope = scope_history->scope;

	bool is_experiment = false;
	SignalEvalExperimentHistory* signal_eval_experiment_history;
	if (scope->curr_signal_eval_experiment != NULL) {
		map<SignalEvalExperiment*, SignalEvalExperimentHistory*>::iterator it =
			wrapper->signal_eval_histories.find(scope->curr_signal_eval_experiment);
		if (it == wrapper->signal_eval_histories.end()) {
			signal_eval_experiment_history = new SignalEvalExperimentHistory();
			wrapper->signal_eval_histories[scope->curr_signal_eval_experiment] = signal_eval_experiment_history;
		} else {
			signal_eval_experiment_history = it->second;
		}

		if (signal_eval_experiment_history->is_on) {
			is_experiment = true;
		}
	}

	if (is_experiment) {
		return scope->curr_signal_eval_experiment->check_signal_activate(
			obs,
			action,
			is_next,
			wrapper,
			signal_eval_experiment_history);
	} else {
		/**
		 * - check pre
		 */
		if (scope_history->node_histories.size() == 0) {
			scope_history->signal_pre_obs.push_back(obs);

			if (scope_history->signal_pre_obs.size() <= scope->signal_pre_actions.size()) {
				action = scope->signal_pre_actions[scope_history->signal_pre_obs.size()-1];
				is_next = true;

				wrapper->num_actions++;

				return true;
			}
		}

		/**
		 * - check post
		 */
		if (wrapper->node_context.back() == NULL
				&& wrapper->experiment_context.back() == NULL) {
			scope_history->signal_post_obs.push_back(obs);

			if (scope_history->signal_post_obs.size() <= scope->signal_post_actions.size()) {
				action = scope->signal_post_actions[scope_history->signal_post_obs.size()-1];
				is_next = true;

				wrapper->num_actions++;

				return true;
			} else {
				if (scope_history->explore_experiment_callbacks.size() > 0
						|| scope_history->signal_eval_experiment_callbacks.size() > 0) {
					double signal = calc_signal(scope_history);
					for (int e_index = 0; e_index < (int)scope_history->explore_experiment_callbacks.size(); e_index++) {
						scope_history->explore_experiment_callbacks[e_index]->signal_is_set.back() = true;
						scope_history->explore_experiment_callbacks[e_index]->signal_vals.back() = signal;
					}
					for (int e_index = 0; e_index < (int)scope_history->signal_eval_experiment_callbacks.size(); e_index++) {
						scope_history->signal_eval_experiment_callbacks[e_index]->signal_is_set.back() = true;
						scope_history->signal_eval_experiment_callbacks[e_index]->signal_vals.back() = signal;
						scope_history->signal_eval_experiment_callbacks[e_index]->outer_is_explore.back() = wrapper->has_explore;
					}
				}
			}
		}
	}

	return false;
}
