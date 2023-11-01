#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::train_activate() {

	context[context.size() - this->scope_context.size()]
		.scope_history->inner_branch_experiment_history = history;

}

void PassThroughExperiment::train_parent_scope_end_activate(
		vector<ContextLayer>& context,
		ScopeHistory* parent_scope_history) {
	if (this->obs_experiment == NULL) {
		// TODO: create obs_experiment
	} else {
		this->input_state_vals_histories.push_back(context.back().input_state_vals);
		this->local_state_vals_histories.push_back(context.back().local_state_vals);
		this->score_state_vals_histories.push_back(context.back().score_state_vals);
		this->experiment_state_vals_histories.push_back(context.back().experiment_state_vals);
	}
}


