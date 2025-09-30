#include "explore_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "signal_experiment.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_ITERS = 10;
#else
const int TRAIN_EXISTING_ITERS = 1000;
#endif /* MDEBUG */

void ExploreExperiment::train_existing_check_activate(
		SolutionWrapper* wrapper,
		ExploreExperimentHistory* history) {
	/**
	 * - add even if explore
	 */
	ScopeHistory* scope_history_copy = new ScopeHistory(wrapper->scope_histories.back());
	scope_history_copy->num_actions_snapshot = wrapper->num_actions;
	this->scope_histories.push_back(scope_history_copy);

	history->sum_signal_vals.push_back(0.0);
	history->sum_counts.push_back(0);

	for (int i_index = 0; i_index < (int)wrapper->scope_histories.size(); i_index++) {
		Scope* scope = wrapper->scope_histories[i_index]->scope;
		if (scope->default_signal != NULL) {
			if (scope->signal_experiment_history == NULL
					|| !scope->signal_experiment_history->is_on) {
				wrapper->scope_histories[i_index]->explore_experiment_callbacks
					.push_back(history);
				wrapper->scope_histories[i_index]->explore_experiment_instance_indexes
					.push_back((int)history->sum_signal_vals.size()-1);
			}
		}
	}
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)history->sum_signal_vals.size(); i_index++) {
		history->sum_signal_vals[i_index] += target_val;
		history->sum_counts[i_index]++;

		double average_val = history->sum_signal_vals[i_index]
			/ (double)history->sum_counts[i_index];

		this->target_val_histories.push_back(average_val);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_ITERS) {
		double constant;
		vector<Input> factor_inputs;
		vector<double> factor_input_averages;
		vector<double> factor_input_standard_deviations;
		vector<double> factor_weights;
		bool is_success = train_existing(scope_histories,
										 target_val_histories,
										 constant,
										 factor_inputs,
										 factor_input_averages,
										 factor_input_standard_deviations,
										 factor_weights);

		for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
			delete this->scope_histories[h_index];
		}
		this->scope_histories.clear();
		this->target_val_histories.clear();

		if (is_success) {
			this->existing_constant = constant;
			this->existing_inputs = factor_inputs;
			this->existing_input_averages = factor_input_averages;
			this->existing_input_standard_deviations = factor_input_standard_deviations;
			this->existing_weights = factor_weights;

			this->best_surprise = 0.0;

			int average_instances_per_run = (this->sum_num_instances + (int)this->last_num_instances.size() - 1)
				/ (int)this->last_num_instances.size();
			uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		} else {
			this->node_context->experiment = NULL;
			delete this;
		}
	}
}
