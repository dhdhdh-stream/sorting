#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::score_step_explore_off_path_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history) {

}

void Fold::score_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::score_step_existing_flat_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::score_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::score_step_update_activate(
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
			local_state_vals.push_back(this->curr_starting_compress_network->output->acti_vals[s_index]);
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

	if (this->existing_actions[this->finished_steps.size()] == NULL) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		this->curr_inner_input_network->activate_small(s_input_vals.back(),
													   state_vals.back());
		vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
		for (int s_index = 0; s_index < this->existing_actions[this->finished_steps.size()]->num_inputs; s_index++) {
			scope_input[s_index] = this->curr_inner_input_network->output->acti_vals[s_index];
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

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}
	fold_input.push_back(vector<double>());	// empty
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			input_fold_inputs[f_index].push_back(vector<double>());	// empty
		}
	}

	FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->curr_score_network);
	this->curr_score_network->activate_subfold(local_s_input_vals,
											   state_vals,
											   score_network_history);
	history->score_network_history = score_network_history;
	history->score_update = this->curr_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_score_network->output->acti_vals[0];

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

void Fold::score_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::score_step_existing_update_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::score_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}
