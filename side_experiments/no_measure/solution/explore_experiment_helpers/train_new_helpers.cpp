#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
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

void ExploreExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];

	if (experiment_state->step_index == 0) {
		if (wrapper->should_explore) {
			this->num_instances_until_target--;
			if (this->num_instances_until_target <= 0) {
				history->obs_histories.push_back(obs);

				uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
				this->num_instances_until_target = until_distribution(generator);
			} else {
				delete experiment_state;
				wrapper->experiment_context.back() = NULL;
				return;
			}
		} else {
			history->obs_histories.push_back(obs);

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		}
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ExploreExperiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		if (history->obs_histories.size() > 0) {
			uniform_int_distribution<int> sample_distribution(0, history->obs_histories.size()-1);
			int sample_index = sample_distribution(generator);
			this->new_obs_histories.push_back(history->obs_histories[sample_index]);
			this->new_target_val_histories.push_back(target_val);

			this->state_iter++;
			if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
				uniform_int_distribution<int> existing_train_distribution(0, this->existing_obs_histories.size()-1);

				for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
					int rand_index = existing_train_distribution(generator);

					this->existing_network->activate(this->existing_obs_histories[rand_index]);

					double error = this->existing_target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

					this->existing_network->backprop(error);
				}

				uniform_int_distribution<int> new_train_distribution(0, this->new_obs_histories.size()-1);

				this->new_network = new Network(this->new_obs_histories[0].size());
				double hidden_1_average_max_update = 0.0;
				double hidden_2_average_max_update = 0.0;
				double hidden_3_average_max_update = 0.0;
				double output_average_max_update = 0.0;
				for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
					int rand_index = new_train_distribution(generator);

					this->new_network->activate(this->new_obs_histories[rand_index]);

					double error = this->new_target_val_histories[rand_index] - this->new_network->output->acti_vals[0];

					this->new_network->init_backprop(error,
													 hidden_1_average_max_update,
													 hidden_2_average_max_update,
													 hidden_3_average_max_update,
													 output_average_max_update);
				}

				this->sum_existing_scores = 0.0;
				this->existing_count = 0;
				this->sum_new_scores = 0.0;
				this->new_count = 0;

				this->start_iter = wrapper->iter;

				this->state = EXPLORE_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			}
		}
	} else {
		uniform_int_distribution<int> sample_distribution(0, history->obs_histories.size()-1);
		int sample_index = sample_distribution(generator);
		this->existing_obs_histories.push_back(history->obs_histories[sample_index]);
		this->existing_target_val_histories.push_back(target_val);
	}
}
