#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "signal.h"
#include "signal_helpers.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 800;
#endif /* MDEBUG */

void Experiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::train_existing_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	history->stack_traces.push_back(wrapper->scope_histories);
	vector<int> curr_explore_index;
	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		curr_explore_index.push_back(wrapper->scope_histories[l_index]->node_histories.size()-1);
	}
	history->explore_indexes.push_back(curr_explore_index);

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
			vector<double> curr_target_vals;
			vector<bool> curr_target_vals_is_on;

			curr_target_vals.push_back(target_val);
			curr_target_vals_is_on.push_back(true);

			vector<int> trimmed_explore_index = history->explore_indexes[i_index];
			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[i_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->post_signal->nodes.size() > 0) {
					double new_signal = scope->post_signal->activate(
						scope_history,
						trimmed_explore_index);

					curr_target_vals.push_back(new_signal);
					curr_target_vals_is_on.push_back(true);
				} else {
					curr_target_vals.push_back(0.0);
					curr_target_vals_is_on.push_back(false);
				}

				trimmed_explore_index.erase(trimmed_explore_index.begin());
			}

			this->existing_target_vals.push_back(curr_target_vals);
			this->existing_target_vals_is_on.push_back(curr_target_vals_is_on);
		}

		if (this->hit_count <= EXPERIMENT_EXPLORE_ITERS) {
			uniform_int_distribution<int> distribution(0, history->stack_traces.size()-1);
			int index = distribution(generator);

			{
				vector<int> trimmed_explore_index = history->explore_indexes[index];
				for (int l_index = 0; l_index < (int)history->stack_traces[index].size(); l_index++) {
					pre_signal_add_sample(history->stack_traces[index][l_index],
										  trimmed_explore_index,
										  target_val,
										  wrapper);

					trimmed_explore_index.erase(trimmed_explore_index.begin());
				}
			}

			{
				double sum_vals = target_val;
				int count = 1;
				vector<int> trimmed_explore_index = history->explore_indexes[index];
				for (int l_index = 0; l_index < (int)history->stack_traces[index].size()-1; l_index++) {
					ScopeHistory* scope_history = history->stack_traces[index][l_index];
					if (scope_history->scope->post_signal->nodes.size() > 0) {
						double val = scope_history->scope->post_signal->activate(
							scope_history,
							trimmed_explore_index);
						sum_vals += val;
						count++;
					}

					trimmed_explore_index.erase(trimmed_explore_index.begin());
				}

				post_signal_add_sample(history->stack_traces[index].back(),
									   trimmed_explore_index,
									   sum_vals / count,
									   true,
									   wrapper);
			}
		}
	}

	if (this->hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;

		// int max_layer = 0;
		// for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		// 	if ((int)this->existing_target_vals[h_index].size() > max_layer) {
		// 		max_layer = (int)this->existing_target_vals[h_index].size();
		// 	}
		// }
		// this->existing_networks = vector<Network*>(max_layer, NULL);
		// this->existing_misguess_standard_deviations = vector<double>(max_layer);
		// for (int l_index = 0; l_index < max_layer; l_index++) {
		// 	vector<vector<double>> existing_on_obs_histories;
		// 	vector<double> existing_on_target_vals;
		// 	for (int h_index = 0; h_index < (int)this->existing_obs_histories.size(); h_index++) {
		// 		if (l_index < (int)this->existing_target_vals[h_index].size()) {
		// 			if (this->existing_target_vals_is_on[h_index][l_index]) {
		// 				existing_on_obs_histories.push_back(this->existing_obs_histories[h_index]);
		// 				existing_on_target_vals.push_back(this->existing_target_vals[h_index][l_index]);
		// 			}
		// 		}
		// 	}

		// 	if (existing_on_obs_histories.size() < MIN_TRAIN_NUM_SAMPLES) {
		// 		continue;
		// 	}

		// 	Network* network = new Network(existing_on_obs_histories[0].size());
		// 	uniform_int_distribution<int> existing_distribution(0, existing_on_obs_histories.size()-1);
		// 	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		// 		int index = existing_distribution(generator);

		// 		network->activate(existing_on_obs_histories[index]);

		// 		double error = existing_on_target_vals[index] - network->output->acti_vals[0];

		// 		network->backprop(error);
		// 	}

		// 	double sum_misguess = 0.0;
		// 	for (int h_index = 0; h_index < (int)existing_on_obs_histories.size(); h_index++) {
		// 		network->activate(existing_on_obs_histories[h_index]);
		// 		sum_misguess += (existing_on_target_vals[h_index] - network->output->acti_vals[0])
		// 			* (existing_on_target_vals[h_index] - network->output->acti_vals[0]);
		// 	}
		// 	double misguess_standard_deviation = sqrt(sum_misguess / (double)existing_on_obs_histories.size());

		// 	this->existing_networks[l_index] = network;
		// 	this->existing_misguess_standard_deviations[l_index] = misguess_standard_deviation;
		// }

		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
