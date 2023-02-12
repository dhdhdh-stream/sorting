#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::flat_step_explore_on_path_activate(double existing_score,
											  Problem& problem,
											  vector<double>& local_s_input_vals,
											  vector<double>& local_state_vals,
											  double& predicted_score,
											  double& scale_factor,
											  RunStatus& run_status,
											  FoldHistory* history) {
	this->existing_average_predicted_score = 0.9999*this->existing_average_predicted_score + 0.0001*existing_score;
	double curr_existing_prediced_score_variance = (this->existing_average_predicted_score - existing_score)*(this->existing_average_predicted_score - existing_score);
	this->existing_predicted_score_variance = 0.9999*this->existing_predicted_score_variance + 0.0001*curr_existing_prediced_score_variance;

	history->existing_score = existing_score;

	this->starting_score_network->activate_small(local_s_input_vals,
												 local_state_vals);
	history->starting_score_update = this->starting_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->starting_score_network->output->acti_vals[0];

	this->combined_score_network->activate_small(local_s_input_vals,
												 local_state_vals);
	history->combined_score_update = this->combined_score_network->output->acti_vals[0];

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (!this->is_inner_scope[f_index]) {
			problem.perform_action(this->actions[f_index]);

			vector<double> new_obs{problem.get_observation()};

			fold_input.push_back(new_obs);
			for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
				if (this->is_inner_scope[ff_index]) {
					input_fold_inputs[ff_index].push_back(new_obs);
				}
			}
		} else {
			this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  local_state_vals);
			vector<double> scope_input(solution->scope_dictionary[this->existing_scope_ids[f_index]]->num_inputs);
			for (int i_index = 0; i_index < solution->scope_dictionary[this->existing_scope_ids[f_index]]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(solution->scope_dictionary[this->existing_scope_ids[f_index]]);
			solution->scope_dictionary[this->existing_scope_ids[f_index]]->existing_flat_activate(
				problem,
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
				if (this->is_inner_scope[ff_index]) {
					input_fold_inputs[ff_index].push_back(scope_output);
				}
			}
		}
	}

	if (run_status.is_recursive) {
		this->is_recursive++;
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

	double end_scale_mod_val = this->end_scale_mod->output->constants[0];
	scale_factor *= end_scale_mod_val;
}

