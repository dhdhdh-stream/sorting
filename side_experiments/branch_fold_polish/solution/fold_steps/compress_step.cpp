#include "fold.h"

#include <cmath>
#include <iostream>

#include "definitions.h"

using namespace std;

void Fold::compress_step_explore_off_path_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int n_index = 0; n_index < (int)this->finished_steps.size(); n_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[n_index]);
		this->finished_steps[n_index]->explore_off_path_activate(flat_vals,
																 s_input_vals,
																 state_vals,
																 predicted_score,
																 scale_factor,
																 explore_phase,
																 finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		fold_input.push_back(vector<double>());	// empty
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				input_fold_inputs[f_index].push_back(vector<double>());	// empty
			}
		}
	}

	if (!this->is_existing[this->finished_steps.size()]) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* inner_input_input_network_history = new FoldNetworkHistory(this->inner_input_input_networks[i_index]);
				this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																		  state_vals[this->inner_input_input_layer[i_index]],
																		  inner_input_input_network_history);
				history->inner_input_input_network_histories.push_back(inner_input_input_network_history);
			} else {
				this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																		  state_vals[this->inner_input_input_layer[i_index]]);
			}
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		if (explore_phase == EXPLORE_PHASE_FLAT) {
			FoldNetworkHistory* curr_input_network_history = new FoldNetworkHistory(this->curr_input_network);
			this->curr_input_network->activate_small(s_input_vals.back(),
													 state_vals.back(),
													 curr_input_network_history);
			history->curr_input_network_history = curr_input_network_history;
		} else {
			this->curr_input_network->activate_small(s_input_vals.back(),
													 state_vals.back());
		}
		vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
		for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
			scope_input[i_index] = this->curr_input_network->output->acti_vals[i_index];
		}

		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
		this->existing_actions[this->finished_steps.size()]->explore_off_path_activate(
			flat_vals,
			scope_input,
			scope_output,
			predicted_score,
			scale_factor,
			explore_phase,
			scope_history);
		history->scope_histories[this->finished_steps.size()] = scope_history;

		scale_factor /= scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}
	fold_input.push_back(vector<double>());	// empty
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(vector<double>());	// empty
		}
	}

	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* curr_score_network_history = new FoldNetworkHistory(this->curr_score_network);
		this->curr_score_network->activate_subfold(local_s_input_vals,
												   state_vals,
												   curr_score_network_history);
		history->curr_score_network_history = curr_score_network_history;
	} else {
		this->curr_score_network->activate_subfold(local_s_input_vals,
												   state_vals);
	}
	history->score_update = this->curr_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_score_network->output->acti_vals[0];

	if (this->curr_compress_num_layers > 0) {
		if (this->curr_compress_new_size > 0) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* curr_compress_network_history = new FoldNetworkHistory(this->curr_compress_network);
				this->curr_compress_network->activate_subfold(local_s_input_vals,
															  state_vals,
															  curr_compress_network_history);
				history->curr_compress_network_history = curr_compress_network_history;
			} else {
				this->curr_compress_network->activate_subfold(local_s_input_vals,
															  state_vals);
			}
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals (though doesn't matter here)
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->curr_compress_new_size));
			for (int s_index = 0; s_index < this->curr_compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->curr_compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			if (s_input_vals.size() == 1) {
				// edge case where set last layer to 0 instead of removing
				state_vals[0].clear();
			} else {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
				this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
															   local_s_input_vals,
															   state_vals,
															   curr_input_fold_history);
				history->curr_input_fold_histories[f_index] = curr_input_fold_history;
			} else {
				this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
															   local_s_input_vals,
															   state_vals);
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
		// TODO: if pointers don't match, then don't backprop
		this->curr_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   state_vals,
									   curr_fold_history);
		history->curr_fold_history = curr_fold_history;
	} else {
		this->curr_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   state_vals);
	}
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* curr_end_fold_history = new FoldNetworkHistory(this->curr_end_fold);
		this->curr_end_fold->activate_fold(fold_input,
										   local_s_input_vals,
										   state_vals,
										   curr_end_fold_history);
		history->curr_end_fold_history = curr_end_fold_history;
	} else {
		this->curr_end_fold->activate_fold(fold_input,
										   local_s_input_vals,
										   state_vals);
	}
	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	// end_scale_mod passed on
}

