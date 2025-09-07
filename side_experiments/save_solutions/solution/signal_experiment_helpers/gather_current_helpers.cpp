#include "signal_experiment.h"

#include <iostream>

#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int GATHER_CURRENT_NUM_ITERS = 19;
#else
const int GATHER_CURRENT_NUM_ITERS = 3900;
#endif /* MDEBUG */

bool SignalExperiment::gather_current_check_signal(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	/**
	 * - check pre
	 */
	if (scope_history->node_histories.size() == 0) {
		if (scope_history->signal_pre_obs.size() == 0) {
			SignalExperimentInstanceHistory* new_instance_history = new SignalExperimentInstanceHistory();
			new_instance_history->scope_history = scope_history;
			/**
			 * - start from layer above
			 */
			for (int l_index = (int)wrapper->scope_histories.size()-2; l_index >= 0; l_index--) {
				map<int, Signal*>::iterator it = wrapper->signals.find(wrapper->scope_histories[l_index]->scope->id);
				if (it != wrapper->signals.end()) {
					new_instance_history->signal_needed_from = wrapper->scope_histories[l_index];
					break;
				}
			}
			wrapper->signal_experiment_instance_histories.push_back(new_instance_history);
		}

		scope_history->signal_pre_obs.push_back(obs);

		if (scope_history->signal_pre_obs.size() <= this->pre_actions.size()) {
			action = this->pre_actions[scope_history->signal_pre_obs.size()-1];
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

		if (scope_history->signal_post_obs.size() <= this->post_actions.size()) {
			action = this->post_actions[scope_history->signal_post_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		}
	}

	return false;
}

void SignalExperiment::gather_current_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)wrapper->signal_experiment_instance_histories.size(); i_index++) {
		SignalExperimentInstanceHistory* instance_history = wrapper->signal_experiment_instance_histories[i_index];

		this->current_pre_obs.push_back(instance_history->scope_history->signal_pre_obs);
		this->current_post_obs.push_back(instance_history->scope_history->signal_post_obs);

		double inner_target_val;
		if (instance_history->signal_needed_from == NULL) {
			inner_target_val = target_val;
		} else {
			inner_target_val = calc_signal(instance_history->signal_needed_from,
										   wrapper);
		}
		this->current_scores.push_back(inner_target_val);
	}

	this->state_iter++;
	if (this->state_iter >= GATHER_CURRENT_NUM_ITERS) {
		// temp
		cout << "SIGNAL_EXPERIMENT_STATE_EXPLORE" << endl;
		this->state = SIGNAL_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_POSITIVE;
		this->solution_index = 0;

		set_explore(wrapper);

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}
