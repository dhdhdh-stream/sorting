#include "signal_experiment.h"

#include <cmath>

#include "scope.h"
#include "signal.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_POSITIVE_NUM_ITERS = 1;
#else
const int MEASURE_POSITIVE_NUM_ITERS = 100;
#endif /* MDEBUG */

bool SignalExperiment::measure_positive_check_signal(
		vector<double>& obs,
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

void SignalExperiment::measure_positive_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	this->existing_positive_scores[this->solution_index].push_back(target_val);

	this->solution_index++;
	if (this->solution_index >= (int)wrapper->positive_solutions.size()) {
		this->solution_index = 0;

		this->state_iter++;
		if (this->state_iter >= MEASURE_POSITIVE_NUM_ITERS) {
			for (int s_index = 0; s_index < (int)wrapper->positive_solutions.size(); s_index++) {
				double sum_vals = 0.0;
				for (int h_index = 0; h_index < (int)this->existing_positive_scores[s_index].size(); h_index++) {
					sum_vals += this->existing_positive_scores[s_index][h_index];
				}
				this->existing_positive_score_averages.push_back(sum_vals / (double)this->existing_positive_scores[s_index].size());

				double sum_variance = 0.0;
				for (int h_index = 0; h_index < (int)this->existing_positive_scores[s_index].size(); h_index++) {
					sum_variance += (this->existing_positive_scores[s_index][h_index] - this->existing_positive_score_averages[s_index])
						* (this->existing_positive_scores[s_index][h_index] - this->existing_positive_score_averages[s_index]);
				}
				this->existing_positive_score_standard_deviations.push_back(sqrt(sum_variance / (double)this->existing_positive_scores[s_index].size()));
			}

			set_actions(wrapper);

			this->state = SIGNAL_EXPERIMENT_STATE_FIND_SAFE;
			this->state_iter = 0;
			this->solution_type = SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT;
		}
	}
}
