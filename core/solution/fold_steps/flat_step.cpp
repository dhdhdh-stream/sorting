#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::flat_step_explore_on_path_activate(double existing_score,
											  Problem& problem,
											  vector<double>& local_s_input_vals,
											  vector<double>& local_state_vals,
											  double& predicted_score,
											  double& scale_factor,
											  RunStatus& run_status,
											  FoldHistory* history) {
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
		if (!this->is_existing[f_index]) {
			problem.perform_action(this->actions[f_index]);

			vector<double> new_obs{problem.get_observation()};

			fold_input.push_back(new_obs);
			for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
				if (this->is_existing[ff_index]) {
					input_fold_inputs[ff_index].push_back(new_obs);
				}
			}
		} else {
			this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  local_state_vals);
			vector<double> scope_input(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[f_index]);
			this->existing_actions[f_index]->existing_flat_activate(problem,
																	scope_input,
																	scope_output,
																	predicted_score,
																	scale_factor,
																	run_status,
																	scope_history);
			history->scope_histories[f_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			if (run_status.exceeded_depth) {
				history->exit_index = f_index;
				history->exit_location = EXIT_LOCATION_SPOT;
				return;
			}

			fold_input.push_back(scope_output);
			for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
				if (this->is_existing[ff_index]) {
					input_fold_inputs[ff_index].push_back(scope_output);
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
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::flat_step_explore_on_path_backprop(vector<double>& local_state_errors,
											  double& predicted_score,
											  double target_val,
											  double& scale_factor,
											  FoldHistory* history) {
	if (this->state_iter >= 450000) {
		this->new_misguess += (target_val - predicted_score)*(target_val - predicted_score);
		// TODO: misguess should be based on final_value?
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	double predicted_score_error = target_val - predicted_score;

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		this->sum_error += abs(predicted_score_error);
		if (this->state_iter <= 300000) {
			this->curr_end_fold->backprop(local_state_errors, 0.05);

			vector<double> curr_fold_error{scale_factor*predicted_score_error};
			this->curr_fold->backprop(curr_fold_error, 0.05);
		} else if (this->state_iter <= 400000) {
			this->curr_end_fold->backprop(local_state_errors, 0.01);

			vector<double> curr_fold_error{scale_factor*predicted_score_error};
			this->curr_fold->backprop(curr_fold_error, 0.01);
		} else {
			this->curr_end_fold->backprop(local_state_errors, 0.002);

			vector<double> curr_fold_error{scale_factor*predicted_score_error};
			this->curr_fold->backprop(curr_fold_error, 0.002);
		}
		// don't need to worry about s_input_errors and state_errors
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				for (int i_index = 0; i_index < this->existing_actions[f_index]->num_outputs; i_index++) {
					scope_input_errors[f_index][i_index] += this->curr_end_fold->flat_inputs[f_index]->errors[i_index];
					this->curr_end_fold->flat_inputs[f_index]->errors[i_index] = 0.0;

					scope_input_errors[f_index][i_index] += this->curr_fold->flat_inputs[f_index]->errors[i_index];
					this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
				}
			}
		}
		predicted_score -= scale_factor*history->ending_score_update;
	}

	for (int f_index = history->exit_index; f_index >= 0; f_index--) {
		if (this->is_existing[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_flat_backprop(scope_input_errors[f_index],
																	scope_output_errors,
																	predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scope_scale_factor_error,
																	history->scope_histories[f_index]);

			scale_factor /= scope_scale_mod_val;

			if (this->state_iter <= 300000) {
				vector<double> mod_errors{scope_scale_factor_error};
				this->scope_scale_mod[f_index]->backprop(mod_errors, 0.005);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.05);
			} else if (this->state_iter <= 400000) {
				vector<double> mod_errors{scope_scale_factor_error};
				this->scope_scale_mod[f_index]->backprop(mod_errors, 0.001);
				
				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.01);
			} else {
				vector<double> mod_errors{scope_scale_factor_error};
				this->scope_scale_mod[f_index]->backprop(mod_errors, 0.0002);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.002);
			}
			for (int ff_index = f_index-1; ff_index >= 0; ff_index--) {
				if (this->is_existing[ff_index]) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	double starting_predicted_score_error = target_val - predicted_score;
	vector<double> starting_score_errors{scale_factor*starting_predicted_score_error};
	if (this->state_iter <= 300000) {
		this->starting_score_network->backprop_small_weights_with_no_error_signal(
			starting_score_errors,
			0.05);
	} else if (this->state_iter <= 400000) {
		this->starting_score_network->backprop_small_weights_with_no_error_signal(
			starting_score_errors,
			0.01);
	} else {
		this->starting_score_network->backprop_small_weights_with_no_error_signal(
			starting_score_errors,
			0.002);
	}
	// end of backprop so no need to modify predicted_score

	if (this->state_iter >= 450000) {
		double score_standard_deviation = sqrt(*this->existing_score_variance);
		double t_value = (history->existing_score - scale_factor*history->starting_score_update) / score_standard_deviation;
		if (t_value > 1.0) {	// >75%
			this->existing_noticably_better++;
		} else if (t_value < -1.0) {	// >75%
			this->new_noticably_better++;
		}

		this->replace_existing += scale_factor*history->starting_score_update - history->existing_score;
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
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.05);
	} else if (this->state_iter <= 400000) {
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.01);
	} else {
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.002);
	}
}
