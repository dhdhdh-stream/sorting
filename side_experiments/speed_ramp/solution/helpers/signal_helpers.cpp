#include "helpers.h"

#include <iostream>

#include "constants.h"
#include "default_signal.h"
#include "explore_experiment.h"
#include "scope.h"
#include "signal.h"
#include "signal_experiment.h"
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
									  bool& fetch_action,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();
	Scope* scope = scope_history->scope;

	if (scope->signal_experiment == NULL) {
		/**
		 * - assuming no signal if no SignalExperiment
		 */
		return false;
	} else {
		if (scope->signal_experiment_history == NULL) {
			scope->signal_experiment_history = new SignalExperimentHistory(scope->signal_experiment);
			wrapper->signal_experiment_histories.push_back({scope->signal_experiment, scope->signal_experiment_history});
		}

		if (scope->signal_experiment_history->is_on) {
			/**
			 * - check pre
			 */
			if (scope_history->node_histories.size() == 0) {
				scope_history->signal_pre_obs.push_back(obs);

				if (scope_history->signal_pre_obs.size() <= scope->signal_experiment->pre_actions.size()) {
					if (!scope->signal_experiment->pre_action_initialized[scope_history->signal_pre_obs.size()-1]) {
						is_next = true;
						fetch_action = true;

						wrapper->num_actions++;

						SignalExperimentState* new_experiment_state = new SignalExperimentState(scope->signal_experiment);
						new_experiment_state->is_pre = true;
						new_experiment_state->index = scope_history->signal_pre_obs.size()-1;
						wrapper->experiment_context.back() = new_experiment_state;
					} else {
						action = scope->signal_experiment->pre_actions[scope_history->signal_pre_obs.size()-1];
						is_next = true;

						wrapper->num_actions++;
					}

					return true;
				}
			}

			/**
			 * - check post
			 */
			if (wrapper->node_context.back() == NULL
					&& wrapper->experiment_context.back() == NULL) {
				scope_history->signal_post_obs.push_back(obs);

				if (scope_history->signal_post_obs.size() <= scope->signal_experiment->post_actions.size()) {
					if (!scope->signal_experiment->post_action_initialized[scope_history->signal_post_obs.size()-1]) {
						is_next = true;
						fetch_action = true;

						wrapper->num_actions++;

						SignalExperimentState* new_experiment_state = new SignalExperimentState(scope->signal_experiment);
						new_experiment_state->is_pre = false;
						new_experiment_state->index = scope_history->signal_post_obs.size()-1;
						wrapper->experiment_context.back() = new_experiment_state;
					} else {
						action = scope->signal_experiment->post_actions[scope_history->signal_post_obs.size()-1];
						is_next = true;

						wrapper->num_actions++;
					}

					return true;
				} else {
					scope->signal_experiment_history->pre_obs.push_back(scope_history->signal_pre_obs);
					scope->signal_experiment_history->post_obs.push_back(scope_history->signal_post_obs);
					scope->signal_experiment_history->inner_is_explore.push_back(wrapper->has_explore);

					scope->signal_experiment_history->signal_is_set.push_back(false);
					scope->signal_experiment_history->signal_vals.push_back(0.0);
					scope->signal_experiment_history->outer_is_explore.push_back(false);

					/**
					 * - start from layer above
					 */
					for (int l_index = (int)wrapper->scope_histories.size()-2; l_index >= 0; l_index--) {
						Scope* scope = wrapper->scope_histories[l_index]->scope;
						if (scope->signal_experiment_history != NULL) {
							/**
							 * - assuming no signal if no SignalExperiment
							 */
							if (!scope->signal_experiment_history->is_on) {
								wrapper->scope_histories[l_index]->signal_experiment_callbacks
									.push_back(scope->signal_experiment_history);
								break;
							}
						}
					}
				}
			}
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
							|| scope_history->signal_experiment_callbacks.size() > 0) {
						double signal = calc_signal(scope_history);
						for (int e_index = 0; e_index < (int)scope_history->explore_experiment_callbacks.size(); e_index++) {
							scope_history->explore_experiment_callbacks[e_index]->signal_is_set.back() = true;
							scope_history->explore_experiment_callbacks[e_index]->signal_vals.back() = signal;
						}
						for (int e_index = 0; e_index < (int)scope_history->signal_experiment_callbacks.size(); e_index++) {
							scope_history->signal_experiment_callbacks[e_index]->signal_is_set.back() = true;
							scope_history->signal_experiment_callbacks[e_index]->signal_vals.back() = signal;
							scope_history->signal_experiment_callbacks[e_index]->outer_is_explore.back() = wrapper->has_explore;
						}
					}

					scope->signal_experiment_history->pre_obs.push_back(scope_history->signal_pre_obs);
					scope->signal_experiment_history->post_obs.push_back(scope_history->signal_post_obs);
					scope->signal_experiment_history->inner_is_explore.push_back(wrapper->has_explore);

					scope->signal_experiment_history->signal_is_set.push_back(false);
					scope->signal_experiment_history->signal_vals.push_back(0.0);
					scope->signal_experiment_history->outer_is_explore.push_back(false);

					/**
					 * - start from layer above
					 */
					for (int l_index = (int)wrapper->scope_histories.size()-2; l_index >= 0; l_index--) {
						Scope* scope = wrapper->scope_histories[l_index]->scope;
						if (scope->signal_experiment_history != NULL) {
							/**
							 * - assuming no signal if no SignalExperiment
							 */
							if (!scope->signal_experiment_history->is_on) {
								wrapper->scope_histories[l_index]->signal_experiment_callbacks
									.push_back(scope->signal_experiment_history);
								break;
							}
						}
					}
				}
			}
		}
	}

	return false;
}
