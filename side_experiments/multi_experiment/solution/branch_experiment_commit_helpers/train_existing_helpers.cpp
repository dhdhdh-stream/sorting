#include "branch_experiment.h"

#include "solution_helpers.h"

using namespace std;

void BranchExperiment::train_existing_commit_activate(
		ScopeHistory* scope_history,
		ScopeHistory* temp_history) {
	vector<double> input_vals(this->existing_inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		{
			double obs;
			fetch_input_helper(scope_history,
							   this->existing_inputs[i_index],
							   0,
							   obs);
			if (obs != 0.0) {
				input_vals[i_index] = obs;
			}
		}
		{
			double obs;
			fetch_input_helper(temp_history,
							   this->existing_inputs[i_index],
							   0,
							   obs);
			if (obs != 0.0) {
				input_vals[i_index] = obs;
			}
		}
	}
	this->existing_input_histories.push_back(input_vals);

	vector<double> factor_vals(this->existing_factor_ids.size(), 0.0);
	for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
		{
			double val;
			fetch_factor_helper(scope_history,
								this->existing_factor_ids[f_index],
								val);
			if (val != 0.0) {
				factor_vals[f_index] = val;
			}
		}
		{
			double val;
			fetch_factor_helper(temp_history,
								this->existing_factor_ids[f_index],
								val);
			if (val != 0.0) {
				factor_vals[f_index] = val;
			}
		}
	}
	this->existing_factor_histories.push_back(factor_vals);
}