void Fold::flat_step_explore_on_path_backprop(vector<double>& local_state_errors,
											  double& predicted_score,
											  double target_val,
											  double final_misguess,
											  double& scale_factor,
											  double& scale_factor_error,
											  FoldHistory* history) {
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*final_misguess;
	double curr_misguess_variance = (this->average_misguess - final_misguess)*(this->average_misguess - final_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_score_variance = (this->average_score - target_val)*(this->average_score - target_val);
	this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;

	double end_scale_mod_val = this->end_scale_mod->output->constants[0];
	scale_factor /= end_scale_mod_val;

	vector<double> end_scale_mod_errors{scale_factor_error};
	if (this->state_iter <= 100000) {
		this->end_scale_mod->backprop(end_scale_mod_errors, 0.005);
	} else if (this->state_iter <= 400000) {
		this->end_scale_mod->backprop(end_scale_mod_errors, 0.001);
	} else {
		this->end_scale_mod->backprop(end_scale_mod_errors, 0.0002);
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			scope_input_errors[f_index] = vector<double>(solution->scope_dictionary[this->existing_scope_ids[f_index]]->num_outputs, 0.0);
		}
	}

	double predicted_score_error = target_val - predicted_score;

	// TODO: add normalization for inputs and look for numbers that work well in general
	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		this->sum_error += abs(predicted_score_error);
		if (this->state_iter <= 100000) {
			this->curr_end_fold->backprop(local_state_errors, 0.1);

			vector<double> curr_fold_error{scale_factor*predicted_score_error};
			this->curr_fold->backprop(curr_fold_error, 0.1);
		} else if (this->state_iter <= 400000) {
			this->curr_end_fold->backprop(local_state_errors, 0.02);

			vector<double> curr_fold_error{scale_factor*predicted_score_error};
			this->curr_fold->backprop(curr_fold_error, 0.02);
		} else {
			this->curr_end_fold->backprop(local_state_errors, 0.004);

			vector<double> curr_fold_error{scale_factor*predicted_score_error};
			this->curr_fold->backprop(curr_fold_error, 0.004);
		}
		// don't need to worry about s_input_errors and state_errors
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				for (int i_index = 0; i_index < solution->scope_dictionary[this->existing_scope_ids[f_index]]->num_outputs; i_index++) {
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
		if (this->is_inner_scope[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			solution->scope_dictionary[this->existing_scope_ids[f_index]]->existing_flat_backprop(
				scope_input_errors[f_index],
				scope_output_errors,
				predicted_score,
				predicted_score_error,
				scale_factor,
				scope_scale_factor_error,
				history->scope_histories[f_index]);

			scale_factor /= scope_scale_mod_val;

			vector<double> mod_errors{scope_scale_factor_error};
			if (this->state_iter <= 100000) {
				this->scope_scale_mod[f_index]->backprop(mod_errors, 0.005);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.05);
			} else if (this->state_iter <= 400000) {
				this->scope_scale_mod[f_index]->backprop(mod_errors, 0.001);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.01);
			} else {
				this->scope_scale_mod[f_index]->backprop(mod_errors, 0.0002);

				this->curr_input_folds[f_index]->backprop(scope_output_errors, 0.002);
			}
			for (int ff_index = f_index-1; ff_index >= 0; ff_index--) {
				if (this->is_inner_scope[ff_index]) {
					for (int i_index = 0; i_index < solution->scope_dictionary[this->existing_scope_ids[ff_index]]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	double starting_predicted_score_error = target_val - predicted_score;
	vector<double> starting_score_errors{scale_factor*starting_predicted_score_error};
	if (this->state_iter <= 100000) {
		this->starting_score_network->backprop_small_weights_with_no_error_signal(
			starting_score_errors,
			0.1);
	} else if (this->state_iter <= 400000) {
		this->starting_score_network->backprop_small_weights_with_no_error_signal(
			starting_score_errors,
			0.02);
	} else {
		this->starting_score_network->backprop_small_weights_with_no_error_signal(
			starting_score_errors,
			0.004);
	}
	// end of backprop so no need to modify predicted_score

	// occasionally train on seed to better recognize what led to this fold
	if (this->state_iter < 200000 && rand()%20 == 0) {
		this->starting_score_network->activate_small(this->seed_local_s_input_vals,
													 this->seed_local_state_vals);
		double starting_predicted_score_error = this->seed_target_val - 
			(this->seed_start_score + scale_factor*this->starting_score_network->output->acti_vals[0]);
		vector<double> starting_score_errors{scale_factor*starting_predicted_score_error};
		if (this->state_iter <= 100000) {
			this->starting_score_network->backprop_small_weights_with_no_error_signal(
				starting_score_errors,
				0.1);
		} else {
			this->starting_score_network->backprop_small_weights_with_no_error_signal(
				starting_score_errors,
				0.02);
		}
	}

	if (this->state_iter >= 490000) {
		// Note: use predicted_score_variance (as opposed to score_variance for surprise) to measure difference between predicted scores
		double score_standard_deviation = sqrt(this->existing_predicted_score_variance);
		double t_value = (history->existing_score - scale_factor*history->starting_score_update) / score_standard_deviation;
		if (t_value > 1.0) {	// >75%
			this->existing_noticably_better++;
		} else if (t_value < -1.0) {	// >75%
			this->new_noticably_better++;
		}
	}

	double higher_branch_val;
	if (history->existing_score > this->starting_score_network->output->acti_vals[0]) {
		higher_branch_val = history->existing_score;
	} else {
		higher_branch_val = scale_factor*history->starting_score_update;
	}
	double combined_score_error = higher_branch_val - scale_factor*history->combined_score_update;

	vector<double> combined_score_errors{scale_factor*combined_score_error};
	if (this->state_iter <= 100000) {
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.1);
	} else if (this->state_iter <= 400000) {
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.02);
	} else {
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.004);
	}
}
