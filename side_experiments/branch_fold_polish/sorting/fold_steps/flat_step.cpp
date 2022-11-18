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
	// train starting_score_network as part of flat, but don't worry about starting_compress_network yet
	this->starting_score_network->activate(starting_state_vals,
										   starting_s_input_vals);

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

			// don't worry about scope_average_mods at flat

			vector<double> empty_input;
			this->scope_scale_mod_calcs[f_index]->activate(empty_input);
			double scope_scale_mod_val = this->scope_scale_mod_calcs[f_index]->output->acti_vals[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			this->compound_actions[f_index]->activate(flat_vals,
													  scope_input,
													  scope_output,
													  predicted_score,
													  scale_factor);

			scale_factor /= scope_scale_mod_val;

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

	// don't worry about end_average_mod at flat
	vector<double> empty_input;
	this->end_scale_mod_calc->activate(empty_input);
	double end_scale_mod_val = this->end_scale_mod_calc->output->acti_vals[0];
	scale_factor *= end_scale_mod_val;
	new_scale_factor = end_scale_mod_val;
}

void Fold::flat_step_backprop(vector<double> input_errors,
							  double& predicted_score,
							  double target_val,
							  double& scale_factor,
							  double& new_scale_factor_error) {
	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	double predicted_score_error = target_val - predicted_score;
	if (this->stage_iter <= 300000) {
		this->ending_scale_mod_calc->backprop(new_scale_factor_error, 0.005);

		this->curr_end_fold->backprop(input_errors, 0.05);

		vector<double> curr_fold_error{predicted_score_error};
		this->curr_fold->backprop(curr_fold_error, 0.05);
	} else if (this->stage_iter <= 400000) {
		this->ending_scale_mod_calc->backprop(new_scale_factor_error, 0.001);

		this->curr_end_fold->backprop(input_errors, 0.01);

		vector<double> curr_fold_error{predicted_score_error};
		this->curr_fold->backprop(curr_fold_error, 0.01);
	} else {
		this->ending_scale_mod_calc->backprop(new_scale_factor_error, 0.0002);

		this->curr_end_fold->backprop(input_errors, 0.002);

		vector<double> curr_fold_error{predicted_score_error};
		this->curr_fold->backprop(curr_fold_error, 0.002);
	}
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_outputs; i_index++) {
				scope_input_errors[f_index][i_index] += this->curr_end_fold->flat_inputs[f_index]->errors[i_index];
				this->curr_end_fold->flat_inputs[f_index]->errors[i_index] = 0.0;

				scope_input_errors[f_index][i_inidex] += this->curr_fold->flat_inputs[f_index]->errors[i_index];
				this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
			}
		}
	}
	predicted_score -= this->curr_fold->output->acti_vals[0];

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		if (this->existing_actions[f_index] != NULL) {
			double scope_scale_mod_val = this->scope_scale_mod_calcs[f_index]->output->acti_vals[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;
			double new_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_backprop(scope_input_errors[f_index],
															   scope_output_errors,
															   predicted_score,
															   predicted_score_error,
															   scale_factor,
															   scope_scale_mod_val,
															   new_scale_factor_error);

			scale_factor /= scope_scale_mod_val;

			if (this->stage_iter <= 300000) {
				this->scope_scale_mod_calcs[f_index]->backprop(new_scale_factor_error, 0.005);

				this->curr_scope_input_folds[f_index]->backprop(scope_output_errors, 0.05);
			} else if (this->stage_iter <= 400000) {
				this->scope_scale_mod_calcs[f_index]->backprop(new_scale_factor_error, 0.001);
				
				this->curr_scope_input_folds[f_index]->backprop(scope_output_errors, 0.01);
			} else {
				this->scope_scale_mod_calcs[f_index]->backprop(new_scale_factor_error, 0.0002);

				this->curr_scope_input_folds[f_index]->backprop(scope_output_errors, 0.002);
			}
			for (int ff_index = f_index-1; ff_index >= 0; ff_index--) {
				if (this->existing_actions[ff_index] != NULL) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	vector<double> starting_score_errors{target_val - predicted_score};
	if (this->stage_iter <= 300000) {
		this->starting_score_network->backprop_weights_with_no_error_signal(
			starting_score_errors,
			0.05);
	} else if (this->stage_iter <= 400000) {
		this->starting_score_network->backprop_weights_with_no_error_signal(
			starting_score_errors,
			0.01);
	} else {
		this->starting_score_network->backprop_weights_with_no_error_signal(
			starting_score_errors,
			0.002);
	}
}
