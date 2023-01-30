#include "fold.h"

#include <cmath>
#include <iostream>

#include "constants.h"

using namespace std;

void Fold::inner_scope_input_step_explore_off_path_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[f_index]);
		this->finished_steps[f_index]->explore_off_path_activate(problem,
																 s_input_vals,
																 state_vals,
																 predicted_score,
																 scale_factor,
																 run_status,
																 finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		if (run_status.exceeded_depth) {
			history->exit_index = f_index;
			history->exit_location = EXIT_LOCATION_SPOT;
			return;
		}

		fold_input.push_back(vector<double>());	// empty
		for (int ff_index = (int)this->finished_steps.size()+1; ff_index < this->sequence_length; ff_index++) {
			if (this->is_existing[ff_index]) {
				input_fold_inputs[ff_index].push_back(vector<double>());	// empty
			}
		}
	}

	// this->is_existing[this->finished_steps.size()] == true
	for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
		if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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

	if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* curr_inner_input_network_history = new FoldNetworkHistory(this->curr_inner_input_network);
		this->curr_inner_input_network->activate_subfold(s_input_vals[this->curr_inner_input_network->subfold_index+1],
												   state_vals,
												   curr_inner_input_network_history);
		history->curr_inner_input_network_history = curr_inner_input_network_history;
	} else {
		this->curr_inner_input_network->activate_subfold(s_input_vals[this->curr_inner_input_network->subfold_index+1],
												   state_vals);
	}
	vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		scope_input[i_index] = this->curr_inner_input_network->output->acti_vals[i_index];
	}

	double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
	scale_factor *= scope_scale_mod_val;

	vector<double> scope_output;
	ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
	this->existing_actions[this->finished_steps.size()]->explore_off_path_activate(
		problem,
		scope_input,
		scope_output,
		predicted_score,
		scale_factor,
		run_status,
		scope_history);
	history->scope_histories[this->finished_steps.size()] = scope_history;

	scale_factor /= scope_scale_mod_val;

	if (run_status.exceeded_depth) {
		history->exit_index = (int)this->finished_steps.size();
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	}

	// not folded yet
	fold_input.push_back(scope_output);
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(scope_output);
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (!this->is_existing[f_index]) {
			problem.perform_action(this->actions[f_index]);

			vector<double> new_obs{problem.get_observation()};

			fold_input.push_back(new_obs);
			for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
				if (this->existing_actions[ff_index] != NULL) {
					input_fold_inputs[ff_index].push_back(new_obs);
				}
			}
		} else {
			if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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

	if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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

	if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::inner_scope_input_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {
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
	// include this->finished_steps.size()
	for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	double predicted_score_error = target_val - predicted_score;

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		this->curr_end_fold->backprop_fold_errors_with_no_weight_change(
			local_state_errors,
			history->curr_end_fold_history);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop_fold_errors_with_no_weight_change(
			curr_fold_error,
			history->curr_fold_history);

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
		// include this->finished_steps.size()
		for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
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

	for (int f_index = history->exit_index; f_index >= (int)this->finished_steps.size()+1; f_index--) {
		if (this->is_existing[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;	// unused
			this->existing_actions[f_index]->existing_flat_backprop(scope_input_errors[f_index],
																	scope_output_errors,
																	predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scope_scale_factor_error,
																	history->scope_histories[f_index]);

			scale_factor /= scope_scale_mod_val;

			// don't update scope_scale_mod on explore_off_path

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
			// include this->finished_steps.size()
			for (int ff_index = f_index-1; ff_index >= (int)this->finished_steps.size(); ff_index--) {
				if (this->is_existing[ff_index]) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	if (history->exit_index < (int)this->finished_steps.size()) {
		for (int i_index = (int)this->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_errors[this->inner_input_input_layer[i_index]+1].pop_back();
			}
		}
	} else {
		// this->is_existing[this->finished_steps.size()] == true
		double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		vector<double> scope_output_errors;
		this->existing_actions[this->finished_steps.size()]->explore_off_path_backprop(
			scope_input_errors[this->finished_steps.size()],
			scope_output_errors,
			predicted_score,
			target_val,
			scale_factor,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor /= scope_scale_mod_val;

		this->curr_inner_input_network->backprop_subfold_errors_with_no_weight_change(
			scope_output_errors,
			history->curr_inner_input_network_history);
		for (int st_index = 0; st_index < (int)s_input_errors[this->curr_inner_input_network->subfold_index+1].size(); st_index++) {
			s_input_errors[this->curr_inner_input_network->subfold_index+1][st_index] += this->curr_inner_input_network->s_input_input->errors[st_index];
			this->curr_inner_input_network->s_input_input->errors[st_index] = 0.0;
		}
		for (int sc_index = this->curr_inner_input_network->subfold_index+1; sc_index < (int)state_errors.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
				state_errors[sc_index][st_index] += this->curr_inner_input_network->state_inputs[sc_index]->errors[st_index];
				this->curr_inner_input_network->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
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

	for (int f_index = (int)this->finished_steps.size()-1; f_index >= 0; f_index--) {
		if (history->exit_index < f_index) {
			if (this->finished_steps[f_index]->compress_num_layers > 0) {
				if (this->finished_steps[f_index]->active_compress) {
					// don't pop last_s_input_errors
					state_errors.pop_back();
					state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[0], 0.0));
					for (int l_index = 1; l_index < this->finished_steps[f_index]->compress_num_layers; l_index++) {
						s_input_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_s_input_sizes[l_index], 0.0));
						state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[l_index], 0.0));
					}
				} else {
					if (state_errors.back().size() == 0) {
						// edge case where compressed down to 0
						// s_input_errors unchanged
						state_errors.back() = vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[0], 0.0);
					} else {
						s_input_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_s_input_sizes[0]));
						state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[0]));
					}
					for (int l_index = 1; l_index < this->finished_steps[f_index]->compress_num_layers; l_index++) {
						s_input_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_s_input_sizes[l_index], 0.0));
						state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[l_index], 0.0));
					}
				}
			}

			for (int i_index = (int)this->finished_steps[f_index]->input_networks.size()-1; i_index >= 0; i_index--) {
				for (int s_index = 0; s_index < this->finished_steps[f_index]->input_sizes[i_index]; s_index++) {
					s_input_errors[this->finished_steps[f_index]->input_layer[i_index]+1].pop_back();
				}
			}

			s_input_errors.pop_back();
			state_errors.pop_back();

			for (int i_index = (int)this->finished_steps[f_index]->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
				for (int s_index = 0; s_index < this->finished_steps[f_index]->inner_input_input_sizes[i_index]; s_index++) {
					s_input_errors[this->finished_steps[f_index]->inner_input_input_layer[i_index]+1].pop_back();
				}
			}
		} else {
			this->finished_steps[f_index]->explore_off_path_backprop(s_input_errors,
																	 state_errors,
																	 predicted_score,
																	 target_val,
																	 scale_factor,
																	 history->finished_step_histories[f_index]);
		}
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

