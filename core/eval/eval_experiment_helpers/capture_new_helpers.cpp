#include "eval_experiment.h"

using namespace std;

void EvalExperiment::capture_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context) {
	this->new_decision_scope_histories.push_back(new ScopeHistory(context.back().scope_history));

	context.back().scope_history->experiment_history = history;

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		problem->perform_action(this->actions[a_index]->action);
	}

	curr_node = this->curr_exit_next_node;
}

void EvalExperiment::capture_new_back_activate(
		vector<ContextLayer>& context) {
	this->new_final_scope_histories.push_back(new ScopeHistory(context.back().scope_history));
}

void EvalExperiment::capture_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	while (this->new_target_val_histories.size() < this->new_decision_scope_histories.size()) {
		this->new_target_val_histories.push_back(target_val);
	}

	if (this->new_decision_scope_histories.size() > NUM_DATAPOINTS) {
		vector<double> existing_target_vals;
		vector<double> new_target_vals;
		train_eval_helper(existing_target_vals,
						  new_target_vals);

		train_existing_helper(existing_target_vals);

		train_new_helper(new_target_vals);

		this->combined_misguess = 0.0;
		this->original_count = 0;
		this->branch_count = 0;

		this->state = EVAL_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
