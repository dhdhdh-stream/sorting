#include "outer_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 10;
#else
const int EXPLORE_ITERS = 200;
#endif /* MDEBUG */

void OuterExperiment::explore_check_activate(SolutionWrapper* wrapper) {
	OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;

	this->num_instances_until_target--;
	if (history->existing_predicted_trues.size() == 0
			&& this->num_instances_until_target <= 0) {
		this->curr_new_scope = outer_create_new_scope(wrapper);
		this->curr_step_types.push_back(STEP_TYPE_SCOPE);
		this->curr_actions.push_back(-1);
		this->curr_scopes.push_back(this->curr_new_scope);

		OuterExperimentState* new_experiment_state = new OuterExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void OuterExperiment::explore_step(vector<double>& obs,
								   int& action,
								   bool& is_next,
								   bool& fetch_action,
								   SolutionWrapper* wrapper) {
	OuterExperimentState* experiment_state = (OuterExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;

		this->existing_true_network->activate(obs);
		history->existing_predicted_trues.push_back(
			this->existing_true_network->output->acti_vals[0]);
	}

	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void OuterExperiment::explore_exit_step(SolutionWrapper* wrapper) {
	OuterExperimentState* experiment_state = (OuterExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void OuterExperiment::explore_backprop(double target_val,
									   SolutionWrapper* wrapper) {
	OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;

	uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
	this->num_instances_until_target = until_distribution(generator);

	if (history->existing_predicted_trues.size() != 0) {
		// double curr_surprise = target_val - history->existing_predicted_trues[0];

		double existing_result = get_existing_result(wrapper);
		double curr_surprise = target_val - existing_result;

		#if defined(MDEBUG) && MDEBUG
		if (curr_surprise > this->best_surprise || true) {
		#else
		if (curr_surprise > this->best_surprise) {
		#endif /* MDEBUG */
			this->best_surprise = curr_surprise;
			if (this->best_new_scope != NULL) {
				delete this->best_new_scope;
			}
			this->best_new_scope = this->curr_new_scope;
			this->curr_new_scope = NULL;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_scopes = this->curr_scopes;
		}

		if (this->curr_new_scope != NULL) {
			delete this->curr_new_scope;
			this->curr_new_scope = NULL;
		}
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_surprise >= 0.0) {
			#endif /* MDEBUG */
				this->state = OUTER_EXPERIMENT_STATE_TRAIN_NEW;
				this->state_iter = 0;
			} else {
				wrapper->curr_outer_experiment = NULL;
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
