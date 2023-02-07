#include "fold.h"

#include <cmath>
#include <iostream>

#include "constants.h"

using namespace std;

void Fold::starting_compress_step_explore_off_path_activate(
		Problem& problem,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		RunStatus& run_status,
		FoldHistory* history) {
	// starting_score_network activated in branch
	history->starting_score_update = starting_score;
	predicted_score += starting_score;	// already scaled

	if (this->curr_starting_compress_new_size < this->starting_compress_original_size) {
		if (this->curr_starting_compress_new_size > 0) {
			if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
			if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
			vector<double> scope_input(this->scopes[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->scopes[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[f_index]);
			this->scopes[f_index]->existing_flat_activate(problem,
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

	if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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

	if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::starting_compress_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->scopes[f_index]->num_outputs, 0.0);
		}
	}

	double predicted_score_error = target_val - predicted_score;

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		this->curr_end_fold->backprop_errors_with_no_weight_change(
			local_state_errors,
			history->curr_end_fold_history);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop_errors_with_no_weight_change(
			curr_fold_error,
			history->curr_fold_history);

		local_state_errors = vector<double>(this->curr_scope_sizes[0], 0.0);

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
			if (this->is_inner_scope[f_index]) {
				for (int i_index = 0; i_index < this->scopes[f_index]->num_outputs; i_index++) {
					scope_input_errors[f_index][i_index] += this->curr_end_fold->flat_inputs[f_index]->errors[i_index];
					this->curr_end_fold->flat_inputs[f_index]->errors[i_index] = 0.0;

					scope_input_errors[f_index][i_index] += this->curr_fold->flat_inputs[f_index]->errors[i_index];
					this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
				}
			}
		}
		predicted_score -= scale_factor*history->ending_score_update;
	} else {
		local_state_errors = vector<double>(this->curr_scope_sizes[0], 0.0);
	}

	for (int f_index = history->exit_index; f_index >= 0; f_index--) {
		if (this->is_inner_scope[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			this->scopes[f_index]->existing_flat_backprop(scope_input_errors[f_index],
																	scope_output_errors,
																	predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scope_scale_factor_error,
																	history->scope_histories[f_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

			// don't update scope_scale_mod on explore_off_path

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
				if (this->is_inner_scope[ff_index]) {
					for (int i_index = 0; i_index < this->scopes[ff_index]->num_outputs; i_index++) {
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
		Problem& problem,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		RunStatus& run_status,
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
			FoldNetworkHistory* curr_input_fold_history = new FoldNetworkHistory(this->curr_input_folds[f_index]);
			this->curr_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  local_state_vals,
													  curr_input_fold_history);
			history->curr_input_fold_histories[f_index] = curr_input_fold_history;
			vector<double> scope_input(this->scopes[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->scopes[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[f_index]);
			this->scopes[f_index]->existing_flat_activate(problem,
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
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::starting_compress_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->scopes[f_index]->num_outputs, 0.0);
		}
	}

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		this->curr_end_fold->backprop_errors_with_no_weight_change(
			local_state_errors,
			history->curr_end_fold_history);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop_errors_with_no_weight_change(
			curr_fold_error,
			history->curr_fold_history);

		local_state_errors = vector<double>(this->curr_scope_sizes[0], 0.0);

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
			if (this->is_inner_scope[f_index]) {
				for (int i_index = 0; i_index < this->scopes[f_index]->num_outputs; i_index++) {
					scope_input_errors[f_index][i_index] += this->curr_end_fold->flat_inputs[f_index]->errors[i_index];
					this->curr_end_fold->flat_inputs[f_index]->errors[i_index] = 0.0;

					scope_input_errors[f_index][i_index] += this->curr_fold->flat_inputs[f_index]->errors[i_index];
					this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
				}
			}
		}
		predicted_score -= scale_factor*history->ending_score_update;
	} else {
		local_state_errors = vector<double>(this->curr_scope_sizes[0], 0.0);
	}

	for (int f_index = history->exit_index; f_index >= 0; f_index--) {
		if (this->is_inner_scope[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			this->scopes[f_index]->existing_flat_backprop(scope_input_errors[f_index],
																	scope_output_errors,
																	predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scope_scale_factor_error,
																	history->scope_histories[f_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

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
				if (this->is_inner_scope[ff_index]) {
					for (int i_index = 0; i_index < this->scopes[ff_index]->num_outputs; i_index++) {
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
		Problem& problem,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		RunStatus& run_status,
		FoldHistory* history) {
	// starting_score_network activated in branch
	history->starting_score_update = starting_score;
	predicted_score += starting_score;	// already scaled

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

			this->test_input_folds[f_index]->activate(input_fold_inputs[f_index],
													  local_s_input_vals,
													  test_local_state_vals);

			vector<double> input_fold_errors(this->scopes[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->scopes[f_index]->num_inputs; i_index++) {
				input_fold_errors[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index]
					- this->test_input_folds[f_index]->output->acti_vals[i_index];
				this->sum_error += input_fold_errors[i_index]*input_fold_errors[i_index];
			}
			if (this->state_iter <= 130000) {
				this->test_input_folds[f_index]->backprop(input_fold_errors, 0.01);
			} else {
				this->test_input_folds[f_index]->backprop(input_fold_errors, 0.002);
			}
			for (int s_index = 0; s_index < this->test_starting_compress_new_size; s_index++) {
				test_local_state_errors[s_index] += this->test_input_folds[f_index]->state_inputs.back()->errors[s_index];
				this->test_input_folds[f_index]->state_inputs.back()->errors[s_index] = 0.0;
			}

			vector<double> scope_input(this->scopes[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->scopes[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[f_index]);
			this->scopes[f_index]->existing_update_activate(problem,
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
				break;
			}

			fold_input.push_back(scope_output);
			for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
				if (this->is_inner_scope[ff_index]) {
					input_fold_inputs[ff_index].push_back(scope_output);
				}
			}
		}
	}

	if (!run_status.exceeded_depth) {
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
		if (this->state_iter <= 130000) {
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

		vector<double> end_fold_errors(this->num_outputs);
		for (int o_index = 0; o_index < this->num_outputs; o_index++) {
			end_fold_errors[o_index] = this->curr_end_fold->output->acti_vals[o_index]
				- this->test_end_fold->output->acti_vals[o_index];
			this->sum_error += end_fold_errors[o_index]*end_fold_errors[o_index];
		}
		if (this->state_iter <= 130000) {
			this->test_end_fold->backprop(end_fold_errors, 0.01);
		} else {
			this->test_end_fold->backprop(end_fold_errors, 0.002);
		}
		for (int s_index = 0; s_index < this->test_starting_compress_new_size; s_index++) {
			test_local_state_errors[s_index] += this->test_end_fold->state_inputs.back()->errors[s_index];
			this->test_end_fold->state_inputs.back()->errors[s_index] = 0.0;
		}

		local_state_vals.clear();
		local_state_vals.reserve(this->num_outputs);
		for (int o_index = 0; o_index < this->num_outputs; o_index++) {
			local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
		}
	}

	if (this->test_starting_compress_new_size > 0) {
		if (this->state_iter <= 110000) {
			this->test_starting_compress_network->backprop_small_weights_with_no_error_signal(
				test_local_state_errors,
				0.05);
		} else if (this->state_iter <= 130000) {
			this->test_starting_compress_network->backprop_small_weights_with_no_error_signal(
				test_local_state_errors,
				0.01);
		} else {
			this->test_starting_compress_network->backprop_small_weights_with_no_error_signal(
				test_local_state_errors,
				0.002);
		}
	}
}

void Fold::starting_compress_step_update_backprop(
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

	double predicted_score_error = target_val - predicted_score;

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop_weights_with_no_error_signal(
			curr_fold_error,
			0.001,
			history->curr_fold_history);

		predicted_score -= scale_factor*history->ending_score_update;
	}

	for (int f_index = history->exit_index; f_index >= 0; f_index--) {
		if (this->is_inner_scope[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			double scope_scale_factor_error = 0.0;
			this->scopes[f_index]->existing_update_backprop(predicted_score,
																	  predicted_score_error,
																	  scale_factor,
																	  scope_scale_factor_error,
																	  history->scope_histories[f_index]);

			vector<double> mod_errors{scope_scale_factor_error};
			this->scope_scale_mod[f_index]->backprop(mod_errors, 0.0002);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;
		}
	}

	predicted_score -= history->starting_score_update;	// already scaled

	this->starting_average_local_impact = 0.9999*this->starting_average_local_impact
		+ 0.0001*abs(history->starting_score_update);
}

void Fold::starting_compress_step_existing_update_activate(
		Problem& problem,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		RunStatus& run_status,
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
			vector<double> scope_input(this->scopes[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->scopes[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[f_index]);
			this->scopes[f_index]->existing_update_activate(problem,
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

void Fold::starting_compress_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		predicted_score -= scale_factor*history->ending_score_update;
	}

	for (int f_index = history->exit_index; f_index >= 0; f_index--) {
		if (this->is_inner_scope[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			double scope_scale_factor_error = 0.0;
			this->scopes[f_index]->existing_update_backprop(predicted_score,
																	  predicted_score_error,
																	  scale_factor,
																	  scope_scale_factor_error,
																	  history->scope_histories[f_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;
		}
	}

	predicted_score -= history->starting_score_update;	// already scaled
}
