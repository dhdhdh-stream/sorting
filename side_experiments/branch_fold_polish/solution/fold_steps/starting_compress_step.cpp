#include "fold.h"

#include <cmath>
#include <iostream>

#include "definitions.h"

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
	// starting_score_network activated in branch
	history->starting_score_update = starting_score;
	predicted_score += starting_score;	// already scaled

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		if (this->curr_starting_compress_new_size > 0) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* curr_starting_compress_network_history = new FoldNetworkHistory(this->curr_starting_compress_network);
				this->curr_starting_compress_network->activate_small(local_s_input_vals,
																	 local_state_vals,
																	 curr_starting_compress_network_history);
				history->curr_starting_compress_network_history = curr_starting_compress_network_history;
			} else {
				this->curr_starting_compress_network->activate_small(local_s_input_vals,
																	 local_state_vals);
			}
			local_state_vals.clear();
			local_state_vals.reserve(this->curr_starting_compress_new_size);
			for (int s_index = 0; s_index < this->curr_starting_compress_new_size; s_index++) {
				local_state_vals.push_back(this->curr_starting_compress_network->output->acti_vals[s_index]);
			}
		} else {
			// compress down to 0
			local_state_vals.clear();
		}
	}

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			fold_input.push_back(*flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(*flat_vals.begin());
				}
			}

			flat_vals.erase(flat_vals.begin());
		} else {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* curr_input_fold_history = new FoldNetworkHistory(this->curr_input_folds[f_index]);
				this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
														  local_s_input_vals,
														  local_state_vals,
														  curr_input_fold_history);
				history->curr_input_fold_histories[f_index] = curr_input_fold_history;
			} else {
				this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
														  local_s_input_vals,
														  local_state_vals);
			}
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
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(scope_output);
				}
			}
		}
	}

	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* curr_fold_history = new FoldNetworkHistory(this->curr_fold);
		this->curr_fold->activate(fold_input,
								  local_s_input_vals,
								  local_state_vals,
								  curr_fold_history);
		history->curr_fold_history = curr_fold_history;
	} else {
		this->curr_fold->activate(fold_input,
								  local_s_input_vals,
								  local_state_vals);
	}
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* curr_end_fold_history = new FoldNetworkHistory(this->curr_end_fold);
		this->curr_end_fold->activate(fold_input,
									  local_s_input_vals,
									  local_state_vals,
									  curr_end_fold_history);
		history->curr_end_fold_history = curr_end_fold_history;
	} else {
		this->curr_end_fold->activate(fold_input,
									  local_s_input_vals,
									  local_state_vals);
	}
	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	// end_scale_mod passed on
}