void Fold::inner_scope_input_step_existing_flat_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[f_index]);
		this->finished_steps[f_index]->existing_flat_activate(problem,
															  s_input_vals,
															  state_vals,
															  predicted_score,
															  scale_factor,
															  run_status,
															  finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		if (run_status.exceeded_depth) {
			history->exit_index = f_index;
			history->exit_location = EXIT_LOCATION_SPOT;
			return;
		}

		fold_input.push_back(vector<double>());	// empty
		for (int ff_index = (int)this->finished_steps.size()+1; ff_index < this->sequence_length; ff_index++) {
			if (this->is_existing[ff_index]) {
				input_fold_inputs[ff_index].push_back(vector<double>());	// empty
			}
		}
	}

	// this->existing_actions[this->finished_steps.size()] != NULL
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

	FoldNetworkHistory* curr_inner_input_network_history = new FoldNetworkHistory(this->curr_inner_input_network);
	this->curr_inner_input_network->activate_subfold(s_input_vals[this->curr_inner_input_network->subfold_index+1],
											   state_vals,
											   curr_inner_input_network_history);
	history->curr_inner_input_network_history = curr_inner_input_network_history;
	vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		scope_input[i_index] = this->curr_inner_input_network->output->acti_vals[i_index];
	}

	double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
	scale_factor *= scope_scale_mod_val;

	vector<double> scope_output;
	ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
	this->existing_actions[this->finished_steps.size()]->existing_flat_activate(
		problem,
		scope_input,
		scope_output,
		predicted_score,
		scale_factor,
		run_status,
		scope_history);
	history->scope_histories[this->finished_steps.size()] = scope_history;

	scale_factor /= scope_scale_mod_val;

	if (run_status.exceeded_depth) {
		history->exit_index = (int)this->finished_steps.size();
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	}

	// not folded yet
	fold_input.push_back(scope_output);
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(scope_output);
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::inner_scope_input_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
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
	// include this->finished_steps.size()
	for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			scope_input_errors[f_index] = vector<double>(this->existing_actions[f_index]->num_outputs, 0.0);
		}
	}

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		this->curr_end_fold->backprop_fold_errors_with_no_weight_change(
			local_state_errors,
			history->curr_end_fold_history);

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop_fold_errors_with_no_weight_change(
			curr_fold_error,
			history->curr_fold_history);

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
		// include this->finished_steps.size()
		for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
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

	for (int f_index = history->exit_index; f_index >= (int)this->finished_steps.size()+1; f_index--) {
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

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

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
			// include this->finished_steps.size()
			for (int ff_index = f_index-1; ff_index >= (int)this->finished_steps.size(); ff_index--) {
				if (this->is_existing[ff_index]) {
					for (int i_index = 0; i_index < this->existing_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] += this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
				}
			}
		}
	}

	if (history->exit_index < (int)this->finished_steps.size()) {
		for (int i_index = (int)this->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_errors[this->inner_input_input_layer[i_index]+1].pop_back();
			}
		}
	} else {
		// this->is_existing[this->finished_steps.size()] == true
		double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->existing_actions[this->finished_steps.size()]->existing_flat_backprop(
			scope_input_errors[this->finished_steps.size()],
			scope_output_errors,
			predicted_score,
			predicted_score_error,
			scale_factor,
			scope_scale_factor_error,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

		scale_factor /= scope_scale_mod_val;

		this->curr_inner_input_network->backprop_subfold_errors_with_no_weight_change(
			scope_output_errors,
			history->curr_inner_input_network_history);
		for (int st_index = 0; st_index < (int)s_input_errors[this->curr_inner_input_network->subfold_index+1].size(); st_index++) {
			s_input_errors[this->curr_inner_input_network->subfold_index+1][st_index] += this->curr_inner_input_network->s_input_input->errors[st_index];
			this->curr_inner_input_network->s_input_input->errors[st_index] = 0.0;
		}
		for (int sc_index = this->curr_inner_input_network->subfold_index+1; sc_index < (int)state_errors.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
				state_errors[sc_index][st_index] += this->curr_inner_input_network->state_inputs[sc_index]->errors[st_index];
				this->curr_inner_input_network->state_inputs[sc_index]->errors[st_index] = 0.0;
			}
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

	for (int f_index = (int)this->finished_steps.size()-1; f_index >= 0; f_index--) {
		if (history->exit_index < f_index) {
			if (this->finished_steps[f_index]->compress_num_layers > 0) {
				if (this->finished_steps[f_index]->active_compress) {
					// don't pop last_s_input_errors
					state_errors.pop_back();
					state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[0], 0.0));
					for (int l_index = 1; l_index < this->finished_steps[f_index]->compress_num_layers; l_index++) {
						s_input_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_s_input_sizes[l_index], 0.0));
						state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[l_index], 0.0));
					}
				} else {
					if (state_errors.back().size() == 0) {
						// edge case where compressed down to 0
						// s_input_errors unchanged
						state_errors.back() = vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[0], 0.0);
					} else {
						s_input_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_s_input_sizes[0]));
						state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[0]));
					}
					for (int l_index = 1; l_index < this->finished_steps[f_index]->compress_num_layers; l_index++) {
						s_input_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_s_input_sizes[l_index], 0.0));
						state_errors.push_back(vector<double>(this->finished_steps[f_index]->compressed_scope_sizes[l_index], 0.0));
					}
				}
			}

			for (int i_index = (int)this->finished_steps[f_index]->input_networks.size()-1; i_index >= 0; i_index--) {
				for (int s_index = 0; s_index < this->finished_steps[f_index]->input_sizes[i_index]; s_index++) {
					s_input_errors[this->finished_steps[f_index]->input_layer[i_index]+1].pop_back();
				}
			}

			s_input_errors.pop_back();
			state_errors.pop_back();

			for (int i_index = (int)this->finished_steps[f_index]->inner_input_input_networks.size()-1; i_index >= 0; i_index--) {
				for (int s_index = 0; s_index < this->finished_steps[f_index]->inner_input_input_sizes[i_index]; s_index++) {
					s_input_errors[this->finished_steps[f_index]->inner_input_input_layer[i_index]+1].pop_back();
				}
			}
		} else {
			this->finished_steps[f_index]->existing_flat_backprop(s_input_errors,
																  state_errors,
																  predicted_score,
																  predicted_score_error,
																  scale_factor,
																  scale_factor_error,
																  history->finished_step_histories[f_index]);
		}
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

