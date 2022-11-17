#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::flat_step_activate(vector<vector<double>>& flat_vals,
							  vector<double>& starting_state_vals,
							  vector<double>& starting_s_input_vals,
							  vector<double>& output_state_vals,
							  double& predicted_score,
							  double& scale_factor,
							  double& new_scale_factor) {
	predicted_score *= this->previous_scale_mod;
	predicted_score += this->previous_average_mod;

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] == NULL) {
			fold_input.push_back(flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->existing_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(flat_vals.begin());
				}
			}

			flat_vals.erase(flat_vals.begin());
		} else {
			this->curr_scope_input_folds[f_index]->activate(input_fold_inputs[f_index],
															starting_state_vals,
															starting_s_input_vals);
			vector<double> scope_input(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_scope_input_folds[f_index]->output->acti_vals[i_index];
			}

			predicted_score += this->scope_average_mod[f_index];

			vector<double> scope_output;
			this->compound_actions[f_index]->existing_activate(flat_vals,
															   scope_input,
															   scope_output,
															   predicted_score,
															   scale_factor,
															   this->scope_scale_mod[f_index]);

			fold_input.push_back(scope_output);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->existing_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(scope_output);
				}
			}
		}
	}

	this->curr_fold->activate(fold_input);
	predicted_score += this->curr_fold->output->acti_vals[0];

	this->curr_end_fold->activate(fold_input);
	output_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		output_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	predicted_score += this->ending_average_mod;
	new_scale_factor = this->ending_scale_mod;
}

void Fold::flat_step_backprop(vector<double> input_errors,
							  double& predicted_score,
							  double target_val,
							  double& scale_factor,
							  double& new_average_factor_error,
							  double& new_scale_factor_error) {
	vector<vector<double>> scope_input_errors(this->sequence_length);
	if (this->stage_iter <= 270000) {
		this->ending_average_mod_calc->backprop(new_average_factor_error, 0.005);
		this->ending_scale_mod_calc->backprop(new_scale_factor_error, 0.005);

		this->curr_end_fold->backprop(input_errors);
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->existing_actions[f_index] != NULL) {

			}
		}

		
	} else {
		this->ending_average_mod_calc->backprop(new_average_factor_error, 0.001);
		this->ending_scale_mod_calc->backprop(new_scale_factor_error, 0.001);

	}


}
