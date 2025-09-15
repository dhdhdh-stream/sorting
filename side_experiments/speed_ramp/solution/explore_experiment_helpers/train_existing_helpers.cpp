#include "explore_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
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

	history->signal_is_set.push_back(false);
	history->signal_vals.push_back(0.0);

	for (int l_index = (int)wrapper->scope_histories.size()-1; l_index >= 0; l_index--) {
		if (wrapper->scope_histories[l_index]->scope->signals.size() > 0) {
			wrapper->scope_histories[l_index]->explore_experiment_callbacks.push_back(history);
			break;
		}
	}
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history) {
	for (int i_index = 0; i_index < (int)history->signal_is_set.size(); i_index++) {
		if (history->signal_is_set[i_index]) {
			this->target_val_histories.push_back(history->signal_vals[i_index]);
		} else {
			this->target_val_histories.push_back(target_val);
		}
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_ITERS) {
		double average_score;
		vector<Input> factor_inputs;
		vector<double> factor_input_averages;
		vector<double> factor_input_standard_deviations;
		vector<double> factor_weights;
		bool is_success = train_existing(scope_histories,
										 target_val_histories,
										 average_score,
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
			this->existing_average_score = average_score;
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
