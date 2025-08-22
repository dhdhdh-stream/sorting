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

void SignalExperiment::backprop(double target_val,
								SolutionWrapper* wrapper) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		if (this->new_scope_histories.size() < MEASURE_ITERS) {
			this->new_scope_histories.push_back(wrapper->scope_histories[0]);
			this->new_target_val_histories.push_back(target_val);
		} else {
			delete wrapper->scope_histories[0];
		}

		find_safe_backprop(target_val);

		break;
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 wrapper);

		delete wrapper->scope_histories[0];

		break;
	}
}
