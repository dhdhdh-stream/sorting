#include "experiment.h"

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

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */
const double TRAIN_NEW_VALIDATION_RATIO = 0.2;

const int NEW_BOOST_NUM_TRIES = 6;

void Experiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
		history->stack_traces.push_back(wrapper->scope_histories);

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

		this->new_obs_histories.push_back(obs);

		this->existing_true_network->activate(obs);
		history->existing_predicted_trues.push_back(
			this->existing_true_network->output->acti_vals[0]);
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

	vector<double> obs = wrapper->problem->get_observations();
	wrapper->scope_histories.back()->obs_history = obs;

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
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			vector<ScopeHistory*> stack_trace_copy(history->stack_traces[i_index].size());
			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				stack_trace_copy[l_index] = history->stack_traces[i_index][l_index]->copy_obs_history();
			}
			this->new_stack_traces.push_back(stack_trace_copy);

			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->new_true_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_stack_traces.begin(), this->new_stack_traces.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_true_histories.begin(), this->new_true_histories.end(), generator_copy);
			}

			int num_train = (1.0 - TRAIN_NEW_VALIDATION_RATIO) * TRAIN_NEW_NUM_DATAPOINTS;

			vector<vector<double>> train_obs_histories(this->new_obs_histories.begin(), this->new_obs_histories.begin() + num_train);
			vector<vector<ScopeHistory*>> train_stack_traces(this->new_stack_traces.begin(), this->new_stack_traces.begin() + num_train);
			vector<double> train_true_histories(this->new_true_histories.begin(), this->new_true_histories.begin() + num_train);

			vector<vector<double>> validation_obs_histories(this->new_obs_histories.begin() + num_train, this->new_obs_histories.end());
			vector<vector<ScopeHistory*>> validation_stack_traces(this->new_stack_traces.begin() + num_train, this->new_stack_traces.end());
			vector<double> validation_true_histories(this->new_true_histories.begin() + num_train, this->new_true_histories.end());

			this->new_true_network = new Network(train_obs_histories[0].size(),
												 NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> val_input_distribution(0, train_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->new_true_network->activate(train_obs_histories[rand_index]);

				double error = train_true_histories[rand_index] - this->new_true_network->output->acti_vals[0];

				this->new_true_network->backprop(error);
			}

			double best_sum_misguess = 0.0;
			int best_num_positive = 0;
			double best_sum_predicted_score = 0.0;
			for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
				this->new_true_network->activate(validation_obs_histories[h_index]);
				double predicted_score = this->new_true_network->output->acti_vals[0];
				best_sum_misguess += (validation_true_histories[h_index] - predicted_score) * (validation_true_histories[h_index] - predicted_score);

				if (predicted_score > 0.0) {
					best_num_positive++;
				}

				if (predicted_score >= 0.0) {
					best_sum_predicted_score += validation_true_histories[h_index];
				}
			}

			// temp
			Network* starting_true_network = this->new_true_network;

			for (int t_index = 0; t_index < NEW_BOOST_NUM_TRIES; t_index++) {
				boost_try(train_obs_histories,
						  train_stack_traces,
						  train_true_histories,
						  validation_obs_histories,
						  validation_stack_traces,
						  validation_true_histories,
						  this->new_true_network,
						  best_sum_misguess,
						  best_num_positive,
						  best_sum_predicted_score);
			}

			// temp
			Network* ending_true_network = this->new_true_network;

			if (starting_true_network == ending_true_network) {
				this->new_is_boost = false;
			} else {
				this->new_is_boost = true;
			}

			#if defined(MDEBUG) && MDEBUG
			if ((best_num_positive > 0 && best_sum_predicted_score >= 0.0) || rand()%4 != 0) {
			#else
			if (best_num_positive > 0 && best_sum_predicted_score >= 0.0) {
			#endif /* MDEBUG */
				this->sum_true = 0.0;
				this->hit_count = 0;

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->state = EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