void Fold::compress_step_explore_off_path_backprop(
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

	this->curr_end_fold->backprop_fold_errors_with_no_weight_change(
		local_state_errors,
		history->curr_end_fold_history);

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop_fold_errors_with_no_weight_change(
		curr_fold_error,
		history->curr_fold_history);

	vector<vector<double>> s_input_errors;
	s_input_errors.reserve(this->curr_s_input_sizes.size());
	s_input_errors.push_back(local_s_input_errors);
	for (int sc_index = 1; sc_index < (int)this->curr_s_input_sizes.size(); sc_index++) {
		s_input_errors.push_back(vector<double>(this->curr_s_input_sizes[sc_index], 0.0));
	}
	vector<vector<double>> state_errors;
	state_errors.reserve(this->curr_scope_sizes.size());
	for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
		state_errors.push_back(vector<double>(this->curr_scope_sizes[sc_index], 0.0));
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	for (int st_index = 0; st_index < (int)s_input_errors[0].size(); st_index++) {
		s_input_errors[0][st_index] += this->curr_end_fold->s_input_input->errors[st_index];
		this->curr_end_fold->s_input_input->errors[st_index] = 0.0;

		s_input_errors[0][st_index] += this->curr_fold->s_input_input->errors[st_index];
		this->curr_fold->s_input_input->errors[st_index] = 0.0;
	}
	for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
			state_errors[sc_index][st_index] += this->curr_end_fold->state_inputs[sc_index]->errors[st_index];
			this->curr_end_fold->state_inputs[sc_index]->errors[st_index] = 0.0;

			state_errors[sc_index][st_index] += this->curr_fold->state_inputs[sc_index]->errors[st_index];
			this->curr_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
		}
	}
	// don't include this->finished_steps.size()
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size()+1; f_index--) {
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

			this->curr_input_folds[f_index]->backprop_fold_errors_with_no_weight_change(
				scope_output_errors,
				history->curr_input_fold_histories[f_index]);
			for (int st_index = 0; st_index < (int)s_input_errors[0].size(); st_index++) {
				s_input_errors[0][st_index] += this->curr_input_folds[f_index]->s_input_input->errors[st_index];
				this->curr_input_folds[f_index]->s_input_input->errors[st_index] = 0.0;
			}
			for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->curr_input_folds[f_index]->state_inputs[sc_index]->errors[st_index];
					this->curr_input_folds[f_index]->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			// don't include this->finished_steps.size()
			for (int ff_index = f_index-1; ff_index >= (int)this->finished_steps.size()+1; ff_index--) {
				if (this->is_existing[ff_index]) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	if (this->curr_compress_num_layers > 0) {
		if (this->curr_compress_new_size > 0) {
			this->curr_compress_network->backprop_subfold_errors_with_no_weight_change(
				state_errors.back(),
				history->curr_compress_network_history);

			// don't pop last_s_input_errors
			state_errors.pop_back();
			state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[0], 0.0));
			for (int l_index = 1; l_index < this->curr_compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->curr_compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[l_index], 0.0));
			}

			for (int s_index = 0; s_index < (int)s_input_errors[0].size(); s_index++) {
				s_input_errors[0][s_index] += this->curr_compress_network->s_input_input->errors[s_index];
				this->curr_compress_network->s_input_input->errors[s_index] = 0.0;
			}
			for (int l_index = 0; l_index < (int)state_errors.size(); l_index++) {
				for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
					state_errors[l_index][s_index] += this->curr_compress_network->state_inputs[l_index]->errors[s_index];
					this->curr_compress_network->state_inputs[l_index]->errors[s_index] = 0.0;
				}
			}
		} else {
			if (state_errors.back().size() == 0) {
				// edge case where compressed down to 0
				// s_input_errors unchanged
				state_errors.back() = vector<double>(this->curr_compressed_scope_sizes[0], 0.0);
			} else {
				s_input_errors.push_back(vector<double>(this->curr_compressed_s_input_sizes[0]));
				state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[0]));
			}
			for (int l_index = 1; l_index < this->curr_compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->curr_compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[l_index], 0.0));
			}
		}
	}

	double inner_predicted_score_error = target_val - predicted_score;

	scale_factor_error += history->score_update*inner_predicted_score_error;

	vector<double> score_errors{scale_factor*inner_predicted_score_error};
	this->curr_score_network->backprop_subfold_errors_with_no_weight_change(
		score_errors,
		history->curr_score_network_history);
	for (int s_index = 0; s_index < (int)s_input_errors[0].size(); s_index++) {
		s_input_errors[0][s_index] += this->curr_score_network->s_input_input->errors[s_index];
		this->curr_score_network->s_input_input->errors[s_index] = 0.0;
	}
	for (int l_index = 0; l_index < (int)state_errors.size(); l_index++) {
		for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
			state_errors[l_index][s_index] += this->curr_score_network->state_inputs[l_index]->errors[s_index];
			this->curr_score_network->state_inputs[l_index]->errors[s_index] = 0.0;
		}
	}
	predicted_score -= scale_factor*history->score_update;

	if (!this->is_existing[this->finished_steps.size()]) {
		s_input_errors.pop_back();
		state_errors.pop_back();
	} else {
		vector<double> existing_scope_output_errors = s_input_errors.back();
		s_input_errors.pop_back();
		vector<double> scope_input_errors = state_errors.back();
		state_errors.pop_back();

		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->existing_actions[this->finished_steps.size()]->explore_off_path_backprop(
			scope_input_errors,
			scope_output_errors,
			predicted_score,
			target_val,
			scale_factor,
			scope_scale_factor_error,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor_error += scope_scale_mod*scope_scale_factor_error;

		scale_factor /= scope_scale_mod;

		for (int s_index = 0; s_index < this->existing_actions[this->finished_steps.size()]->num_inputs; s_index++) {
			scope_output_errors[s_index] += existing_scope_output_errors[s_index];
		}

		vector<double> curr_input_s_input_output_errors;
		vector<double> curr_input_state_output_errors;
		this->curr_input_network->backprop_small_errors_with_no_weight_change(
			scope_output_errors,
			curr_input_s_input_output_errors,
			curr_input_state_output_errors,
			history->curr_input_network_history);
		for (int s_index = 0; s_index < (int)s_input_errors.back().size(); s_index++) {
			s_input_errors.back()[s_index] += curr_input_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors.back().size(); s_index++) {
			state_errors.back()[s_index] += curr_input_state_output_errors[s_index];
		}

		for (int i_index = (int)this->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> inner_input_input_errors(this->inner_input_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				inner_input_input_errors[this->inner_input_input_sizes[i_index]-1-s_index] = s_input_errors[this->inner_input_input_layer[i_index]+1].back();
				s_input_errors[this->inner_input_input_layer[i_index]+1].pop_back();
			}
			vector<double> s_input_output_errors;
			vector<double> state_output_errors;
			this->inner_input_input_networks[i_index]->backprop_small_errors_with_no_weight_change(
				inner_input_input_errors,
				s_input_output_errors,
				state_output_errors,
				history->inner_input_input_network_histories[i_index]);
			for (int s_index = 0; s_index < (int)s_input_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				s_input_errors[this->inner_input_input_layer[i_index]][s_index] += s_input_output_errors[s_index];
			}
			for (int s_index = 0; s_index < (int)state_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				state_errors[this->inner_input_input_layer[i_index]][s_index] += state_output_errors[s_index];
			}
		}
	}

	for (int n_index = (int)this->finished_steps.size()-1; n_index >= 0; n_index--) {
		this->finished_steps[n_index]->explore_off_path_backprop(s_input_errors,
																 state_errors,
																 predicted_score,
																 target_val,
																 scale_factor,
																 scale_factor_error,
																 history->finished_step_histories[n_index]);
	}

	// s_input_errors and state_errors should now have size 1
	local_s_input_errors = s_input_errors[0];
	local_state_errors = state_errors[0];

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

void Fold::compress_step_existing_flat_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int n_index = 0; n_index < (int)this->finished_steps.size(); n_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[n_index]);
		this->finished_steps[n_index]->existing_flat_activate(flat_vals,
															  s_input_vals,
															  state_vals,
															  predicted_score,
															  scale_factor,
															  finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		fold_input.push_back(vector<double>());	// empty
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				input_fold_inputs[f_index].push_back(vector<double>());	// empty
			}
		}
	}

	if (!this->is_existing[this->finished_steps.size()]) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			FoldNetworkHistory* inner_input_input_network_history = new FoldNetworkHistory(this->inner_input_input_networks[i_index]);
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]],
																	  inner_input_input_network_history);
			history->inner_input_input_network_histories.push_back(inner_input_input_network_history);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		FoldNetworkHistory* curr_input_network_history = new FoldNetworkHistory(this->curr_input_network);
		this->curr_input_network->activate_small(s_input_vals.back(),
												 state_vals.back(),
												 curr_input_network_history);
		history->curr_input_network_history = curr_input_network_history;
		vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
		for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
			scope_input[i_index] = this->curr_input_network->output->acti_vals[i_index];
		}

		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
		this->existing_actions[this->finished_steps.size()]->existing_flat_activate(
			flat_vals,
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
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(vector<double>());	// empty
		}
	}

	FoldNetworkHistory* curr_score_network_history = new FoldNetworkHistory(this->curr_score_network);
	this->curr_score_network->activate_subfold(local_s_input_vals,
											   state_vals,
											   curr_score_network_history);
	history->curr_score_network_history = curr_score_network_history;
	history->score_update = this->curr_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_score_network->output->acti_vals[0];

	if (this->curr_compress_num_layers > 0) {
		if (this->curr_compress_new_size > 0) {
			FoldNetworkHistory* curr_compress_network_history = new FoldNetworkHistory(this->curr_compress_network);
			this->curr_compress_network->activate_subfold(local_s_input_vals,
														  state_vals,
														  curr_compress_network_history);
			history->curr_compress_network_history = curr_compress_network_history;
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals (though doesn't matter here)
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->curr_compress_new_size));
			for (int s_index = 0; s_index < this->curr_compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->curr_compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			if (s_input_vals.size() == 1) {
				// edge case where set last layer to 0 instead of removing
				state_vals[0].clear();
			} else {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
			this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
														   local_s_input_vals,
														   state_vals,
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
	// TODO: if pointers don't match, then don't backprop
	this->curr_fold->activate_fold(fold_input,
								   local_s_input_vals,
								   state_vals,
								   curr_fold_history);
	history->curr_fold_history = curr_fold_history;
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	FoldNetworkHistory* curr_end_fold_history = new FoldNetworkHistory(this->curr_end_fold);
	this->curr_end_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   state_vals,
									   curr_end_fold_history);
	history->curr_end_fold_history = curr_end_fold_history;
	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	// end_scale_mod passed on
}

void Fold::compress_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	// end_scale_mod passed on

	scale_factor_error += history->ending_score_update*predicted_score_error;

	this->curr_end_fold->backprop_fold_errors_with_no_weight_change(
		local_state_errors,
		history->curr_end_fold_history);

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop_fold_errors_with_no_weight_change(
		curr_fold_error,
		history->curr_fold_history);

	vector<vector<double>> s_input_errors;
	s_input_errors.reserve(this->curr_s_input_sizes.size());
	s_input_errors.push_back(local_s_input_errors);
	for (int sc_index = 1; sc_index < (int)this->curr_s_input_sizes.size(); sc_index++) {
		s_input_errors.push_back(vector<double>(this->curr_s_input_sizes[sc_index], 0.0));
	}
	vector<vector<double>> state_errors;
	state_errors.reserve(this->curr_scope_sizes.size());
	for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
		state_errors.push_back(vector<double>(this->curr_scope_sizes[sc_index], 0.0));
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	for (int st_index = 0; st_index < (int)s_input_errors[0].size(); st_index++) {
		s_input_errors[0][st_index] += this->curr_end_fold->s_input_input->errors[st_index];
		this->curr_end_fold->s_input_input->errors[st_index] = 0.0;

		s_input_errors[0][st_index] += this->curr_fold->s_input_input->errors[st_index];
		this->curr_fold->s_input_input->errors[st_index] = 0.0;
	}
	for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
			state_errors[sc_index][st_index] += this->curr_end_fold->state_inputs[sc_index]->errors[st_index];
			this->curr_end_fold->state_inputs[sc_index]->errors[st_index] = 0.0;

			state_errors[sc_index][st_index] += this->curr_fold->state_inputs[sc_index]->errors[st_index];
			this->curr_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
		}
	}
	// don't include this->finished_steps.size()
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size()+1; f_index--) {
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

			this->curr_input_folds[f_index]->backprop_fold_errors_with_no_weight_change(
				scope_output_errors,
				history->curr_input_fold_histories[f_index]);
			for (int st_index = 0; st_index < (int)s_input_errors[0].size(); st_index++) {
				s_input_errors[0][st_index] += this->curr_input_folds[f_index]->s_input_input->errors[st_index];
				this->curr_input_folds[f_index]->s_input_input->errors[st_index] = 0.0;
			}
			for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] += this->curr_input_folds[f_index]->state_inputs[sc_index]->errors[st_index];
					this->curr_input_folds[f_index]->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			// don't include this->finished_steps.size()
			for (int ff_index = f_index-1; ff_index >= (int)this->finished_steps.size()+1; ff_index--) {
				if (this->is_existing[ff_index]) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	if (this->curr_compress_num_layers > 0) {
		if (this->curr_compress_new_size > 0) {
			this->curr_compress_network->backprop_subfold_errors_with_no_weight_change(
				state_errors.back(),
				history->curr_compress_network_history);

			// don't pop last_s_input_errors
			state_errors.pop_back();
			state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[0], 0.0));
			for (int l_index = 1; l_index < this->curr_compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->curr_compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[l_index], 0.0));
			}

			for (int s_index = 0; s_index < (int)s_input_errors[0].size(); s_index++) {
				s_input_errors[0][s_index] += this->curr_compress_network->s_input_input->errors[s_index];
				this->curr_compress_network->s_input_input->errors[s_index] = 0.0;
			}
			for (int l_index = 0; l_index < (int)state_errors.size(); l_index++) {
				for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
					state_errors[l_index][s_index] += this->curr_compress_network->state_inputs[l_index]->errors[s_index];
					this->curr_compress_network->state_inputs[l_index]->errors[s_index] = 0.0;
				}
			}
		} else {
			if (state_errors.back().size() == 0) {
				// edge case where compressed down to 0
				// s_input_errors unchanged
				state_errors.back() = vector<double>(this->curr_compressed_scope_sizes[0], 0.0);
			} else {
				s_input_errors.push_back(vector<double>(this->curr_compressed_s_input_sizes[0]));
				state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[0]));
			}
			for (int l_index = 1; l_index < this->curr_compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->curr_compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->curr_compressed_scope_sizes[l_index], 0.0));
			}
		}
	}

	scale_factor_error += history->score_update*predicted_score_error;

	vector<double> score_errors{scale_factor*predicted_score_error};
	this->curr_score_network->backprop_subfold_errors_with_no_weight_change(
		score_errors,
		history->curr_score_network_history);
	for (int s_index = 0; s_index < (int)s_input_errors[0].size(); s_index++) {
		s_input_errors[0][s_index] += this->curr_score_network->s_input_input->errors[s_index];
		this->curr_score_network->s_input_input->errors[s_index] = 0.0;
	}
	for (int l_index = 0; l_index < (int)state_errors.size(); l_index++) {
		for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
			state_errors[l_index][s_index] += this->curr_score_network->state_inputs[l_index]->errors[s_index];
			this->curr_score_network->state_inputs[l_index]->errors[s_index] = 0.0;
		}
	}
	predicted_score -= scale_factor*history->score_update;

	if (!this->is_existing[this->finished_steps.size()]) {
		s_input_errors.pop_back();
		state_errors.pop_back();
	} else {
		vector<double> existing_scope_output_errors = s_input_errors.back();
		s_input_errors.pop_back();
		vector<double> scope_input_errors = state_errors.back();
		state_errors.pop_back();

		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->existing_actions[this->finished_steps.size()]->existing_flat_backprop(
			scope_input_errors,
			scope_output_errors,
			predicted_score,
			predicted_score_error,
			scale_factor,
			scope_scale_factor_error,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor_error += scope_scale_mod*scope_scale_factor_error;

		scale_factor /= scope_scale_mod;

		for (int s_index = 0; s_index < this->existing_actions[this->finished_steps.size()]->num_inputs; s_index++) {
			scope_output_errors[s_index] += existing_scope_output_errors[s_index];
		}

		vector<double> curr_input_s_input_output_errors;
		vector<double> curr_input_state_output_errors;
		this->curr_input_network->backprop_small_errors_with_no_weight_change(
			scope_output_errors,
			curr_input_s_input_output_errors,
			curr_input_state_output_errors,
			history->curr_input_network_history);
		for (int s_index = 0; s_index < (int)s_input_errors.back().size(); s_index++) {
			s_input_errors.back()[s_index] += curr_input_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors.back().size(); s_index++) {
			state_errors.back()[s_index] += curr_input_state_output_errors[s_index];
		}

		for (int i_index = (int)this->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
			vector<double> inner_input_input_errors(this->inner_input_input_sizes[i_index]);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				inner_input_input_errors[this->inner_input_input_sizes[i_index]-1-s_index] = s_input_errors[this->inner_input_input_layer[i_index]+1].back();
				s_input_errors[this->inner_input_input_layer[i_index]+1].pop_back();
			}
			vector<double> s_input_output_errors;
			vector<double> state_output_errors;
			this->inner_input_input_networks[i_index]->backprop_small_errors_with_no_weight_change(
				inner_input_input_errors,
				s_input_output_errors,
				state_output_errors,
				history->inner_input_input_network_histories[i_index]);
			for (int s_index = 0; s_index < (int)s_input_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				s_input_errors[this->inner_input_input_layer[i_index]][s_index] += s_input_output_errors[s_index];
			}
			for (int s_index = 0; s_index < (int)state_errors[this->inner_input_input_layer[i_index]].size(); s_index++) {
				state_errors[this->inner_input_input_layer[i_index]][s_index] += state_output_errors[s_index];
			}
		}
	}

	for (int n_index = (int)this->finished_steps.size()-1; n_index >= 0; n_index--) {
		this->finished_steps[n_index]->existing_flat_backprop(s_input_errors,
															  state_errors,
															  predicted_score,
															  predicted_score_error,
															  scale_factor,
															  scale_factor_error,
															  history->finished_step_histories[n_index]);
	}

	// s_input_errors and state_errors should now have size 1
	local_s_input_errors = s_input_errors[0];
	local_state_errors = state_errors[0];

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

