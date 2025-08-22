#include "signal_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "explore.h"
#include "globals.h"
#include "helpers.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

// temp
#include "simpler.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_POSITIVE = 10;
const int MIN_NUM_EXPLORE = 40;
const int MAX_NUM_EXPLORE = 80;

const int NUM_POSITIVE_VERIFY = 2;
const int NUM_VERIFY = 2;
#else
const int NUM_POSITIVE = 1000;
const int MIN_NUM_EXPLORE = 4000;
const int MAX_NUM_EXPLORE = 8000;

const int NUM_POSITIVE_VERIFY = 10;
const int NUM_VERIFY = 10;
#endif /* MDEBUG */

void SignalExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	if (is_branch == this->curr_explore->explore_is_branch) {
		this->num_instances_until_target--;
		if (!wrapper->signal_experiment_history->is_hit
				&& this->num_instances_until_target == 0) {
			wrapper->signal_experiment_history->is_hit = true;

			wrapper->signal_experiment_history->scope_history = wrapper->scope_histories.back();

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
	SignalExperimentHistory* history = wrapper->signal_experiment_history;

	uniform_int_distribution<int> until_distribution(0, (int)this->scope_context->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (history->is_hit) {
		double inner_target_val;
		if (history->signal_needed_from == NULL) {
			inner_target_val = target_val;
		} else {
			inner_target_val = calc_signal(history->signal_needed_from);
		}

		this->curr_explore->explore_node->experiment = NULL;
		#if defined(MDEBUG) && MDEBUG
		if (inner_target_val > this->existing_average_outer_signal || rand()%2 == 0) {
		#else
		if (inner_target_val > this->existing_average_outer_signal) {
		#endif /* MDEBUG */
			this->positive_count++;
			if (this->positive_pre_obs_histories.size() < NUM_POSITIVE) {
				this->positive_pre_obs_histories.push_back(history->scope_history->signal_pre_obs);
				this->positive_post_obs_histories.push_back(history->scope_history->signal_post_obs);
				this->positive_target_val_histories.push_back(target_val);
				this->positive_explores.push_back(this->curr_explore);
				this->curr_explore = NULL;
			} else {
				if (this->verify_positive_pre_obs_histories.size() < NUM_POSITIVE_VERIFY) {
					this->verify_positive_pre_obs_histories.push_back(history->scope_history->signal_pre_obs);
					this->verify_positive_post_obs_histories.push_back(history->scope_history->signal_post_obs);
					this->verify_positive_target_val_histories.push_back(target_val);
					this->verify_positive_problems.push_back(wrapper->problem->copy_snapshot());
				}
			}

			Simpler* simpler = (Simpler*)wrapper->problem;
			if (simpler->world[2] > 0) {
				this->true_positive_count++;
			}
		}
		if (this->curr_explore != NULL) {
			delete this->curr_explore;
			this->curr_explore = NULL;
		}

		this->total_count++;
		if (this->pre_obs_histories.size() < MAX_NUM_EXPLORE) {
			this->pre_obs_histories.push_back(history->scope_history->signal_pre_obs);
			this->post_obs_histories.push_back(history->scope_history->signal_post_obs);
			this->target_val_histories.push_back(target_val);
		} else {
			if (this->positive_pre_obs_histories.size() == 0) {
				this->state = SIGNAL_EXPERIMENT_STATE_DONE;
				return;
			}

			if (this->verify_pre_obs_histories.size() < NUM_VERIFY) {
				this->verify_pre_obs_histories.push_back(history->scope_history->signal_pre_obs);
				this->verify_post_obs_histories.push_back(history->scope_history->signal_post_obs);
				this->verify_target_val_histories.push_back(target_val);
				this->verify_problems.push_back(wrapper->problem->copy_snapshot());
			}
		}

		// if (this->positive_pre_obs_histories.size() >= NUM_POSITIVE
		// 		&& this->pre_obs_histories.size() >= MIN_NUM_EXPLORE) {
		if (this->positive_pre_obs_histories.size() >= NUM_POSITIVE
				&& this->verify_positive_pre_obs_histories.size() >= NUM_POSITIVE_VERIFY
				&& this->pre_obs_histories.size() >= MIN_NUM_EXPLORE
				&& this->verify_pre_obs_histories.size() >= NUM_VERIFY) {
			// temp
			cout << "pre_actions:";
			for (int a_index = 0; a_index < (int)this->pre_actions.size(); a_index++) {
				cout << " " << this->pre_actions[a_index];
			}
			cout << endl;
			cout << "post_actions:";
			for (int a_index = 0; a_index < (int)this->post_actions.size(); a_index++) {
				cout << " " << this->post_actions[a_index];
			}
			cout << endl;

			cout << "this->positive_count: " << this->positive_count << endl;
			cout << "this->true_positive_count: " << this->true_positive_count << endl;
			cout << "this->total_count: " << this->total_count << endl;

			create_reward_signal_helper(wrapper);

			this->state = SIGNAL_EXPERIMENT_STATE_DONE;
		} else {
			this->curr_explore = create_explore(this->scope_context);
			this->curr_explore->explore_node->experiment = this;
		}
	}
}
