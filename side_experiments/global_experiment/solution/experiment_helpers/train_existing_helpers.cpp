#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "build_network.h"
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

void Experiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::train_existing_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (this->signal_depth != -1) {
		history->stack_traces.push_back(wrapper->scope_histories);
	}

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
				if (this->signal_depth >= (int)history->stack_traces[i_index].size()) {
					scope_history = history->stack_traces[i_index][0];
				} else {
					int index = history->stack_traces[i_index].size()-1 - this->signal_depth;
					scope_history = history->stack_traces[i_index][index];
				}

				vector<double> input;
				input.insert(input.end(), scope_history->pre_obs_history.begin(), scope_history->pre_obs_history.end());
				input.insert(input.end(), scope_history->post_obs_history.begin(), scope_history->post_obs_history.end());

				Scope* scope = scope_history->scope;
				this->existing_target_vals.push_back(scope->signal->activate(input));
			}
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
