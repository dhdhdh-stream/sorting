#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */
const double TRAIN_EXISTING_VALIDATION_RATIO = 0.2;

const int EXISTING_BOOST_NUM_TRIES = 6;

void Experiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	history->stack_traces.push_back(wrapper->scope_histories);

	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::train_existing_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	this->existing_obs_histories.push_back(obs);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void Experiment::train_existing_backprop(double target_val,
										 SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->sum_true += target_val;
		this->hit_count++;

		for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
			vector<ScopeHistory*> stack_trace_copy(history->stack_traces[i_index].size());
			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				stack_trace_copy[l_index] = history->stack_traces[i_index][l_index]->copy_obs_history();
			}
			this->existing_stack_traces.push_back(stack_trace_copy);

			this->existing_true_histories.push_back(target_val);
		}
	}

	if (this->hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;

		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_obs_histories.begin(), this->existing_obs_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_stack_traces.begin(), this->existing_stack_traces.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_true_histories.begin(), this->existing_true_histories.end(), generator_copy);
		}

		int num_train = (1.0 - TRAIN_EXISTING_VALIDATION_RATIO) * TRAIN_EXISTING_NUM_DATAPOINTS;

		vector<vector<double>> train_obs_histories(this->existing_obs_histories.begin(), this->existing_obs_histories.begin() + num_train);
		vector<vector<ScopeHistory*>> train_stack_traces(this->existing_stack_traces.begin(), this->existing_stack_traces.begin() + num_train);
		vector<double> train_true_histories(this->existing_true_histories.begin(), this->existing_true_histories.begin() + num_train);

		vector<vector<double>> validation_obs_histories(this->existing_obs_histories.begin() + num_train, this->existing_obs_histories.end());
		vector<vector<ScopeHistory*>> validation_stack_traces(this->existing_stack_traces.begin() + num_train, this->existing_stack_traces.end());
		vector<double> validation_true_histories(this->existing_true_histories.begin() + num_train, this->existing_true_histories.end());

		this->existing_true_network = new Network(this->existing_obs_histories[0].size(),
												  NETWORK_SIZE_SMALL);
		uniform_int_distribution<int> input_distribution(0, train_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = input_distribution(generator);

			this->existing_true_network->activate(train_obs_histories[rand_index]);

			double error = train_true_histories[rand_index] - this->existing_true_network->output->acti_vals[0];

			this->existing_true_network->backprop(error);
		}

		double best_sum_misguess = 0.0;
		for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
			this->existing_true_network->activate(validation_obs_histories[h_index]);
			double predicted_score = this->existing_true_network->output->acti_vals[0];
			best_sum_misguess += (validation_true_histories[h_index] - predicted_score) * (validation_true_histories[h_index] - predicted_score);
		}

		for (int t_index = 0; t_index < EXISTING_BOOST_NUM_TRIES; t_index++) {
			int best_num_positive = 0;	// unused
			boost_try(train_obs_histories,
					  train_stack_traces,
					  train_true_histories,
					  validation_obs_histories,
					  validation_stack_traces,
					  validation_true_histories,
					  this->existing_true_network,
					  best_sum_misguess,
					  best_num_positive);
		}

		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