void Fold::compress_step_update_activate(
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
		history->finished_step_histories.push_back(finished_step_history);

		fold_input.push_back(vector<double>());	// empty
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				input_fold_inputs[f_index].push_back(vector<double>());	// empty
			}
		}
	}

	if (!this->is_existing[this->finished_steps.size()]) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
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

		this->curr_input_network->activate_small(s_input_vals.back(),
												 state_vals.back());
		vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
		for (int s_index = 0; s_index < this->existing_actions[this->finished_steps.size()]->num_inputs; s_index++) {
			scope_input[s_index] = this->curr_input_network->output->acti_vals[s_index];
		}

		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
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
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(vector<double>());	// empty
		}
	}

	FoldNetworkHistory* curr_score_network_history = new FoldNetworkHistory(this->curr_score_network);
	this->curr_score_network->activate_subfold(local_s_input_vals,
											   state_vals,
											   curr_score_network_history);
	history->curr_score_network_history = curr_score_network_history;
	history->score_update = this->curr_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_score_network->output->acti_vals[0];

	// this->test_compress_num_layers > 0
	
	// vector<vector<double>> test_s_input_vals = s_input_vals;
	vector<vector<double>> test_state_vals = state_vals;
	vector<double> test_state_errors(this->test_compress_new_size, 0.0);

	if (this->test_compress_new_size > 0) {
		this->test_compress_network->activate_subfold(local_s_input_vals,
													  state_vals);
		for (int l_index = 0; l_index < this->test_compress_num_layers-1; l_index++) {
			// test_s_input_vals.pop_back();
			test_state_vals.pop_back();
		}
		// don't pop last s_input_vals (though doesn't matter here)
		test_state_vals.pop_back();
		test_state_vals.push_back(vector<double>(this->test_compress_new_size));
		for (int s_index = 0; s_index < this->test_compress_new_size; s_index++) {
			test_state_vals.back()[s_index] = this->new_state_factor*this->test_compress_network->output->acti_vals[s_index];
		}
	} else {
		for (int l_index = 0; l_index < this->test_compress_num_layers-1; l_index++) {
			// test_s_input_vals.pop_back();
			test_state_vals.pop_back();
		}
		// if (test_s_input_vals.size() == 1) {
		if (test_state_vals.size() == 1) {
			// edge case where set last layer to 0 instead of removing
			test_state_vals[0].clear();
		} else {
			// test_s_input_vals.pop_back();
			test_state_vals.pop_back();
		}
	}

	if (this->curr_compress_num_layers > 0) {
		if (this->curr_compress_new_size > 0) {
			this->curr_compress_network->activate_subfold(local_s_input_vals,
														  state_vals);
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals (though doesn't matter here)
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->curr_compress_new_size));
			for (int s_index = 0; s_index < this->curr_compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->curr_compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			if (s_input_vals.size() == 1) {
				// edge case where set last layer to 0 instead of removing
				state_vals[0].clear();
			} else {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			fold_input.push_back(*flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(*flat_vals.begin());
				}
			}

			flat_vals.erase(flat_vals.begin());
		} else {
			this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
														   local_s_input_vals,
														   state_vals);

			this->test_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
														   local_s_input_vals,
														   test_state_vals);

			vector<double> input_fold_errors(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				input_fold_errors[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index]
					- this->test_input_folds[f_index]->output->acti_vals[i_index];
				this->sum_error += input_fold_errors[i_index]*input_fold_errors[i_index];
			}
			if (this->test_compress_new_size > 0) {
				if (this->state_iter <= 130000) {
					this->test_input_folds[f_index]->backprop_fold_last_state(input_fold_errors, 0.01);
				} else {
					this->test_input_folds[f_index]->backprop_fold_last_state(input_fold_errors, 0.002);
				}
				for (int s_index = 0; s_index < this->test_compress_new_size; s_index++) {
					test_state_errors[s_index] += this->test_input_folds[f_index]->state_inputs.back()->errors[s_index];
					this->test_input_folds[f_index]->state_inputs.back()->errors[s_index] = 0.0;
				}
			} else {
				if (this->state_iter <= 130000) {
					this->test_input_folds[f_index]->backprop_fold_weights_with_no_error_signal(input_fold_errors, 0.01);
				} else {
					this->test_input_folds[f_index]->backprop_fold_weights_with_no_error_signal(input_fold_errors, 0.002);
				}
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
	this->curr_fold->activate_fold(fold_input,
								   local_s_input_vals,
								   state_vals,
								   curr_fold_history);
	history->curr_fold_history = curr_fold_history;
	history->ending_score_update = this->curr_fold->output->acti_vals[0];

	this->test_fold->activate_fold(fold_input,
								   local_s_input_vals,
								   test_state_vals);

	double fold_error = this->curr_fold->output->acti_vals[0] - this->test_fold->output->acti_vals[0];
	this->sum_error += fold_error*fold_error;
	vector<double> fold_errors{fold_error};
	if (this->test_compress_new_size > 0) {
		if (this->state_iter <= 130000) {
			this->test_fold->backprop_fold_last_state(fold_errors, 0.01);
		} else {
			this->test_fold->backprop_fold_last_state(fold_errors, 0.002);
		}
		for (int s_index = 0; s_index < this->test_compress_new_size; s_index++) {
			test_state_errors[s_index] += this->test_fold->state_inputs.back()->errors[s_index];
			this->test_fold->state_inputs.back()->errors[s_index] = 0.0;
		}
	} else {
		if (this->state_iter <= 130000) {
			this->test_fold->backprop_fold_weights_with_no_error_signal(fold_errors, 0.01);
		} else {
			this->test_fold->backprop_fold_weights_with_no_error_signal(fold_errors, 0.002);
		}
	}

	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	this->curr_end_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   state_vals);
	
	this->test_end_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   test_state_vals);

	vector<double> end_fold_errors(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		end_fold_errors[o_index] = this->curr_end_fold->output->acti_vals[o_index]
			- this->test_end_fold->output->acti_vals[o_index];
		this->sum_error += end_fold_errors[o_index]*end_fold_errors[o_index];
	}
	if (this->test_compress_new_size > 0) {
		if (this->state_iter <= 130000) {
			this->test_end_fold->backprop_fold_last_state(end_fold_errors, 0.01);
		} else {
			this->test_end_fold->backprop_fold_last_state(end_fold_errors, 0.002);
		}
		for (int s_index = 0; s_index < this->test_compress_new_size; s_index++) {
			test_state_errors[s_index] += this->test_end_fold->state_inputs.back()->errors[s_index];
			this->test_end_fold->state_inputs.back()->errors[s_index] = 0.0;
		}
	} else {
		if (this->state_iter <= 130000) {
			this->test_end_fold->backprop_fold_weights_with_no_error_signal(end_fold_errors, 0.01);
		} else {
			this->test_end_fold->backprop_fold_weights_with_no_error_signal(end_fold_errors, 0.002);
		}
	}

	local_state_vals.clear();
	local_state_vals.reserve(this->output_size);
	for (int o_index = 0; o_index < this->output_size; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}

	if (this->test_compress_new_size > 0) {
		if (this->state_iter <= 110000) {
			this->test_compress_network->backprop_subfold_weights_with_no_error_signal(
				test_state_errors,
				0.05);
		} else if (this->state_iter <= 130000) {
			this->test_compress_network->backprop_subfold_weights_with_no_error_signal(
				test_state_errors,
				0.01);
		} else {
			this->test_compress_network->backprop_subfold_weights_with_no_error_signal(
				test_state_errors,
				0.002);
		}
	}

	// end_scale_mod passed on
}

