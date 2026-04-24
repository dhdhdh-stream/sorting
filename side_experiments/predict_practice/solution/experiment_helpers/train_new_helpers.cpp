#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 5000;
#endif /* MDEBUG */

const double VERIFY_RATIO = 0.2;

void Experiment::train_new_check_activate(
		vector<double>& obs,
		bool& is_next,
		bool& is_done,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
	/**
	 * - still only activate some of the time to prevent correlation with other experiments
	 */
	uniform_int_distribution<int> on_distribution(0, 9);
	if (on_distribution(generator) == 0) {
		this->new_obs_histories.push_back(obs);

		double sum_existing = 0.0;
		for (int r_index = 0; r_index < EXISTING_NUM_SIMULATE; r_index++) {
			sum_existing += simulate_helper(wrapper);
		}
		double existing_average = sum_existing / EXISTING_NUM_SIMULATE;

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;

		double sum_new = 0.0;
		for (int r_index = 0; r_index < NEW_NUM_SIMULATE; r_index++) {
			sum_new += simulate_helper(wrapper);
		}
		double new_average = sum_new / NEW_NUM_SIMULATE;

		delete new_experiment_state;
		wrapper->experiment_context.back() = NULL;

		this->new_target_val_histories.push_back(new_average - existing_average);
	}
}

void Experiment::train_new_backprop(double target_val,
									ExperimentHistory* history,
									SolutionWrapper* wrapper) {
	if ((int)this->new_obs_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_target_val_histories.begin(), this->new_target_val_histories.end(), generator_copy);
		}

		Network* new_network = new Network(this->new_obs_histories[0].size());

		int num_train = (1.0 - VERIFY_RATIO) * (double)this->new_obs_histories.size();

		uniform_int_distribution<int> distribution(0, num_train-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = distribution(generator);

			new_network->activate(this->new_obs_histories[rand_index]);

			double error = this->new_target_val_histories[rand_index] - new_network->output->acti_vals[0];

			new_network->backprop(error);
		}

		double sum_vals = 0.0;
		for (int h_index = num_train; h_index < (int)this->new_obs_histories.size(); h_index++) {
			new_network->activate(this->new_obs_histories[h_index]);

			if (new_network->output->acti_vals[0] >= 0.0) {
				sum_vals += this->new_target_val_histories[h_index];
			}
		}
		double local_improvement = sum_vals / ((double)this->new_obs_histories.size() - (double)num_train);

		int total_iters = wrapper->iter - this->starting_iter;
		if (total_iters < 0) {
			total_iters += numeric_limits<int>::max();
		}
		double average_hits_per_run = (10.0 * (double)this->new_obs_histories.size()) / (double)total_iters;
		double global_improvement = average_hits_per_run * local_improvement;

		// // temp
		// cout << "local_improvement: " << local_improvement << endl;
		// cout << "average_hits_per_run: " << average_hits_per_run << endl;
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
			this->new_target_val_histories.clear();

			this->new_networks.push_back(new_network);

			this->curr_ramp = 0;
			this->measure_status = MEASURE_STATUS_N_A;

			this->state = EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
		} else {
			delete new_network;

			this->node_context->experiment = NULL;
			delete this;
		}
	}
}

void Experiment::result_train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper,
									   ExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->result_node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->result_experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->result_num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->result_scope_histories.push_back(inner_scope_history);
			wrapper->result_node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->result_experiment_context.push_back(NULL);
		}
	}
}

void Experiment::result_train_new_exit_step(SolutionWrapper* wrapper,
											ExperimentState* experiment_state) {
	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	experiment_state->step_index++;
}
