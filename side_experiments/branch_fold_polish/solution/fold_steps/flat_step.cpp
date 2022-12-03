#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::flat_step_explore_on_path_activate(double existing_score,
											  vector<vector<double>>& flat_vals,
											  vector<double>& local_s_input_vals,
											  vector<double>& local_state_vals,
											  double& predicted_score,
											  double& scale_factor,
											  FoldHistory* history) {
	// train starting_score_network as part of flat, but don't worry about starting_compress_network yet
	this->starting_score_network->activate_small(local_s_input_vals,
												 local_state_vals);
	history->starting_score_update = this->starting_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->starting_score_network->output->acti_vals[0];

	history->existing_score = existing_score;
	this->combined_score_network->activate_small(local_s_input_vals,
												 local_state_vals);
	history->combined_score_update = this->combined_score_network->output->acti_vals[0];

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
			this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  local_state_vals);
			vector<double> scope_input(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod = this->scope_scale_mod_calcs[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[f_index]);
			this->existing_actions[f_index]->existing_flat_activate(flat_vals,
																	scope_input,
																	scope_output,
																	predicted_score,
																	scale_factor,
																	scope_history);
			history->scope_histories[f_index] = scope_history;

			scale_factor /= scope_scale_mod;

			fold_input.push_back(scope_output);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->existing_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(scope_output);
				}
			}
		}
	}

	this->curr_fold->activate(fold_input,
							  local_s_input_vals,
							  local_state_vals);
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	this->curr_end_fold->activate(fold_input,
								  local_s_input_vals,
								  local_state_vals);
	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	double end_scale_mod_val = this->end_scale_mod_calc->output->constants[0];
	scale_factor *= end_scale_mod_val;
}

void Fold::flat_step_explore_on_path_backprop(vector<double>& local_state_errors,
											  double& predicted_score,
											  double target_val,
											  double& scale_factor,
											  double& scale_factor_error,
											  FoldHistory* history) {
	double end_scale_mod_val = this->end_scale_mod_calc->output->constants[0];
	scale_factor /= end_scale_mod_val;
	scale_factor_error *= end_scale_mod_val;

	if (this->state_iter >= 490000) {
		double misguess = target_val - predicted_score;
		this->average_misguess += misguess;
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	double predicted_score_error = target_val - predicted_score;
	if (this->state_iter <= 300000) {
		this->end_scale_mod_calc->backprop(scale_factor_error, 0.005);

		this->curr_end_fold->backprop(local_state_errors, 0.05);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop(curr_fold_error, 0.05);
	} else if (this->state_iter <= 400000) {
		this->end_scale_mod_calc->backprop(scale_factor_error, 0.001);

		this->curr_end_fold->backprop(local_state_errors, 0.01);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop(curr_fold_error, 0.01);
	} else {
		this->end_scale_mod_calc->backprop(scale_factor_error, 0.0002);

		this->curr_end_fold->backprop(local_state_errors, 0.002);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop(curr_fold_error, 0.002);
	}
	// don't need to worry about s_input_errors and state_errors
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
	predicted_score -= scale_factor*this->ending_score_update;

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		if (this->existing_actions[f_index] != NULL) {
			double scope_scale_mod = this->scope_scale_mod_calcs[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod;

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_flat_backprop(scope_input_errors[f_index],
																	scope_output_errors,
																	predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scope_scale_factor_error,
																	history->scope_histories[f_index]);

			scope_scale_factor_error *= scope_scale_mod;

			scale_factor /= scope_scale_mod;

			if (this->state_iter <= 300000) {
				this->scope_scale_mod_calcs[f_index]->backprop(scope_scale_factor_error, 0.005);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.05);
			} else if (this->state_iter <= 400000) {
				this->scope_scale_mod_calcs[f_index]->backprop(scope_scale_factor_error, 0.001);
				
				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.01);
			} else {
				this->scope_scale_mod_calcs[f_index]->backprop(scope_scale_factor_error, 0.0002);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.002);
			}
			for (int ff_index = f_index-1; ff_index >= 0; ff_index--) {
				if (this->existing_actions[ff_index] != NULL) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	if (this->state_iter >= 490000) {
		this->replace_improvement += scale_factor*history->starting_score_update - history->existing_score;
	}

	double starting_predicted_score_error = target_val - predicted_score;
	vector<double> starting_score_errors{scale_factor*starting_predicted_score_error};
	if (this->state_iter <= 300000) {
		this->starting_score_network->backprop_weights_with_no_error_signal(
			starting_score_errors,
			0.05);
	} else if (this->state_iter <= 400000) {
		this->starting_score_network->backprop_weights_with_no_error_signal(
			starting_score_errors,
			0.01);
	} else {
		this->starting_score_network->backprop_weights_with_no_error_signal(
			starting_score_errors,
			0.002);
	}
	// end of backprop so no need to modify predicted_score

	if (this->state_iter >= 490000) {
		this->combined_improvement += scale_factor*history->combined_score_update - history->existing_score;
	}

	double higher_branch_val;
	if (history->existing_score > this->starting_score_network->output->acti_vals[0]) {
		higher_branch_val = history->existing_score;
	} else {
		higher_branch_val = scale_factor*history->starting_score_update;
	}
	double combined_score_error = higher_branch_val - scale_factor*history->combined_score_update;
	vector<double> combined_score_errors{scale_factor*combined_score_error};
	if (this->state_iter <= 300000) {
		this->combined_score_network->backprop_weights_with_no_error_signal(
			combined_score_errors,
			0.05);
	} else if (this->state_iter <= 400000) {
		this->combined_score_network->backprop_weights_with_no_error_signal(
			combined_score_errors,
			0.01);
	} else {
		this->combined_score_network->backprop_weights_with_no_error_signal(
			combined_score_errors,
			0.002);
	}
}
