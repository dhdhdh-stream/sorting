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
	if (wrapper->is_explore
			&& on_distribution(generator) == 0) {
		this->new_obs_histories.push_back(obs);

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;

		this->existing_network->activate(obs);
		history->existing_predicted.push_back(this->existing_network->output->acti_vals[0]);

		double next_clean_result = clean_result_helper(wrapper);
		double existing_norm_score = wrapper->prev_clean_result - PROTECT_SCORE_NORM;
		double new_norm_score = next_clean_result - PROTECT_SCORE_NORM;
		if (new_norm_score / existing_norm_score >= MIN_PROTECT) {
			wrapper->prev_clean_result = next_clean_result;

			history->predict_fail.push_back(false);
		} else {
			delete new_experiment_state;
			wrapper->experiment_context.back() = NULL;

			history->predict_fail.push_back(true);
		}
	} else {
		if (this->existing_obs_histories.size() < 2 * this->new_obs_histories.size()) {
			this->existing_obs_histories.push_back(obs);
		}
	}
}

void Experiment::train_new_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper,
								ExperimentState* experiment_state) {
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
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::train_new_exit_step(SolutionWrapper* wrapper,
									 ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::train_new_backprop(double target_val,
									ExperimentHistory* history,
									SolutionWrapper* wrapper) {
	while (this->existing_target_val_histories.size() < this->existing_obs_histories.size()) {
		this->existing_target_val_histories.push_back(target_val);
	}

	if (history->existing_predicted.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted.size(); i_index++) {
			if (history->predict_fail[i_index]) {
				this->new_target_val_histories.push_back(0.0);
			} else {
				this->new_target_val_histories.push_back(target_val);
			}
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
			delete this->existing_network;
			this->existing_network = new Network(this->existing_obs_histories[0].size());

			uniform_int_distribution<int> existing_distribution(0, this->existing_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = existing_distribution(generator);

				this->existing_network->activate(this->existing_obs_histories[rand_index]);

				double error = this->existing_target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

				this->existing_network->backprop(error);
			}

			for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
				this->existing_network->activate(this->new_obs_histories[h_index]);
				this->new_target_val_histories[h_index] -= this->existing_network->output->acti_vals[0];
			}

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
				this->existing_obs_histories.clear();
				this->existing_target_val_histories.clear();
				this->new_obs_histories.clear();
				this->new_target_val_histories.clear();

				this->new_networks.push_back(new_network);

				this->existing_sum_scores = 0.0;
				this->existing_count = 0;
				this->new_sum_scores = 0.0;
				this->new_count = 0;

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