void Fold::inner_scope_input_step_update_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[f_index]);
		this->finished_steps[f_index]->update_activate(problem,
													   s_input_vals,
													   state_vals,
													   predicted_score,
													   scale_factor,
													   run_status,
													   finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		if (run_status.exceeded_depth) {
			history->exit_index = f_index;
			history->exit_location = EXIT_LOCATION_SPOT;
			// fold_increment() even if test networks not hit
			return;
		}

		fold_input.push_back(vector<double>());	// empty
		for (int ff_index = (int)this->finished_steps.size()+1; ff_index < this->sequence_length; ff_index++) {
			if (this->is_existing[ff_index]) {
				input_fold_inputs[ff_index].push_back(vector<double>());	// empty
			}
		}
	}

	// this->is_existing[this->finished_steps.size()] == true
	// don't need different inner_input_input_networks as curr_inner_input_network and test_inner_input_network share (but use different layers)
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

	this->curr_inner_input_network->activate_subfold(s_input_vals[this->curr_inner_input_network->subfold_index+1],
											   state_vals);

	this->test_inner_input_network->activate_subfold(s_input_vals[this->test_inner_input_network->subfold_index+1],
											   state_vals);

	vector<double> inner_input_errors(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		inner_input_errors[i_index] = this->curr_inner_input_network->output->acti_vals[i_index]
			- this->test_inner_input_network->output->acti_vals[i_index];
		this->sum_error += inner_input_errors[i_index]*inner_input_errors[i_index];
	}
	if (this->inner_input_input_networks.size() == 0) {
		if (this->state_iter <= 130000) {
			this->test_inner_input_network->backprop_subfold_weights_with_no_error_signal(
				inner_input_errors,
				0.01);
		} else {
			this->test_inner_input_network->backprop_subfold_weights_with_no_error_signal(
				inner_input_errors,
				0.002);
		}
	} else {
		if (this->state_iter <= 130000) {
			this->test_inner_input_network->backprop_subfold_new_s_input(
				inner_input_errors,
				0.01);
		} else {
			this->test_inner_input_network->backprop_subfold_new_s_input(
				inner_input_errors,
				0.002);
		}
		vector<double> inner_input_input_errors(this->inner_input_input_sizes.back());
		int initial_size = (int)this->test_inner_input_network->s_input_input->errors.size() - this->inner_input_input_sizes.back();
		for (int s_index = 0; s_index < this->inner_input_input_sizes.back(); s_index++) {
			inner_input_input_errors[s_index] = this->test_inner_input_network->s_input_input->errors[initial_size+s_index];
			this->test_inner_input_network->s_input_input->errors[initial_size+s_index] = 0.0;
		}
		if (this->state_iter <= 110000) {
			this->inner_input_input_networks.back()->backprop_small_weights_with_no_error_signal(
				inner_input_input_errors,
				0.05);
		} else if (this->state_iter <= 130000) {
			this->inner_input_input_networks.back()->backprop_small_weights_with_no_error_signal(
				inner_input_input_errors,
				0.01);
		} else {
			this->inner_input_input_networks.back()->backprop_small_weights_with_no_error_signal(
				inner_input_input_errors,
				0.002);
		}
	}

	vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		scope_input[i_index] = this->curr_inner_input_network->output->acti_vals[i_index];
	}

	double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
	scale_factor *= scope_scale_mod_val;

	vector<double> scope_output;
	ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
	this->existing_actions[this->finished_steps.size()]->update_activate(problem,
																		 scope_input,
																		 scope_output,
																		 predicted_score,
																		 scale_factor,
																		 run_status,
																		 scope_history);
	history->scope_histories[this->finished_steps.size()] = scope_history;

	scale_factor /= scope_scale_mod_val;

	if (run_status.exceeded_depth) {
		history->exit_index = (int)this->finished_steps.size();
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	}

	// not folded yet
	fold_input.push_back(scope_output);
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(scope_output);
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
			this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
														   local_s_input_vals,
														   state_vals);
			vector<double> scope_input(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[f_index]);
			this->existing_actions[f_index]->existing_update_activate(problem,
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
				if (this->is_existing[ff_index]) {
					input_fold_inputs[ff_index].push_back(scope_output);
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
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::inner_scope_input_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	double predicted_score_error = target_val - predicted_score;

	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		vector<double> curr_fold_error{scale_factor*predicted_score_error};
		this->curr_fold->backprop_fold_weights_with_no_error_signal(
			curr_fold_error,
			0.001,
			history->curr_fold_history);

		next_predicted_score = predicted_score;	// doesn't matter
		predicted_score -= scale_factor*history->ending_score_update;
	}

	for (int f_index = history->exit_index; f_index >= (int)this->finished_steps.size()+1; f_index--) {
		if (this->is_existing[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			double scope_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_update_backprop(predicted_score,
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

	if (history->exit_index < (int)this->finished_steps.size()) {
		// do nothing
	} else {
		double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		double scope_scale_factor_error = 0.0;
		this->existing_actions[this->finished_steps.size()]->update_backprop(
			predicted_score,
			next_predicted_score,
			target_val,
			scale_factor,
			scope_scale_factor_error,
			history->scope_histories[this->finished_steps.size()]);

		vector<double> mod_errors{scope_scale_factor_error};
		this->scope_scale_mod[this->finished_steps.size()]->backprop(mod_errors, 0.0002);

		scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

		scale_factor /= scope_scale_mod_val;
	}

	for (int f_index = (int)this->finished_steps.size()-1; f_index >= 0; f_index--) {
		if (history->exit_index < f_index) {
			// do nothing
		} else {
			this->finished_steps[f_index]->update_backprop(predicted_score,
														   next_predicted_score,
														   target_val,
														   scale_factor,
														   scale_factor_error,
														   history->finished_step_histories[f_index]);
		}
	}

	double misguess = (target_val - predicted_score)*(target_val - predicted_score);
	this->starting_average_misguess = 0.999*this->starting_average_misguess + 0.001*misguess;

	this->starting_average_score = 0.999*this->starting_average_score + 0.001*predicted_score;

	next_predicted_score = predicted_score;
	// starting score_network updated in branch
	predicted_score -= history->starting_score_update;	// already scaled

	this->starting_average_local_impact = 0.999*this->starting_average_local_impact
		+ 0.001*abs(history->starting_score_update);
}

void Fold::inner_scope_input_step_existing_update_activate(
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

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[f_index]);
		this->finished_steps[f_index]->existing_update_activate(problem,
																s_input_vals,
																state_vals,
																predicted_score,
																scale_factor,
																run_status,
																finished_step_history);
		history->finished_step_histories.push_back(finished_step_history);

		if (run_status.exceeded_depth) {
			history->exit_index = f_index;
			history->exit_location = EXIT_LOCATION_SPOT;
			return;
		}

		fold_input.push_back(vector<double>());	// empty
		for (int ff_index = (int)this->finished_steps.size()+1; ff_index < this->sequence_length; ff_index++) {
			if (this->is_existing[ff_index]) {
				input_fold_inputs[ff_index].push_back(vector<double>());	// empty
			}
		}
	}

	// this->is_existing[this->finished_steps.size()] == true
	for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
		this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																  state_vals[this->inner_input_input_layer[i_index]]);
		for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
			s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
				this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	this->curr_inner_input_network->activate_subfold(s_input_vals[this->curr_inner_input_network->subfold_index+1],
											   state_vals);
	vector<double> scope_input(this->existing_actions[this->finished_steps.size()]->num_inputs);
	for (int i_index = 0; i_index < this->existing_actions[this->finished_steps.size()]->num_inputs; i_index++) {
		scope_input[i_index] = this->curr_inner_input_network->output->acti_vals[i_index];
	}

	double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
	scale_factor *= scope_scale_mod_val;

	vector<double> scope_output;
	ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[this->finished_steps.size()]);
	this->existing_actions[this->finished_steps.size()]->existing_update_activate(
		problem,
		scope_input,
		scope_output,
		predicted_score,
		scale_factor,
		run_status,
		scope_history);
	history->scope_histories[this->finished_steps.size()] = scope_history;

	scale_factor /= scope_scale_mod_val;

	if (run_status.exceeded_depth) {
		history->exit_index = (int)this->finished_steps.size();
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	}

	// not folded yet
	fold_input.push_back(scope_output);
	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			input_fold_inputs[f_index].push_back(scope_output);
		}
	}

	for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
			this->curr_input_folds[f_index]->activate_fold(input_fold_inputs[f_index],
														   local_s_input_vals,
														   state_vals);
			vector<double> scope_input(this->existing_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->existing_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->existing_actions[f_index]);
			this->existing_actions[f_index]->existing_update_activate(problem,
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

	this->curr_fold->activate_fold(fold_input,
								   local_s_input_vals,
								   state_vals);
	history->ending_score_update = this->curr_fold->output->acti_vals[0];
	predicted_score += scale_factor*this->curr_fold->output->acti_vals[0];

	this->curr_end_fold->activate_fold(fold_input,
									   local_s_input_vals,
									   state_vals);
	local_state_vals.clear();
	local_state_vals.reserve(this->num_outputs);
	for (int o_index = 0; o_index < this->num_outputs; o_index++) {
		local_state_vals.push_back(this->curr_end_fold->output->acti_vals[o_index]);
	}
}