void Fold::starting_compress_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	// end_scale_mod passed on

	double predicted_score_error = target_val - predicted_score;

	scale_factor_error += history->ending_score_update*predicted_score_error;

	this->curr_end_fold->backprop_errors_with_no_weight_change(
		local_state_errors,
		history->curr_end_fold_history);

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop_errors_with_no_weight_change(
		curr_fold_error,
		history->curr_fold_history);

	local_state_errors = vector<double>(this->curr_scope_sizes[0], 0.0);

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	for (int st_index = 0; st_index < (int)local_s_input_errors.size(); st_index++) {
		local_s_input_errors[st_index] += this->curr_end_fold->s_input_input->errors[st_index];
		this->curr_end_fold->s_input_input->errors[st_index] = 0.0;

		local_s_input_errors[st_index] += this->curr_fold->s_input_input->errors[st_index];
		this->curr_fold->s_input_input->errors[st_index] = 0.0;
	}
	for (int st_index = 0; st_index < (int)local_state_errors.size(); st_index++) {
		local_state_errors[st_index] += this->curr_end_fold->state_inputs.back()->errors[st_index];
		this->curr_end_fold->state_inputs.back()->errors[st_index] = 0.0;

		local_state_errors[st_index] += this->curr_fold->state_inputs.back()->errors[st_index];
		this->curr_fold->state_inputs.back()->errors[st_index] = 0.0;
	}
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

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size(); f_index--) {
		if (this->is_existing[f_index]) {
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

			scale_factor_error += scope_scale_mod*scope_scale_factor_error;

			scale_factor /= scope_scale_mod;

			this->curr_input_folds[f_index]->backprop_errors_with_no_weight_change(
				scope_output_errors,
				history->curr_input_fold_histories[f_index]);
			for (int st_index = 0; st_index < (int)local_s_input_errors.size(); st_index++) {
				local_s_input_errors[st_index] += this->curr_input_folds[f_index]->s_input_input->errors[st_index];
				this->curr_input_folds[f_index]->s_input_input->errors[st_index] = 0.0;
			}
			for (int st_index = 0; st_index < (int)local_state_errors.size(); st_index++) {
				local_state_errors[st_index] += this->curr_input_folds[f_index]->state_inputs.back()->errors[st_index];
				this->curr_input_folds[f_index]->state_inputs.back()->errors[st_index] = 0.0;
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

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		if (this->curr_starting_compress_new_size > 0) {
			vector<double> s_input_output_errors;
			vector<double> state_output_errors;
			this->curr_starting_compress_network->backprop_small_errors_with_no_weight_change(
				local_state_errors,
				s_input_output_errors,
				state_output_errors,
				history->curr_starting_compress_network_history);
			for (int s_index = 0; s_index < (int)local_s_input_errors.size(); s_index++) {
				local_s_input_errors[s_index] += s_input_output_errors[s_index];
			}
			local_state_errors.clear();
			local_state_errors.reserve(this->starting_compress_original_size);
			for (int s_index = 0; s_index < this->starting_compress_original_size; s_index++) {
				local_state_errors.push_back(state_output_errors[s_index]);
			}
		} else {
			// local_state_errors.size() == 0
			local_state_errors = vector<double>(this->starting_compress_original_size, 0.0);
		}
	}

	predicted_score -= history->starting_score_update;	// already scaled
}

void Fold::starting_compress_step_existing_flat_activate(
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

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		if (this->curr_starting_compress_new_size > 0) {
			FoldNetworkHistory* curr_starting_compress_network_history = new FoldNetworkHistory(this->curr_starting_compress_network);
			this->curr_starting_compress_network->activate_small(local_s_input_vals,
																 local_state_vals,
																 curr_starting_compress_network_history);
			history->curr_starting_compress_network_history = curr_starting_compress_network_history;
			local_state_vals.clear();
			local_state_vals.reserve(this->curr_starting_compress_new_size);
			for (int s_index = 0; s_index < this->curr_starting_compress_new_size; s_index++) {
				local_state_vals.push_back(this->curr_starting_compress_network->output->acti_vals[s_index]);
			}
		} else {
			// compress down to 0
			local_state_vals.clear();
		}
	}

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			fold_input.push_back(*flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(*flat_vals.begin());
				}
			}

			flat_vals.erase(flat_vals.begin());
		} else {
			FoldNetworkHistory* curr_input_fold_history = new FoldNetworkHistory(this->curr_input_folds[f_index]);
			this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  local_state_vals,
													  curr_input_fold_history);
			history->curr_input_fold_histories[f_index] = curr_input_fold_history;
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
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(scope_output);
				}
			}
		}
	}

	FoldNetworkHistory* curr_fold_history = new FoldNetworkHistory(this->curr_fold);
	this->curr_fold->activate(fold_input,
							  local_s_input_vals,
							  local_state_vals,
							  curr_fold_history);
	history->curr_fold_history = curr_fold_history;
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	FoldNetworkHistory* curr_end_fold_history = new FoldNetworkHistory(this->curr_end_fold);
	this->curr_end_fold->activate(fold_input,
								  local_s_input_vals,
								  local_state_vals,
								  curr_end_fold_history);
	history->curr_end_fold_history = curr_end_fold_history;
	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	// end_scale_mod passed on
}

