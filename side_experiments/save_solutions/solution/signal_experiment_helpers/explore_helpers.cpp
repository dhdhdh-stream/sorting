#include "signal_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 40;
#else
const int EXPLORE_ITERS = 4000;
#endif /* MDEBUG */

bool SignalExperiment::explore_check_signal(vector<double>& obs,
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

void SignalExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	if (is_branch == this->explore_is_branch) {
		this->num_instances_until_target--;
		if (wrapper->signal_experiment_instance_histories.size() == 0
				&& this->num_instances_until_target == 0) {
			SignalExperimentInstanceHistory* new_instance_history = new SignalExperimentInstanceHistory();
			new_instance_history->scope_history = wrapper->scope_histories.back();
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

			SignalExperimentState* new_experiment_state = new SignalExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void SignalExperiment::experiment_step(std::vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	SignalExperimentState* experiment_state = (SignalExperimentState*)wrapper->experiment_context.back();
	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void SignalExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	SignalExperimentState* experiment_state = (SignalExperimentState*)wrapper->experiment_context.back();

	this->actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void SignalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	SignalExperimentState* experiment_state = (SignalExperimentState*)wrapper->experiment_context.back();
	experiment_state->step_index++;
}

void SignalExperiment::explore_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	if (wrapper->signal_experiment_instance_histories.size() > 0) {
		SignalExperimentInstanceHistory* instance_history = wrapper->signal_experiment_instance_histories[0];

		double inner_target_val;
		if (instance_history->signal_needed_from == NULL) {
			inner_target_val = target_val;
		} else {
			inner_target_val = calc_signal(instance_history->signal_needed_from,
										   wrapper);
		}

		this->explore_node->experiment = NULL;
		if (this->new_scope != NULL) {
			delete this->new_scope;
			this->new_scope = NULL;
		}
		this->step_types.clear();
		this->actions.clear();
		this->scopes.clear();

		this->explore_pre_obs.push_back(instance_history->scope_history->signal_pre_obs);
		this->explore_post_obs.push_back(instance_history->scope_history->signal_post_obs);
		this->explore_scores.push_back(inner_target_val);

		this->solution_index++;
		if (this->solution_index >= (int)wrapper->solutions.size()) {
			this->solution_index = 0;
			this->state_iter++;

			if (this->state_iter >= EXPLORE_ITERS) {
				create_reward_signal_helper(wrapper);
				this->state = SIGNAL_EXPERIMENT_STATE_DONE;
				return;
			}
		}

		set_explore(wrapper);

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}
