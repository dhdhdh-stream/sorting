#include "helpers.h"

#include <iostream>

#include "constants.h"
#include "scope.h"
#include "signal.h"
#include "signal_experiment.h"
#include "signal_instance.h"
#include "signal_network.h"
#include "solution_wrapper.h"

using namespace std;

void calc_signal(vector<vector<double>>& pre_obs_histories,
				 vector<vector<double>>& post_obs_histories,
				 vector<SignalInstance*>& instances,
				 bool& is_match,
				 double& val) {
	for (int s_index = 0; s_index < (int)instances.size(); s_index++) {
		vector<double> input_vals(instances[s_index]->match_input_is_pre.size());
		for (int i_index = 0; i_index < (int)instances[s_index]->match_input_is_pre.size(); i_index++) {
			if (instances[s_index]->match_input_is_pre[i_index]) {
				input_vals[i_index] = pre_obs_histories[
					instances[s_index]->match_input_indexes[i_index]][instances[s_index]->match_input_obs_indexes[i_index]];
			} else {
				input_vals[i_index] = post_obs_histories[
					instances[s_index]->match_input_indexes[i_index]][instances[s_index]->match_input_obs_indexes[i_index]];
			}
		}
		instances[s_index]->match_network->activate(input_vals);
		#if defined(MDEBUG) && MDEBUG
		if (rand()%3 == 0) {
		#else
		if (instances[s_index]->match_network->output->acti_vals[0] >= MATCH_WEIGHT) {
		#endif /* MDEBUG */
			vector<double> input_vals(instances[s_index]->score_input_is_pre.size());
			for (int i_index = 0; i_index < (int)instances[s_index]->score_input_is_pre.size(); i_index++) {
				if (instances[s_index]->score_input_is_pre[i_index]) {
					input_vals[i_index] = pre_obs_histories[
						instances[s_index]->score_input_indexes[i_index]][instances[s_index]->score_input_obs_indexes[i_index]];
				} else {
					input_vals[i_index] = post_obs_histories[
						instances[s_index]->score_input_indexes[i_index]][instances[s_index]->score_input_obs_indexes[i_index]];
				}
			}
			instances[s_index]->score_network->activate(input_vals);

			is_match = true;
			val = instances[s_index]->score_network->output->acti_vals[0];
			return;
		}
	}

	is_match = false;
	return;
}

double calc_signal(vector<vector<double>>& pre_obs_histories,
				   vector<vector<double>>& post_obs_histories,
				   vector<SignalInstance*>& potential_signals,
				   double potential_miss_average_guess) {
	for (int s_index = 0; s_index < (int)potential_signals.size(); s_index++) {
		vector<double> input_vals(potential_signals[s_index]->match_input_is_pre.size());
		for (int i_index = 0; i_index < (int)potential_signals[s_index]->match_input_is_pre.size(); i_index++) {
			if (potential_signals[s_index]->match_input_is_pre[i_index]) {
				input_vals[i_index] = pre_obs_histories[
					potential_signals[s_index]->match_input_indexes[i_index]][potential_signals[s_index]->match_input_obs_indexes[i_index]];
			} else {
				input_vals[i_index] = post_obs_histories[
					potential_signals[s_index]->match_input_indexes[i_index]][potential_signals[s_index]->match_input_obs_indexes[i_index]];
			}
		}
		potential_signals[s_index]->match_network->activate(input_vals);
		#if defined(MDEBUG) && MDEBUG
		if (rand()%3 == 0) {
		#else
		if (potential_signals[s_index]->match_network->output->acti_vals[0] >= MATCH_WEIGHT) {
		#endif /* MDEBUG */
			vector<double> input_vals(potential_signals[s_index]->score_input_is_pre.size());
			for (int i_index = 0; i_index < (int)potential_signals[s_index]->score_input_is_pre.size(); i_index++) {
				if (potential_signals[s_index]->score_input_is_pre[i_index]) {
					input_vals[i_index] = pre_obs_histories[
						potential_signals[s_index]->score_input_indexes[i_index]][potential_signals[s_index]->score_input_obs_indexes[i_index]];
				} else {
					input_vals[i_index] = post_obs_histories[
						potential_signals[s_index]->score_input_indexes[i_index]][potential_signals[s_index]->score_input_obs_indexes[i_index]];
				}
			}
			potential_signals[s_index]->score_network->activate(input_vals);
			return potential_signals[s_index]->score_network->output->acti_vals[0];
		}
	}

	return potential_miss_average_guess;
}

double calc_signal(ScopeHistory* signal_needed_from,
				   SolutionWrapper* wrapper) {
	Scope* scope = signal_needed_from->scope;
	Signal* signal = wrapper->signals[scope->id];
	return calc_signal(signal_needed_from->signal_pre_obs,
					   signal_needed_from->signal_post_obs,
					   signal->instances,
					   signal->default_guess);
}

bool check_signal(vector<double>& obs,
				  int& action,
				  bool& is_next,
				  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();
	Scope* scope = scope_history->scope;
	map<int, Signal*>::iterator it = wrapper->signals.find(scope->id);
	if (it != wrapper->signals.end()) {
		/**
		 * - check pre
		 */
		if (scope_history->node_histories.size() == 0) {
			scope_history->signal_pre_obs.push_back(obs);

			if (scope_history->signal_pre_obs.size() <= it->second->signal_pre_actions.size()) {
				action = it->second->signal_pre_actions[scope_history->signal_pre_obs.size()-1];
				is_next = true;

				wrapper->num_actions++;

				return true;
			}
		}

		/**
		 * - check post
		 */
		if (wrapper->node_context.back() == NULL
				&& (wrapper->experiment_context.size() == 0
					|| wrapper->experiment_context.back() == NULL)) {
			scope_history->signal_post_obs.push_back(obs);

			if (scope_history->signal_post_obs.size() <= it->second->signal_post_actions.size()) {
				action = it->second->signal_post_actions[scope_history->signal_post_obs.size()-1];
				is_next = true;

				wrapper->num_actions++;

				return true;
			}
		}
	}

	return false;
}

bool signal_experiment_check_signal(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();
	Scope* scope = scope_history->scope;

	if (scope->id == wrapper->signal_experiment->scope_context_id) {
		return wrapper->signal_experiment->check_signal(
			obs,
			action,
			is_next,
			wrapper);
	} else {
		map<int, Signal*>::iterator it = wrapper->signals.find(scope->id);
		if (it != wrapper->signals.end()) {
			/**
			 * - check pre
			 */
			if (scope_history->node_histories.size() == 0) {
				scope_history->signal_pre_obs.push_back(obs);

				if (scope_history->signal_pre_obs.size() <= it->second->signal_pre_actions.size()) {
					action = it->second->signal_pre_actions[scope_history->signal_pre_obs.size()-1];
					is_next = true;

					wrapper->num_actions++;

					return true;
				}
			}

			/**
			 * - check post
			 */
			if (wrapper->node_context.back() == NULL
					&& (wrapper->experiment_context.size() == 0
						|| wrapper->experiment_context.back() == NULL)) {
				scope_history->signal_post_obs.push_back(obs);

				if (scope_history->signal_post_obs.size() <= it->second->signal_post_actions.size()) {
					action = it->second->signal_post_actions[scope_history->signal_post_obs.size()-1];
					is_next = true;

					wrapper->num_actions++;

					return true;
				}
			}
		}
	}

	return false;
}
