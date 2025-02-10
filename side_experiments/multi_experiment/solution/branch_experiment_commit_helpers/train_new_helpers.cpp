#include "branch_experiment.h"

#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::train_new_commit_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		ScopeHistory* temp_history,
		BranchExperimentHistory* history) {
	double sum_vals = this->existing_average_score;
	for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
		{
			double val;
			fetch_factor_helper(scope_history,
								this->existing_factor_ids[f_index],
								val);
			if (val != 0.0) {
				sum_vals += this->existing_factor_weights[f_index] * val;
			}
		}
		{
			double val;
			fetch_factor_helper(temp_history,
								this->existing_factor_ids[f_index],
								val);
			if (val != 0.0) {
				sum_vals += this->existing_factor_weights[f_index] * val;
			}
		}
	}
	history->existing_predicted_score = sum_vals;

	vector<double> input_vals(this->new_inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
		{
			double obs;
			fetch_input_helper(scope_history,
							   this->new_inputs[i_index],
							   0,
							   obs);
			if (obs != 0.0) {
				input_vals[i_index] = obs;
			}
		}
		{
			double obs;
			fetch_input_helper(temp_history,
							   this->new_inputs[i_index],
							   0,
							   obs);
			if (obs != 0.0) {
				input_vals[i_index] = obs;
			}
		}
	}
	this->new_input_histories.push_back(input_vals);

	vector<double> factor_vals(this->new_factor_ids.size(), 0.0);
	for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
		{
			double val;
			fetch_factor_helper(scope_history,
								this->new_factor_ids[f_index],
								val);
			if (val != 0.0) {
				factor_vals[f_index] = val;
			}
		}
		{
			double val;
			fetch_factor_helper(temp_history,
								this->new_factor_ids[f_index],
								val);
			if (val != 0.0) {
				factor_vals[f_index] = val;
			}
		}
	}
	this->new_factor_histories.push_back(factor_vals);

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			double score = problem->perform_action(this->best_actions[s_index]);
			run_helper.sum_score += score;
			run_helper.num_actions++;
			double individual_impact = score / run_helper.num_actions;
			for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size(); h_index++) {
				run_helper.experiment_histories[h_index]->impact += individual_impact;
			}
			if (score < 0.0) {
				run_helper.early_exit = true;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
			this->best_scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		if (run_helper.early_exit) {
			break;
		}
	}

	curr_node = this->best_exit_next_node;
}
