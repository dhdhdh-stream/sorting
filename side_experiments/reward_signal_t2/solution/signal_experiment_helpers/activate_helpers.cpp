#include "signal_experiment.h"

#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

bool SignalExperiment::check_signal(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	/**
	 * - check pre
	 */
	if (scope_history->node_histories.size() == 0) {
		SignalExperimentHistory* history = wrapper->signal_experiment_history;

		history->pre_obs.push_back(obs);

		if (history->pre_obs.size() <= this->pre_actions.size()) {
			action = this->pre_actions[history->pre_obs.size()-1];
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
		SignalExperimentHistory* history = wrapper->signal_experiment_history;

		history->post_obs.push_back(obs);

		if (history->post_obs.size() <= this->post_actions.size()) {
			action = this->post_actions[history->post_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		}
	}

	return false;
}

void SignalExperiment::backprop(double target_val,
								SignalExperimentHistory* history) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		find_safe_backprop(target_val);
		break;
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history);
		break;
	}
}
