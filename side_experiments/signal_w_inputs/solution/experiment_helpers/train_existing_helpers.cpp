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
const int TRAIN_EXISTING_NUM_DATAPOINTS = 1000;
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

		if (this->signal_depth == -1) {
			while (this->existing_target_vals.size() < this->existing_obs_histories.size()) {
				this->existing_target_vals.push_back(target_val);
			}
		} else {
			for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
				ScopeHistory* scope_history;
				vector<int> trimmed_explore_index;
				if (this->signal_depth >= (int)history->stack_traces[i_index].size()) {
					scope_history = history->stack_traces[i_index][0];
					trimmed_explore_index = history->explore_indexes[i_index];
				} else {
					int index = history->stack_traces[i_index].size()-1 - this->signal_depth;
					scope_history = history->stack_traces[i_index][index];
					trimmed_explore_index = vector<int>(
						history->explore_indexes[i_index].end() - (this->signal_depth + 1),
						history->explore_indexes[i_index].end());
				}

				Scope* scope = scope_history->scope;

				double new_signal = scope->signal->activate(scope_history,
															trimmed_explore_index);

				this->existing_target_vals.push_back(new_signal);
			}
		}

		if (this->hit_count <= EXPERIMENT_EXPLORE_ITERS) {
			uniform_int_distribution<int> distribution(0, history->stack_traces.size()-1);
			int index = distribution(generator);

			double sum_vals = target_val;
			int count = 1;
			vector<int> trimmed_explore_index = history->explore_indexes[index];
			for (int l_index = 0; l_index < (int)history->stack_traces[index].size()-1; l_index++) {
				ScopeHistory* scope_history = history->stack_traces[index][l_index];
				if (scope_history->scope->signal->nodes.size() > 0) {
					double val = scope_history->scope->signal->activate(scope_history,
																		trimmed_explore_index);
					sum_vals += val;
					count++;
				}

				trimmed_explore_index.erase(trimmed_explore_index.begin());
			}

			signal_add_sample(history->stack_traces[index].back(),
							  trimmed_explore_index,
							  sum_vals / count,
							  true,
							  wrapper);
		}
	}

	if (this->hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;

		if (this->signal_depth != -1) {
			double sum_signals = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
				sum_signals += this->existing_target_vals[h_index];
			}
			this->existing_signal = sum_signals / (double)this->existing_target_vals.size();
		}

		uniform_int_distribution<int> val_input_distribution(0, this->existing_obs_histories.size()-1);

		this->existing_network = new Network(this->existing_obs_histories[0].size());
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_network->activate(this->existing_obs_histories[rand_index]);

			double error = this->existing_target_vals[rand_index] - this->existing_network->output->acti_vals[0];

			this->existing_network->backprop(error);
		}

		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
