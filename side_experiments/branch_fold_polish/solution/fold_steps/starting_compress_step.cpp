#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::starting_compress_step_explore_off_path_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history) {

}

void Fold::starting_compress_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::starting_compress_step_existing_flat_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::starting_compress_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::starting_compress_step_update_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {
	// starting_score_network activated in branch
	history->starting_score_update = starting_score;
	predicted_score += starting_score;	// already scaled

	// this->starting_compress_size > 0

	int test_starting_compress_new_size = (int)local_state_vals.size() - this->test_starting_compress_size;
	vector<double> test_local_state_vals(test_starting_compress_new_size);
	vector<double> test_local_state_errors(test_starting_compress_new_size, 0.0);

	this->test_starting_compress_network->activate_small(local_s_input_vals,
														 local_state_vals);
	for (int s_index = 0; s_index < test_starting_compress_new_size; s_index++) {
		test_local_state_vals[s_index] = this->test_starting_compress_network->output->acti_vals[s_index];
	}

	if (this->curr_starting_compress_network != NULL) {
		this->curr_starting_compress_network->activate_small(local_s_input_vals,
															 local_state_vals);
		int compress_new_size = (int)local_state_vals.size() - this->curr_starting_compress_size;
		local_state_vals.clear();
		local_state_vals.reserve(compress_new_size);
		for (int s_index = 0; s_index < compress_new_size; s_index++) {
			local_state_vals.push_back(this->curr_starting_compress_network->output->acti_vals[s_index]);
		}
	}

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

			this->test_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  test_local_state_vals);

			vector<double> input_fold_errors(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				input_fold_errors[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index]
					- this->test_input_folds[f_index]->output->acti_vals[i_index];
				this->sum_error += input_fold_errors[i_index]*input_fold_errors[i_index];
			}
			if (this->state_iter <= 270000) {
				this->test_input_folds[f_index]->backprop_last_state(input_fold_errors, 0.01);
			} else {
				this->test_input_folds[f_index]->backprop_last_state(input_fold_errors, 0.002);
			}
			for (int s_index = 0; s_index < test_starting_compress_new_size; s_index++) {
				test_local_state_errors[s_index] += this->test_input_folds[f_index]->state_inputs.back()->errors[s_index];
				this->test_input_folds[f_index]->state_inputs.back()->errors[s_index] = 0.0;
			}

			vector<double> scope_input(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod = this->scope_scale_mod_calcs[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[f_index]);
			this->existing_actions[f_index]->existing_update_activate(flat_vals,
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

	FoldNetworkHistory* curr_fold_history = new FoldNetworkHistory(this->curr_fold);
	// TODO: if pointers don't match, then don't backprop
	this->curr_fold->activate(fold_input,
							  local_s_input_vals,
							  local_state_vals,
							  curr_fold_history);
	history->curr_fold_history = curr_fold_history;
	history->ending_score_update = this->curr_fold->output->acti_vals[0];

	this->test_fold->activate(fold_input,
							  local_s_input_vals,
							  test_local_state_vals);

	// TODO: special case empty state
	vector<double> fold_errors{this->curr_fold->output->acti_vals[0] - this->test_fold->output->acti_vals[0]};
	if (this->state_iter <= 270000) {
		this->test_fold->backprop_last_state(fold_errors, 0.01);
	} else {
		this->test_fold->backprop_last_state(fold_errors, 0.002);
	}
	for (int s_index = 0; s_index < test_starting_compress_new_size; s_index++) {
		test_local_state_errors[s_index] += this->test_fold->state_inputs.back()->errors[s_index];
		this->test_fold->state_inputs.back()->errors[s_index] = 0.0;
	}

	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	if (this->state_iter <= 240000) {
		this->test_starting_compress_network->backprop_small_weights_with_no_error_signal(
			test_local_state_errors,
			0.05);
	} else if (this->state_iter <= 270000) {
		this->test_starting_compress_network->backprop_small_weights_with_no_error_signal(
			test_local_state_errors,
			0.01);
	} else {
		this->test_starting_compress_network->backprop_small_weights_with_no_error_signal(
			test_local_state_errors,
			0.002);
	}

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

void Fold::starting_compress_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::starting_compress_step_existing_update_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::starting_compress_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}
