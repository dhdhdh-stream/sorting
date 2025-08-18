#include "signal_experiment.h"

#include "explore.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

SignalExperiment::SignalExperiment(Scope* scope_context,
								   SolutionWrapper* wrapper) {
	this->scope_context = scope_context;

	this->curr_explore = NULL;

	vector<double> existing_scores;
	vector<double> target_val_histories;
	for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
		if (hit_helper(wrapper->solution->existing_scope_histories[h_index],
					   this->scope_context)) {
			fetch_histories_helper(wrapper->solution->existing_scope_histories[h_index],
								   wrapper->solution->existing_target_val_histories[h_index],
								   this->scope_context,
								   target_val_histories);

			existing_scores.push_back(wrapper->solution->existing_target_val_histories[h_index]);
		}
	}

	double sum_existing_vals = 0.0;
	for (int h_index = 0; h_index < (int)existing_scores.size(); h_index++) {
		sum_existing_vals += existing_scores[h_index];
	}
	this->existing_average = sum_existing_vals / (double)existing_scores.size();

	double sum_existing_variance = 0.0;
	for (int h_index = 0; h_index < (int)existing_scores.size(); h_index++) {
		sum_existing_variance += (existing_scores[h_index] - existing_average)
			* (existing_scores[h_index] - existing_average);
	}
	this->existing_standard_deviation = sqrt(sum_existing_variance / (double)existing_scores.size());

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)target_val_histories.size(); h_index++) {
		sum_vals += target_val_histories[h_index];
	}
	this->existing_average_outer_signal = sum_vals / (double)target_val_histories.size();

	set_actions();

	this->state = SIGNAL_EXPERIMENT_STATE_FIND_SAFE;
}

SignalExperiment::~SignalExperiment() {
	if (this->curr_explore != NULL) {
		delete this->curr_explore;
	}

	for (int e_index = 0; e_index < (int)this->positive_explores.size(); e_index++) {
		delete this->positive_explores[e_index];
	}

	for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
		delete this->signals[s_index];
	}
}

SignalExperimentHistory::SignalExperimentHistory() {
	this->is_hit = false;

	this->signal_needed_from = NULL;
}

SignalExperimentState::SignalExperimentState(SignalExperiment* experiment) {
	this->experiment = experiment;
}
