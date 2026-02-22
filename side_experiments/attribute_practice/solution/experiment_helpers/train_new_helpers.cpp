#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "build_network.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void Experiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void Experiment::train_new_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

		history->stack_traces.push_back(wrapper->scope_histories);

		this->new_obs_histories.push_back(obs);
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[experiment_state->step_index];

			ScopeHistory* scope_history = wrapper->scope_histories.back();

			ScopeNodeHistory* history = new ScopeNodeHistory(scope_node);
			history->index = (int)scope_history->node_histories.size();
			scope_history->node_histories[scope_node->id] = history;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
			vector<double> curr_target_vals;
			vector<bool> curr_target_vals_is_on;

			curr_target_vals.push_back(target_val);
			curr_target_vals_is_on.push_back(true);

			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[i_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->signal->nodes.size() > 0) {
					vector<double> input;
					input.insert(input.end(), scope_history->pre_obs_history.begin(), scope_history->pre_obs_history.end());
					input.insert(input.end(), scope_history->post_obs_history.begin(), scope_history->post_obs_history.end());

					double new_signal = scope->signal->activate(input);

					curr_target_vals.push_back(new_signal);
					curr_target_vals_is_on.push_back(true);
				} else {
					curr_target_vals.push_back(0.0);
					curr_target_vals_is_on.push_back(false);
				}
			}

			this->new_target_vals.push_back(curr_target_vals);
			this->new_target_vals_is_on.push_back(curr_target_vals_is_on);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->new_target_vals.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_target_vals.begin(), this->new_target_vals.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_target_vals_is_on.begin(), this->new_target_vals_is_on.end(), generator_copy);
			}

			double best_val_average = numeric_limits<double>::lowest();
			for (int l_index = 0; l_index < (int)this->existing_networks.size(); l_index++) {
				if (this->existing_networks[l_index] != NULL) {
					train_and_eval_helper(l_index,
										  best_val_average);
				}
			}

			this->new_branch_node = new BranchNode();
			this->new_branch_node->parent = this->scope_context;
			this->new_branch_node->id = this->scope_context->node_counter + (int)this->best_step_types.size();

			this->sum_true = 0.0;
			this->hit_count = 0;

			this->total_count = 0;
			this->total_sum_true = 0.0;

			this->state = EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		}
	}
}
