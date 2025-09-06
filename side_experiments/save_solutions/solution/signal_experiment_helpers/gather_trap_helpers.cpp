#include "signal_experiment.h"

#include "helpers.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int GATHER_TRAP_NUM_ITERS = 1;
#else
const int GATHER_TRAP_NUM_ITERS = 100;
#endif /* MDEBUG */

bool SignalExperiment::gather_trap_check_signal(
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

void SignalExperiment::gather_trap_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)wrapper->signal_experiment_instance_histories.size(); i_index++) {
		SignalExperimentInstanceHistory* instance_history = wrapper->signal_experiment_instance_histories[i_index];

		this->trap_pre_obs.push_back(instance_history->scope_history->signal_pre_obs);
		this->trap_post_obs.push_back(instance_history->scope_history->signal_post_obs);

		double inner_target_val;
		if (instance_history->signal_needed_from == NULL) {
			inner_target_val = target_val;
		} else {
			inner_target_val = calc_signal(instance_history->signal_needed_from,
										   wrapper);
		}
		this->trap_scores.push_back(inner_target_val);
	}

	this->solution_index++;
	if (this->solution_index >= (int)wrapper->positive_solutions.size()) {
		this->solution_index = 0;
		this->state_iter++;
		if (this->state_iter >= GATHER_TRAP_NUM_ITERS) {
			this->state = SIGNAL_EXPERIMENT_STATE_GATHER_CURRENT;
			this->state_iter = 0;
			this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT;
		}
	}
}