void Fold::starting_compress_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	// end_scale_mod passed on

	scale_factor_error += history->ending_score_update*predicted_score_error;

	this->curr_end_fold->backprop_errors_with_no_weight_change(
		local_state_errors,
		history->curr_end_fold_history);

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop_errors_with_no_weight_change(
		curr_fold_error,
		history->curr_fold_history);

	local_state_errors = vector<double>(this->curr_scope_sizes[0], 0.0);

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	for (int st_index = 0; st_index < (int)local_s_input_errors.size(); st_index++) {
		local_s_input_errors[st_index] += this->curr_end_fold->s_input_input->errors[st_index];
		this->curr_end_fold->s_input_input->errors[st_index] = 0.0;

		local_s_input_errors[st_index] += this->curr_fold->s_input_input->errors[st_index];
		this->curr_fold->s_input_input->errors[st_index] = 0.0;
	}
	for (int st_index = 0; st_index < (int)local_state_errors.size(); st_index++) {
		local_state_errors[st_index] += this->curr_end_fold->state_inputs.back()->errors[st_index];
		this->curr_end_fold->state_inputs.back()->errors[st_index] = 0.0;

		local_state_errors[st_index] += this->curr_fold->state_inputs.back()->errors[st_index];
		this->curr_fold->state_inputs.back()->errors[st_index] = 0.0;
	}
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

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size(); f_index--) {
		if (this->is_existing[f_index]) {
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

			scale_factor_error += scope_scale_mod*scope_scale_factor_error;

			scale_factor /= scope_scale_mod;

			this->curr_input_folds[f_index]->backprop_errors_with_no_weight_change(
				scope_output_errors,
				history->curr_input_fold_histories[f_index]);
			for (int st_index = 0; st_index < (int)local_s_input_errors.size(); st_index++) {
				local_s_input_errors[st_index] += this->curr_input_folds[f_index]->s_input_input->errors[st_index];
				this->curr_input_folds[f_index]->s_input_input->errors[st_index] = 0.0;
			}
			for (int st_index = 0; st_index < (int)local_state_errors.size(); st_index++) {
				local_state_errors[st_index] += this->curr_input_folds[f_index]->state_inputs.back()->errors[st_index];
				this->curr_input_folds[f_index]->state_inputs.back()->errors[st_index] = 0.0;
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

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		if (this->curr_starting_compress_new_size > 0) {
			vector<double> s_input_output_errors;
			vector<double> state_output_errors;
			this->curr_starting_compress_network->backprop_small_errors_with_no_weight_change(
				local_state_errors,
				s_input_output_errors,
				state_output_errors,
				history->curr_starting_compress_network_history);
			for (int s_index = 0; s_index < (int)local_s_input_errors.size(); s_index++) {
				local_s_input_errors[s_index] += s_input_output_errors[s_index];
			}
			local_state_errors.clear();
			local_state_errors.reserve(this->starting_compress_original_size);
			for (int s_index = 0; s_index < this->starting_compress_original_size; s_index++) {
				local_state_errors.push_back(state_output_errors[s_index]);
			}
		} else {
			// local_state_errors.size() == 0
			local_state_errors = vector<double>(this->starting_compress_original_size, 0.0);
		}
	}

	predicted_score -= history->starting_score_update;	// already scaled
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

	// this->test_starting_compress_new_size < this->starting_compress_original_size
	vector<double> test_local_state_vals(this->test_starting_compress_new_size);
	vector<double> test_local_state_errors(this->test_starting_compress_new_size, 0.0);

	if (this->test_starting_compress_new_size > 0) {
		this->test_starting_compress_network->activate_small(local_s_input_vals,
															 local_state_vals);
		for (int s_index = 0; s_index < this->test_starting_compress_new_size; s_index++) {
			test_local_state_vals[s_index] = this->new_state_factor*this->test_starting_compress_network->output->acti_vals[s_index];
		}
	} else {
		// compress down to 0
		test_local_state_vals.clear();
	}

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		// this->curr_starting_compress_new_size > 0
		this->curr_starting_compress_network->activate_small(local_s_input_vals,
															 local_state_vals);
		local_state_vals.clear();
		local_state_vals.reserve(this->curr_starting_compress_new_size);
		for (int s_index = 0; s_index < this->curr_starting_compress_new_size; s_index++) {
			local_state_vals.push_back(this->curr_starting_compress_network->output->acti_vals[s_index]);
		}
	}

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			fold_input.push_back(*flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(*flat_vals.begin());
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
				this->test_input_folds[f_index]->backprop(input_fold_errors, 0.01);
			} else {
				this->test_input_folds[f_index]->backprop(input_fold_errors, 0.002);
			}
			for (int s_index = 0; s_index < this->test_starting_compress_new_size; s_index++) {
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
				if (this->is_existing[i_index]) {
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

	double fold_error = this->curr_fold->output->acti_vals[0] - this->test_fold->output->acti_vals[0];
	this->sum_error += fold_error*fold_error;
	vector<double> fold_errors{fold_error};
	if (this->state_iter <= 270000) {
		this->test_fold->backprop(fold_errors, 0.01);
	} else {
		this->test_fold->backprop(fold_errors, 0.002);
	}
	for (int s_index = 0; s_index < this->test_starting_compress_new_size; s_index++) {
		test_local_state_errors[s_index] += this->test_fold->state_inputs.back()->errors[s_index];
		this->test_fold->state_inputs.back()->errors[s_index] = 0.0;
	}

	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	this->curr_end_fold->activate(fold_input,
								  local_s_input_vals,
								  local_state_vals);
	
	this->test_end_fold->activate(fold_input,
								  local_s_input_vals,
								  test_local_state_vals);

	vector<double> end_fold_errors(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		end_fold_errors[o_index] = this->curr_end_fold->output->acti_vals[o_index]
			- this->test_end_fold->output->acti_vals[o_index];
		this->sum_error += end_fold_errors[o_index]*end_fold_errors[o_index];
	}
	if (this->state_iter <= 270000) {
		this->test_end_fold->backprop(end_fold_errors, 0.01);
	} else {
		this->test_end_fold->backprop(end_fold_errors, 0.002);
	}
	for (int s_index = 0; s_index < this->test_starting_compress_new_size; s_index++) {
		test_local_state_errors[s_index] += this->test_end_fold->state_inputs.back()->errors[s_index];
		this->test_end_fold->state_inputs.back()->errors[s_index] = 0.0;
	}

	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	if (this->test_starting_compress_new_size > 0) {
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
	}

	// end_scale_mod passed on
}

void Fold::starting_compress_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {
	// end_scale_mod passed on

	double predicted_score_error = target_val - predicted_score;

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop_weights_with_no_error_signal(
		curr_fold_error,
		0.001,
		history->curr_fold_history);

	next_predicted_score = predicted_score;
	predicted_score -= scale_factor*history->ending_score_update;

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size(); f_index--) {
		if (this->is_existing[f_index]) {
			double scope_scale_mod = this->scope_scale_mod_calcs[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod;

			double scope_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_update_backprop(predicted_score,
																	  predicted_score_error,
																	  scale_factor,
																	  scope_scale_factor_error,
																	  history->scope_histories[f_index]);

			scope_scale_factor_error *= scope_scale_mod;

			scale_factor /= scope_scale_mod;

			vector<double> mod_errors{scope_scale_factor_error};
			this->scope_scale_mod_calcs[f_index]->backprop(mod_errors, 0.0002);
		}
	}

	double misguess = abs(target_val - predicted_score);
	this->starting_average_misguess = 0.999*this->starting_average_misguess + 0.001*misguess;

	next_predicted_score = predicted_score;
	predicted_score -= history->starting_score_update;	// already scaled

	this->starting_average_local_impact = 0.999*this->starting_average_local_impact
		+ 0.001*abs(history->starting_score_update);
}

void Fold::starting_compress_step_existing_update_activate(
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

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		if (this->curr_starting_compress_new_size > 0) {
			this->curr_starting_compress_network->activate_small(local_s_input_vals,
																 local_state_vals);
			local_state_vals.clear();
			local_state_vals.reserve(this->curr_starting_compress_new_size);
			for (int s_index = 0; s_index < this->curr_starting_compress_new_size; s_index++) {
				local_state_vals.push_back(this->curr_starting_compress_network->output->acti_vals[s_index]);
			}
		} else {
			// compress down to 0
			local_state_vals.clear();
		}
	}

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			fold_input.push_back(*flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(*flat_vals.begin());
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
				if (this->is_existing[i_index]) {
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

	// end_scale_mod passed on
}

void Fold::starting_compress_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	// end_scale_mod passed on

	scale_factor_error += history->ending_score_update*predicted_score_error;

	predicted_score -= scale_factor*history->ending_score_update;

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size(); f_index--) {
		if (this->is_existing[f_index]) {
			double scope_scale_mod = this->scope_scale_mod_calcs[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod;

			double scope_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_update_backprop(predicted_score,
																	  predicted_score_error,
																	  scale_factor,
																	  scope_scale_factor_error,
																	  history->scope_histories[f_index]);

			scale_factor_error += scope_scale_mod*scope_scale_factor_error;

			scale_factor /= scope_scale_mod;
		}
	}

	predicted_score -= history->starting_score_update;	// already scaled
}