void Fold::compress_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {
	// end_scale_mod passed on

	double predicted_score_error = target_val - predicted_score;

	vector<double> curr_fold_error{scale_factor*predicted_score_error};
	this->curr_fold->backprop_fold_weights_with_no_error_signal(
		curr_fold_error,
		0.001,
		history->curr_fold_history);

	next_predicted_score = predicted_score;
	predicted_score -= scale_factor*history->ending_score_update;

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size()+1; f_index--) {
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

	double inner_predicted_score_error = target_val - predicted_score;

	vector<double> score_errors{scale_factor*inner_predicted_score_error};
	this->curr_score_network->backprop_subfold_weights_with_no_error_signal(
		score_errors,
		0.001,
		history->curr_score_network_history);
	next_predicted_score = predicted_score;
	predicted_score -= scale_factor*history->score_update;

	if (!this->is_existing[this->finished_steps.size()]) {
		// do nothing
	} else {
		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		this->existing_actions[this->finished_steps.size()]->update_backprop(
			predicted_score,
			next_predicted_score,
			target_val,
			scale_factor,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor /= scope_scale_mod;
	}

	for (int n_index = (int)this->finished_steps.size()-1; n_index >= 0; n_index--) {
		this->finished_steps[n_index]->update_backprop(predicted_score,
													   next_predicted_score,
													   target_val,
													   scale_factor,
													   history->finished_step_histories[n_index]);
	}

	double misguess = abs(target_val - predicted_score);
	this->starting_average_misguess = 0.999*this->starting_average_misguess + 0.001*misguess;

	next_predicted_score = predicted_score;
	predicted_score -= history->starting_score_update;	// already scaled

	this->starting_average_local_impact = 0.999*this->starting_average_local_impact
		+ 0.001*abs(history->starting_score_update);
}

void Fold::compress_step_existing_update_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int n_index = 0; n_index < (int)this->finished_steps.size(); n_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[n_index]);
		this->finished_steps[n_index]->existing_update_activate(flat_vals,
																s_input_vals,
																state_vals,
																predicted_score,
																scale_factor,
																finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		fold_input.push_back(vector<double>());	// empty
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				input_fold_inputs[f_index].push_back(vector<double>());	// empty
			}
		}
	}

	if (!this->is_existing[this->finished_steps.size()]) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(*flat_vals.begin());
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

		this->curr_input_network->activate_small(s_input_vals.back(),
												 state_vals.back());
		vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
		for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
			scope_input[i_index] = this->curr_input_network->output->acti_vals[i_index];
		}

		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
		this->existing_actions[this->finished_steps.size()]->existing_update_activate(
			flat_vals,
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
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(vector<double>());	// empty
		}
	}

	this->curr_score_network->activate_subfold(local_s_input_vals,
											   state_vals);
	history->score_update = this->curr_score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_score_network->output->acti_vals[0];

	if (this->curr_compress_num_layers > 0) {
		if (this->curr_compress_new_size > 0) {
			this->curr_compress_network->activate_subfold(local_s_input_vals,
														  state_vals);
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals (though doesn't matter here)
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->curr_compress_new_size));
			for (int s_index = 0; s_index < this->curr_compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->curr_compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->curr_compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			if (s_input_vals.size() == 1) {
				// edge case where set last layer to 0 instead of removing
				state_vals[0].clear();
			} else {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			fold_input.push_back(*flat_vals.begin());
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(*flat_vals.begin());
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
				if (this->is_existing[i_index]) {
					input_fold_inputs[i_index].push_back(scope_output);
				}
			}
		}
	}

	this->curr_fold->activate_fold(fold_input,
								   local_s_input_vals,
								   state_vals);
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

	// end_scale_mod passed on
}

void Fold::compress_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	// end_scale_mod passed on

	scale_factor_error += history->ending_score_update*predicted_score_error;

	predicted_score -= scale_factor*history->ending_score_update;

	for (int f_index = this->sequence_length-1; f_index >= (int)this->finished_steps.size()+1; f_index--) {
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

	scale_factor_error += history->score_update*predicted_score_error;

	predicted_score -= scale_factor*history->score_update;

	if (!this->is_existing[this->finished_steps.size()]) {
		// do nothing
	} else {
		double scope_scale_mod = this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod;

		double scope_scale_factor_error = 0.0;
		this->existing_actions[this->finished_steps.size()]->existing_update_backprop(
			predicted_score,
			predicted_score_error,
			scale_factor,
			scope_scale_factor_error,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor_error += scope_scale_mod*scope_scale_factor_error;

		scale_factor /= scope_scale_mod;
	}

	for (int n_index = (int)this->finished_steps.size()-1; n_index >= 0; n_index--) {
		this->finished_steps[n_index]->existing_update_backprop(predicted_score,
																predicted_score_error,
																scale_factor,
																scale_factor_error,
																history->finished_step_histories[n_index]);
	}

	predicted_score -= history->starting_score_update;	// already scaled
}
