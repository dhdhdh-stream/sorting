#include "helpers.h"

#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "explore_experiment.h"
#include "eval_experiment.h"
#include "problem.h"
#include "refine_experiment.h"
#include "scope.h"
#include "signal.h"
#include "signal_experiment.h"
#include "signal_network.h"
#include "solution_wrapper.h"

using namespace std;

double calc_signal(ScopeHistory* signal_needed_from) {
	Scope* scope = signal_needed_from->scope;

	double pre_val;
	{
		bool use_default = true;
		for (int s_index = 0; s_index < (int)scope->pre_signals.size(); s_index++) {
			bool is_match;
			double val;
			scope->pre_signals[s_index]->calc(signal_needed_from->signal_pre_obs,
											  signal_needed_from->signal_post_obs,
											  is_match,
											  val);
			if (is_match) {
				use_default = false;
				pre_val = val;
			}
		}

		if (use_default) {
			pre_val = scope->pre_default_signal->calc(signal_needed_from->signal_pre_obs,
													  signal_needed_from->signal_post_obs);
		}
	}

	double post_val;
	{
		bool use_default = true;
		for (int s_index = 0; s_index < (int)scope->post_signals.size(); s_index++) {
			bool is_match;
			double val;
			scope->post_signals[s_index]->calc(signal_needed_from->signal_pre_obs,
											   signal_needed_from->signal_post_obs,
											   is_match,
											   val);
			if (is_match) {
				use_default = false;
				post_val = val;
			}
		}

		if (use_default) {
			post_val = scope->post_default_signal->calc(signal_needed_from->signal_pre_obs,
														signal_needed_from->signal_post_obs);
		}
	}

	return post_val - pre_val;
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
		} else {
			if (scope->pre_default_signal != NULL) {
				scope_history->signal_initialized = true;
				scope_history->signal_val = calc_signal(scope_history);
			}
		}
	}

	return false;
}

bool experiment_check_signal_activate(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();
	Scope* scope = scope_history->scope;

	if (scope->signal_experiment != NULL) {
		return scope->signal_experiment->check_signal_activate(
			obs,
			action,
			is_next,
			fetch_action,
			wrapper);
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
				if (scope->pre_default_signal != NULL) {
					scope_history->signal_initialized = true;
					scope_history->signal_val = calc_signal(scope_history);

					for (int e_index = 0; e_index < (int)scope_history->explore_experiment_callbacks.size(); e_index++) {
						ExploreExperimentHistory* explore_experiment_history = scope_history->explore_experiment_callbacks[e_index];
						int instance_index = scope_history->explore_experiment_instance_indexes[e_index];

						explore_experiment_history->sum_signal_vals[instance_index] += scope_history->signal_val;
						explore_experiment_history->sum_counts[instance_index]++;
					}
					for (int e_index = 0; e_index < (int)scope_history->refine_experiment_callbacks.size(); e_index++) {
						RefineExperimentHistory* refine_experiment_history = scope_history->refine_experiment_callbacks[e_index];
						int instance_index = scope_history->refine_experiment_instance_indexes[e_index];

						refine_experiment_history->sum_signal_vals[instance_index] += scope_history->signal_val;
						refine_experiment_history->sum_counts[instance_index]++;
					}
					for (int e_index = 0; e_index < (int)scope_history->signal_experiment_callbacks.size(); e_index++) {
						SignalExperimentHistory* signal_experiment_history = scope_history->signal_experiment_callbacks[e_index];
						int instance_index = scope_history->signal_experiment_instance_indexes[e_index];

						signal_experiment_history->sum_signal_vals[instance_index] += scope_history->signal_val;
						signal_experiment_history->sum_counts[instance_index]++;
					}
				}
			}
		}
	}

	return false;
}
