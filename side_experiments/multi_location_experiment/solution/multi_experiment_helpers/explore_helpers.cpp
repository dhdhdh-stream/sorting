#include "multi_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 10;
#else
const int EXPLORE_ITERS = 400;
#endif /* MDEBUG */

void MultiExperiment::explore_check_activate(vector<double>& obs,
											 MultiExperimentHistory* history,
											 SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->num_instances_until_target--;
		if (history->existing_predicted.size() == 0
				&& this->num_instances_until_target <= 0) {
			this->existing_network->activate(obs);
			history->existing_predicted.push_back(
				this->existing_network->output->acti_vals[0]);

			geometric_distribution<int> geo_distribution(0.3);
			int new_num_steps = 1 + geo_distribution(generator);

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)this->scope_context->child_scopes.size(); c_index++) {
				if (this->scope_context->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					history->curr_step_types.push_back(STEP_TYPE_SCOPE);
					history->curr_actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					history->curr_scopes.push_back(this->scope_context->child_scopes[child_index]);
				} else {
					history->curr_step_types.push_back(STEP_TYPE_ACTION);

					history->curr_actions.push_back(-1);

					history->curr_scopes.push_back(NULL);
				}
			}

			MultiExperimentState* new_experiment_state = new MultiExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void MultiExperiment::explore_step(vector<double>& obs,
								   int& action,
								   bool& is_next,
								   bool& fetch_action,
								   SolutionWrapper* wrapper) {
	MultiExperimentState* experiment_state = (MultiExperimentState*)wrapper->experiment_context.back();
	MultiExperimentHistory* history = (MultiExperimentHistory*)wrapper->experiment_histories[this];

	if (experiment_state->step_index >= (int)history->curr_step_types.size()) {
		/**
		 * - wrapper->node_context.back() unchanged
		 */

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (history->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(history->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(history->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void MultiExperiment::explore_set_action(int action,
										 SolutionWrapper* wrapper) {
	MultiExperimentState* experiment_state = (MultiExperimentState*)wrapper->experiment_context.back();
	MultiExperimentHistory* history = (MultiExperimentHistory*)wrapper->experiment_histories[this];

	history->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void MultiExperiment::explore_exit_step(SolutionWrapper* wrapper) {
	MultiExperimentState* experiment_state = (MultiExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void MultiExperiment::explore_backprop(double target_val,
									   MultiExperimentHistory* history,
									   SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_hit);
		this->num_instances_until_target = until_distribution(generator);

		if (history->existing_predicted.size() != 0) {
			double curr_surprise = target_val - history->existing_predicted[0];

			#if defined(MDEBUG) && MDEBUG
			if (curr_surprise > this->best_surprise || true) {
			#else
			if (curr_surprise > this->best_surprise) {
			#endif /* MDEBUG */
				this->best_surprise = curr_surprise;
				this->best_step_types = history->curr_step_types;
				this->best_actions = history->curr_actions;
				this->best_scopes = history->curr_scopes;
			}

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->best_surprise >= 0.0) {
				#endif /* MDEBUG */
					this->total_num_instances = 0;
					this->start_iter = wrapper->iters_since_update;

					this->state = MULTI_EXPERIMENT_STATE_TRAIN_NEW;
					this->state_iter = 0;
				} else {
					delete this;
				}
			}
		}
	}
}
