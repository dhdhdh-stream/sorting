#include "signal_eval_experiment.h"

#include "globals.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

const int MAX_SAMPLES = 8000;
const int EXPLORE_TARGET_SAMPLES = 4000;

bool SignalEvalExperiment::check_signal_activate(
		vector<double>& obs,
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
	if (wrapper->node_context.back() == NULL) {
		scope_history->signal_post_obs.push_back(obs);

		if (scope_history->signal_post_obs.size() <= this->post_actions.size()) {
			action = this->post_actions[scope_history->signal_post_obs.size()-1];
			is_next = true;

			wrapper->num_actions++;

			return true;
		} else {
			SignalEvalExperimentHistory* history;
			map<SignalEvalExperiment*, SignalEvalExperimentHistory*>::iterator it =
				wrapper->signal_eval_histories.find(this);
			if (it == wrapper->signal_eval_histories.end()) {
				history = new SignalEvalExperimentHistory();
				wrapper->signal_eval_histories[this] = history;
			} else {
				history = it->second;
			}

			history->pre_obs.push_back(scope_history->signal_pre_obs);
			history->post_obs.push_back(scope_history->signal_post_obs);
			history->inner_is_explore.push_back(wrapper->has_explore);

			history->signal_is_set.push_back(false);
			history->signal_vals.push_back(0.0);
			history->outer_is_explore.push_back(false);

			/**
			 * - start from layer above
			 */
			for (int l_index = (int)wrapper->scope_histories.size()-2; l_index >= 0; l_index--) {
				if (wrapper->scope_histories[l_index]->scope->signals.size() > 0) {
					wrapper->scope_histories[l_index]->signal_eval_experiment_callbacks.push_back(history);
					break;
				}
			}
		}
	}

	return false;
}

void SignalEvalExperiment::backprop(double target_val,
									SignalEvalExperimentHistory* history) {
	for (int i_index = 0; i_index < (int)history->pre_obs.size(); i_index++) {
		if (history->inner_is_explore[i_index] == history->outer_is_explore[i_index]) {
			if (history->inner_is_explore[i_index]) {
				this->explore_pre_obs.push_back(history->pre_obs[i_index]);
				this->explore_post_obs.push_back(history->post_obs[i_index]);
				if (history->signal_is_set[i_index]) {
					this->explore_scores.push_back(history->signal_vals[i_index]);
				} else {
					this->explore_scores.push_back(target_val);
				}
			} else {
				if (this->pre_obs.size() >= MAX_SAMPLES) {
					uniform_int_distribution<int> distribution(0, this->pre_obs.size()-1);
					int index = distribution(generator);
					this->pre_obs[index] = history->pre_obs[i_index];
					this->post_obs[index] = history->post_obs[i_index];
					if (history->signal_is_set[i_index]) {
						this->scores[index] = history->signal_vals[i_index];
					} else {
						this->scores[index] = target_val;
					}
				} else {
					this->pre_obs.push_back(history->pre_obs[i_index]);
					this->post_obs.push_back(history->post_obs[i_index]);
					if (history->signal_is_set[i_index]) {
						this->scores.push_back(history->signal_vals[i_index]);
					} else {
						this->scores.push_back(target_val);
					}
				}
			}
		}
	}

	if (this->explore_pre_obs.size() >= EXPLORE_TARGET_SAMPLES) {

	}
}
