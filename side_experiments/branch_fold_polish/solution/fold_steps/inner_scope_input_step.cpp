#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::inner_scope_input_step_explore_off_path_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_flat_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_update_activate(
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

	// starting compress for Folds will always be active_compress (vs. existing path which may not be)
	if (this->starting_compress_size > 0) {
		this->curr_starting_compress_network->activate_small(local_s_input_vals,
															 local_state_vals);
		int compress_new_size = (int)local_state_vals.size() - this->curr_starting_compress_size;
		local_state_vals.clear();
		local_state_vals.reserve(compress_new_size);
		for (int s_index = 0; s_index < compress_new_size; s_index++) {
			local_state_vals.push_back(this->starting_compress_network->output->acti_vals[s_index]);
		}
	}

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int n_index = 0; n_index < (int)this->finished_steps.size(); n_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[n_index]);
		this->finished_steps[n_index]->update_activate(flat_vals,
													   s_input_vals,
													   state_vals,
													   predicted_score,
													   scale_factor,
													   finished_step_history);
		history->finished_step_histories[n_index] = finished_step_history;

		fold_input.push_back(vector<double>());	// empty
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->existing_actions[f_index] != NULL) {
				input_fold_inputs[f_index].push_back(vector<double>());	// empty
			}
		}
	}

	// this->existing_actions[this->finished_steps.size()] != NULL
	if (this->inner_input_input_networks.size() > 0) {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size()-1; i_index++) {
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}
		this->inner_input_input_networks.back()->activate_small(s_input_vals[this->inner_input_input_layer.back()],
																state_vals[this->inner_input_input_layer.back()]);
		for (int s_index = 0; s_index < this->inner_input_input_sizes.back(); s_index++) {
			s_input_vals[this->inner_input_input_layer.back()+1].push_back(
				this->new_state_factor*this->inner_input_input_networks.back()->output->acti_vals[s_index]);
		}
	}

	// TODO: change to compare against curr_input_network and subfold?
	vector<double> empty_input_fold_input;
	this->curr_input_folds[this->finished_steps.size()]->activate_fold(empty_input_fold_input,
																	   local_s_input_vals,
																	   state_vals);
	vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		scope_input[i_index] = this->curr_input_folds[this->finished_steps.size()]->output->acti_vals[i_index];
	}

	this->test_input_network->activate_subfold(empty_input_fold_input,
											   s_input_vals[this->test_input_network->subfold_index],
											   state_vals);

	vector<double> inner_input_errors(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		inner_input_errors[i_index] = this->curr_input_folds[this->finished_steps.size()]->output->acti_vals[i_index]
			- this->test_input_network->output->acti_vals[i_index];
		this->sum_error += inner_input_errors[i_index]*inner_input_errors[i_index];
	}
	if (this->inner_input_input_networks.size() == 0) {
		if (this->state_iter <= 270000) {
			this->test_input_network->backprop_subfold_weights_with_no_error_signal(
				inner_input_errors,
				0.01);
		} else {
			this->test_input_network->backprop_subfold_weights_with_no_error_signal(
				inner_input_errors,
				0.002);
		}
	} else {
		if (this->state_iter <= 270000) {
			this->test_input_network->backprop_new_s_input(
				this->inner_input_input_layer.back()+1,
				this->inner_input_input_sizes.back(),
				inner_input_errors,
				0.01);
		} else {
			this->test_input_network->backprop_new_s_input(
				this->inner_input_input_layer.back()+1,
				this->inner_input_input_sizes.back(),
				inner_input_errors,
				0.002);
		}
		vector<double> inner_input_input_errors;
		inner_input_input_errors.reserve(this->inner_input_input_sizes.back());
		int layer_size = (int)this->test_input_network->s_input_input->errors.size();
		for (int st_index = layer_size-this->inner_input_input_sizes.back(); st_index < layer_size; st_index++) {
			inner_input_input_errors.push_back(this->test_input_network->s_input_input->errors[st_index]);
			this->test_input_network->s_input_input->errors[st_index] = 0.0;
		}
		if (this->state_iter <= 240000) {
			this->inner_input_input_networks.back()->backprop_small_weights_with_no_error_signal(
				inner_input_input_errors,
				0.05);
		} else if (this->state_iter <= 270000) {
			this->inner_input_input_networks.back()->backprop_small_weights_with_no_error_signal(
				inner_input_input_errors,
				0.01);
		} else {
			this->inner_input_input_networks.back()->backprop_small_weights_with_no_error_signal(
				inner_input_input_errors,
				0.002);
		}
	}

	double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
	scale_factor *= scope_scale_mod;

	vector<double> scope_output;
	ScoopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
	this->existing_actions[this->finished_steps.size()]->update_activate(flat_vals,
																		 scope_input,
																		 scope_output,
																		 predicted_score,
																		 scale_factor,
																		 scope_history);
	history->scope_histories[this->finished_steps.size()] = scope_history;

	scale_factor /= scope_scale_mod;

	// not folded yet
	fold_input.push_back(scope_output);
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			input_fold_inputs[f_index].push_back(scope_output);
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] == NULL) {
			fold_input.push_back(flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->existing_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(flat_vals.begin());
				}
			}

			flat_vals.erase(flat_vals.begin());
		} else {
			this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
														   local_s_input_vals,
														   state_vals);
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
	this->curr_fold->activate_fold(fold_input,
								   local_s_input_vals,
								   state_vals,
								   curr_fold_history);
	history->curr_fold_history = curr_fold_history;
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	this->curr_end_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   state_vals);
	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	double end_scale_mod_val = this->end_scale_mod_calc->output->constants[0];
	scale_factor *= end_scale_mod_val;
}

void Fold::inner_scope_input_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {
	double end_scale_mod_val = this->end_scale_mod_calc->output->constants[0];
	scale_factor /= end_scale_mod_val;

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop(curr_fold_error,
							  0.001,
							  history->curr_fold_history);

	next_predicted_score = predicted_score;	// doesn't matter
	predicted_score -= scale_factor*this->ending_score_update;

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size()+1; f_index--) {
		if (this->existing_actions[f_index] != NULL) {
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

			this->scope_scale_mod_calcs[f_index]->backprop(scope_scale_factor_error, 0.0002);
		}
	}

	double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
	scale_factor *= scope_scale_mod;

	this->existing_actions[this->finished_steps.size()]->update_backprop(predicted_score,
																		 next_predicted_score,
																		 target_val,
																		 scale_factor,
																		 history->scope_histories[f_index]);

	scale_factor /= scope_scale_mod;

	for (int n_index = (int)this->finished_steps.size()-1; n_index >= 0; n_index--) {
		this->finished_steps[n_index]->update_backprop(predicted_score,
													   next_predicted_score,
													   target_val,
													   scale_factor,
													   history->finished_step_histories[n_index]);
	}

	next_predicted_score = predicted_score;
	// starting score_network updated in branch
	predicted_score -= history->starting_score_update;	// already scaled
}

void Fold::inner_scope_input_step_existing_update_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}
