#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_experiment.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "signal.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 5000;
#endif /* MDEBUG */

const double VERIFY_RATIO = 0.2;

void ExploreExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;

	uniform_int_distribution<int> explore_distribution(0, 9);
	/**
	 * - simply fixed 1-in-10 for single for now
	 */
	if (history->existing_predicted_trues.size() == 0
			&& explore_distribution(generator) == 0) {
		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}

	this->sum_num_instances++;
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;

		this->new_obs_histories.push_back(obs);

		ScopeHistory* scope_history = wrapper->scope_histories.back();

		vector<int> explore_index{(int)scope_history->node_histories.size()-1};
		double signal_val = this->scope_context->pre_signal->activate(
			scope_history,
			explore_index);

		history->existing_predicted_trues.push_back(signal_val);
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
		SolutionWrapper* wrapper) {
	this->total_count++;

	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;
	if (history->existing_predicted_trues.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		if (this->new_obs_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_true_histories.begin(), this->new_true_histories.end(), generator_copy);
			}

			Network* new_network = new Network(this->new_obs_histories[0].size(),
											   NETWORK_SIZE_SMALL);

			int num_train = (1.0 - VERIFY_RATIO) * (double)this->new_obs_histories.size();

			uniform_int_distribution<int> distribution(0, num_train-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = distribution(generator);

				new_network->activate(this->new_obs_histories[rand_index]);

				double error = this->new_true_histories[rand_index] - new_network->output->acti_vals[0];

				new_network->backprop(error);
			}

			double sum_vals = 0.0;
			for (int h_index = num_train; h_index < (int)this->new_obs_histories.size(); h_index++) {
				new_network->activate(this->new_obs_histories[h_index]);

				if (new_network->output->acti_vals[0] >= 0.0) {
					sum_vals += this->new_true_histories[h_index];
				}
			}
			double local_improvement = sum_vals / ((double)this->new_obs_histories.size() - (double)num_train);
			double average_instances_per_run = (double)this->sum_num_instances / (double)this->total_count;
			double global_improvement = average_instances_per_run * local_improvement;

			// cout << "local_improvement: " << local_improvement << endl;
			// cout << "average_instances_per_run: " << average_instances_per_run << endl;
			// cout << "global_improvement: " << global_improvement << endl;

			bool is_success = false;
			if (local_improvement > 0.0) {
				if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
							it != wrapper->solution->train_new_last_scores.end(); it++) {
						if (global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->train_new_last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (wrapper->solution->train_new_last_scores.size() >= NUM_LAST_TRACK) {
						wrapper->solution->train_new_last_scores.pop_front();
					}
					wrapper->solution->train_new_last_scores.push_back(global_improvement);
				} else {
					wrapper->solution->train_new_last_scores.push_back(global_improvement);
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (is_success || rand()%3 != 0) {
			#else
			if (is_success) {
			#endif /* MDEBUG */
				this->new_obs_histories.clear();
				this->new_true_histories.clear();

				EvalExperiment* new_eval_experiment = new EvalExperiment();

				new_eval_experiment->scope_context = this->scope_context;
				new_eval_experiment->node_context = this->node_context;
				new_eval_experiment->is_branch = this->is_branch;
				new_eval_experiment->exit_next_node = this->exit_next_node;

				new_eval_experiment->best_new_scope = this->best_new_scope;
				this->best_new_scope = NULL;
				new_eval_experiment->best_step_types = this->best_step_types;
				new_eval_experiment->best_actions = this->best_actions;
				new_eval_experiment->best_scopes = this->best_scopes;

				new_eval_experiment->new_network = new_network;

				wrapper->curr_explore_experiment = NULL;
				this->node_context->experiment = new_eval_experiment;
				delete this;
			} else {
				delete new_network;

				wrapper->curr_explore_experiment = NULL;
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
