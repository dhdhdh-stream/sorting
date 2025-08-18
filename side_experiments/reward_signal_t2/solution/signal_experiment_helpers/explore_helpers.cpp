#include "signal_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "explore.h"
#include "globals.h"
#include "helpers.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_POSITIVE = 10;
const int MIN_NUM_EXPLORE = 40;
const int MAX_NUM_EXPLORE = 80;
#else
const int NUM_POSITIVE = 1000;
const int MIN_NUM_EXPLORE = 4000;
const int MAX_NUM_EXPLORE = 8000;
#endif /* MDEBUG */

void SignalExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	if (is_branch == this->curr_explore->explore_is_branch) {
		wrapper->signal_experiment_history->is_hit = true;

		/**
		 * - start from layer above
		 */
		for (int l_index = (int)wrapper->scope_histories.size()-2; l_index >= 0; l_index--) {
			if (wrapper->scope_histories[l_index]->scope->signals.size() > 0) {
				wrapper->signal_experiment_history->signal_needed_from = wrapper->scope_histories[l_index];
				break;
			}
		}

		SignalExperimentState* new_experiment_state = new SignalExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void SignalExperiment::experiment_step(std::vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	SignalExperimentState* experiment_state = (SignalExperimentState*)wrapper->experiment_context.back();
	if (experiment_state->step_index >= (int)this->curr_explore->step_types.size()) {
		wrapper->node_context.back() = this->curr_explore->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_explore->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_explore->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_explore->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void SignalExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	SignalExperimentState* experiment_state = (SignalExperimentState*)wrapper->experiment_context.back();

	this->curr_explore->actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void SignalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	SignalExperimentState* experiment_state = (SignalExperimentState*)wrapper->experiment_context.back();

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void SignalExperiment::explore_backprop(
		double target_val,
		SignalExperimentHistory* history) {
	if (history->is_hit) {
		double inner_target_val;
		if (history->signal_needed_from == NULL) {
			inner_target_val = target_val;
		} else {
			inner_target_val = calc_signal(history->signal_needed_from);
		}

		this->curr_explore->explore_node->experiment = NULL;
		if (inner_target_val > this->existing_average_outer_signal) {
			this->positive_pre_obs_histories.push_back(history->pre_obs);
			this->positive_post_obs_histories.push_back(history->post_obs);
			this->positive_target_val_histories.push_back(target_val);
			this->positive_explores.push_back(this->curr_explore);
		} else {
			delete this->curr_explore;
		}
		this->curr_explore = NULL;

		if (this->pre_obs_histories.size() < MAX_NUM_EXPLORE) {
			this->pre_obs_histories.push_back(history->pre_obs);
			this->post_obs_histories.push_back(history->post_obs);
			this->target_val_histories.push_back(target_val);
		}

		if (this->positive_pre_obs_histories.size() >= NUM_POSITIVE
				&& this->pre_obs_histories.size() >= MIN_NUM_EXPLORE) {
			create_reward_signal_helper();

			this->state = SIGNAL_EXPERIMENT_STATE_DONE;
		} else {
			this->curr_explore = create_explore(this->scope_context);
			this->curr_explore->explore_node->experiment = this;
		}
	}
}
