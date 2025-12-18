#include "solution_helpers.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_experiment.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void add_existing_samples_helper(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	if ((int)scope->existing_pre_obs.size() < EXISTING_MAX_SAMPLES) {
		scope->existing_pre_obs.push_back(scope_history->pre_obs);
		scope->existing_post_obs.push_back(scope_history->post_obs);
	} else {
		scope->existing_pre_obs[scope->existing_index] = scope_history->pre_obs;
		scope->existing_post_obs[scope->existing_index] = scope_history->post_obs;
	}
	scope->existing_index++;
	if (scope->existing_index >= EXISTING_MAX_SAMPLES) {
		scope->existing_index = 0;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			add_existing_samples_helper(scope_node_history->scope_history);
		}
	}
}

void calc_sum_signal_helper(ScopeHistory* scope_history,
							double& sum_signal) {
	Scope* scope = scope_history->scope;

	if (scope_history->scope->signal_experiment != NULL) {
		/**
		 * - re-calc as signal may have changed
		 */

		vector<double> inputs = scope_history->pre_obs;
		inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

		scope->signal_experiment->network->activate(inputs);

		scope_history->signal_val = scope->signal_experiment->network->output->acti_vals[0];

		sum_signal += scope_history->signal_val;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			calc_sum_signal_helper(scope_node_history->scope_history,
								   sum_signal);
		}
	}
}

void calc_sum_signal(SolutionWrapper* wrapper) {
	double sum_signal = 0.0;
	for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
		calc_sum_signal_helper(wrapper->solution->existing_scope_histories[h_index],
							   sum_signal);
	}
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		if (wrapper->solution->scopes[s_index]->signal_experiment != NULL) {
		SignalExperiment* signal_experiment = wrapper->solution->scopes[s_index]->signal_experiment;
			signal_experiment->signal_history.push_back(sum_signal);
			if (signal_experiment->signal_history.size() > 5) {
				signal_experiment->signal_history.erase(
					signal_experiment->signal_history.begin());
			}
			break;
		}
	}
}

void add_explore_samples_helper(ScopeHistory* scope_history,
								vector<ScopeHistory*>& stack_trace,
								double target_val,
								SolutionWrapper* wrapper) {
	Scope* scope = scope_history->scope;

	double sum_vals = target_val - wrapper->solution->curr_score;
	for (int l_index = 0; l_index < (int)stack_trace.size(); l_index++) {
		if (stack_trace[l_index]->signal_initialized) {
			sum_vals += stack_trace[l_index]->signal_val;
		}
	}

	if ((int)scope->explore_pre_obs.size() < EXPLORE_MAX_SAMPLES) {
		scope->explore_pre_obs.push_back(scope_history->pre_obs);
		scope->explore_post_obs.push_back(scope_history->post_obs);
		scope->explore_target_vals.push_back(sum_vals);
	} else {
		uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.size()-1);
		int index = distribution(generator);
		scope->explore_pre_obs[index] = scope_history->pre_obs;
		scope->explore_post_obs[index] = scope_history->post_obs;
		scope->explore_target_vals[index] = sum_vals;
	}

	if (scope->signal_experiment != NULL
			&& !scope_history->signal_initialized) {
		scope_history->signal_initialized = true;

		vector<double> inputs = scope_history->pre_obs;
		inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

		scope->signal_experiment->network->activate(inputs);

		scope_history->signal_val = scope->signal_experiment->network->output->acti_vals[0];
	}

	stack_trace.push_back(scope_history);

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			if (scope_node_history->scope_history->has_explore) {
				add_explore_samples_helper(scope_node_history->scope_history,
										   stack_trace,
										   target_val,
										   wrapper);
			}
		}
	}

	stack_trace.pop_back();
}