void Fold::inner_scope_input_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {
	if (history->exit_location == EXIT_LOCATION_NORMAL) {
		scale_factor_error += history->ending_score_update*predicted_score_error;

		predicted_score -= scale_factor*history->ending_score_update;
	}

	for (int f_index = history->exit_index; f_index >= (int)this->finished_steps.size()+1; f_index--) {
		if (this->is_existing[f_index]) {
			double scope_scale_mod_val = this->scope_scale_mod[f_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			double scope_scale_factor_error = 0.0;
			this->existing_actions[f_index]->existing_update_backprop(predicted_score,
																	  predicted_score_error,
																	  scale_factor,
																	  scope_scale_factor_error,
																	  history->scope_histories[f_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;
		}
	}

	if (history->exit_index < (int)this->finished_steps.size()) {
		// do nothing
	} else {
		// this->is_existing[this->finished_steps.size()] == true
		double scope_scale_mod_val = this->scope_scale_mod[this->finished_steps.size()]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		double scope_scale_factor_error = 0.0;
		this->existing_actions[this->finished_steps.size()]->existing_update_backprop(
			predicted_score,
			predicted_score_error,
			scale_factor,
			scope_scale_factor_error,
			history->scope_histories[this->finished_steps.size()]);

		scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

		scale_factor /= scope_scale_mod_val;
	}

	for (int f_index = (int)this->finished_steps.size()-1; f_index >= 0; f_index--) {
		if (history->exit_index < f_index) {
			// do nothing
		} else {
			this->finished_steps[f_index]->existing_update_backprop(predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scale_factor_error,
																	history->finished_step_histories[f_index]);
		}
	}

	predicted_score -= history->starting_score_update;	// already scaled
}
