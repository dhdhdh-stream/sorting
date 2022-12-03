#include "finished_step.h"

#include <iostream>

using namespace std;



void FinishedStep::explore_on_path_activate(vector<vector<double>>& flat_vals,
											vector<vector<double>>& s_input_vals,
											vector<vector<double>>& state_vals,
											double& predicted_score,
											double& scale_factor,
											FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
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

		this->inner_input_network->activate_small(s_input_vals.back(),
												  state_vals.back());
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->explore_off_path_activate(flat_vals,
											   scope_input,
											   scope_output,
											   predicted_score,
											   scale_factor,
											   EXPLORE_PHASE_NONE,
											   scope_history);
		this->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals);
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals);
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

// cannot be explore_on_path_backprop for FinishedStep

void FinishedStep::explore_off_path_activate(vector<vector<double>>& flat_vals,
											 vector<vector<double>>& s_input_vals,
											 vector<vector<double>>& state_vals,
											 double& predicted_score,
											 double& scale_factor,
											 int& explore_phase,
											 FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* inner_input_input_history = new FoldNetworkHistory(this->inner_input_input_networks[i_index]);
				this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																		  state_vals[this->inner_input_input_layer[i_index]],
																		  inner_input_input_history);
				history->inner_input_input_network_histories.push_back(inner_input_input_history);
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
			FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_network);
			this->inner_input_network->activate_small(s_input_vals.back(),
													  state_vals.back(),
													  inner_input_network_history);
			history->inner_input_network_history = inner_input_network_history;
		} else {
			this->inner_input_network->activate_small(s_input_vals.back(),
													  state_vals.back());
		}
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->explore_off_path_activate(flat_vals,
											   scope_input,
											   scope_output,
											   predicted_score,
											   scale_factor,
											   explore_phase,
											   scope_history);
		this->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		if (explore_phase == EXPLORE_PHASE_FLAT) {
			FoldNetworkHistory* input_network_history = new FoldNetworkHistory(this->input_networks[i_index]);
			this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
														  state_vals[this->input_layer[i_index]],
														  input_network_history);
			history->input_network_histories.push_back(input_network_history);
		} else {
			this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
														  state_vals[this->input_layer[i_index]]);
		}
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_network);
		this->score_network->activate_subfold(s_input_vals[s_input_index],
											  state_vals,
											  score_network_history);
		history->score_network_history = score_network_history;
	} else {
		this->score_network->activate_subfold(s_input_vals[s_input_index],
											  state_vals);
	}
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_network);
				this->compress_network->activate_subfold(s_input_vals[s_input_index],
														 state_vals,
														 compress_network_history);
				history->compress_network_history = compress_network_history;
			} else {
				this->compress_network->activate_subfold(s_input_vals[s_input_index],
														 state_vals);
			}
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::explore_off_path_backprop(vector<vector<double>>& s_input_errors,
											 vector<vector<double>>& state_errors,
											 double& predicted_score,
											 double target_val,
											 double& scale_factor,
											 double& scale_factor_error,
											 FinishedStepHistory* history) {
	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->backprop_subfold_errors_with_no_weight_change(
				state_errors.back(),
				history->compress_network_history);

			// don't pop last_s_input_errors
			state_errors.pop_back();
			for (int l_index = 1; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}

			for (int s_index = 0; s_index < (int)s_input_errors[s_input_errors.size()-this->compress_num_layers].size(); s_index++) {
				s_input_errors[s_input_errors.size()-this->compress_num_layers][s_index] += this->compress_network->s_input_input->errors[s_index];
				this->compress_network->s_input_input->errors[s_index] = 0.0;
			}
			for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
				for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
					state_errors[l_index][s_index] += this->compress_network->state_inputs[l_index]->errors[s_index];
					this->compress_network->state_inputs[l_index]->errors[s_index] = 0.0;
				}
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}
		}
	}

	double predicted_score_error = target_val - predicted_score;

	scale_factor_error += this->score_update*predicted_score_error;

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	vector<double> score_errors{scale_factor*predicted_score_error};
	this->score_network->backprop_subfold_errors_with_no_weight_change(
		score_errors,
		history->score_network_history);
	for (int s_index = 0; s_index < (int)s_input_errors[s_input_index].size(); s_index++) {
		s_input_errors[s_input_index][s_index] += this->score_network->s_input_input->errors[s_index];
		this->score_network->s_input_input->errors[s_index] = 0.0;
	}
	for (int l_index = s_input_index; l_index < (int)state_errors.size(); l_index++) {
		for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
			state_errors[l_index][s_index] += this->score_network->state_inputs[l_index]->errors[s_index];
			this->score_network->state_inputs[l_index]->errors[s_index] = 0.0;
		}
	}
	predicted_score -= scale_factor*this->score_update;

	for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
		vector<double> input_errors(this->input_sizes[i_index]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			input_errors[this->input_sizes[i_index]-1-s_index] = s_input_errors[this->input_layer[i_index]+1].back();
			s_input_errors[this->input_layer[i_index]+1].pop_back();
		}
		vector<double> s_input_output_errors;
		vector<double> state_output_errors;
		this->input_networks[i_index]->backprop_small_errors_with_no_weight_change(
			input_errors,
			s_input_output_errors,
			state_output_errors,
			history->input_network_histories[i_index]);
		for (int s_index = 0; s_index < (int)s_input_errors[this->input_layer[i_index]].size(); s_index++) {
			s_input_errors[this->input_layer[i_index]][s_index] += s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors[this->input_layer[i_index]].size(); s_index++) {
			state_errors[this->input_layer[i_index]][s_index] += state_output_errors[s_index];
		}
	}

	if (!this->is_inner_scope) {
		s_input_errors.pop_back();
		state_errors.pop_back();
	} else {
		vector<double> scope_input_errors = state_errors.back();
		state_errors.pop_back();

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->scope->explore_off_path_backprop(scope_input_errors,
											   scope_output_errors,
											   predicted_score,
											   target_val,
											   scale_factor,
											   scope_scale_factor_error,
											   history->scope_history);

		scale_factor_error += this->scope_scale_mod*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod;

		vector<double> inner_input_s_input_output_errors;
		vector<double> inner_input_state_output_errors;
		this->inner_input_network->backprop_small_errors_with_no_weight_change(
			scope_output_errors,
			inner_input_s_input_output_errors,
			inner_input_state_output_errors,
			history->inner_input_network_history);
		for (int s_index = 0; s_index < (int)s_input_errors.back().size(); s_index++) {
			s_input_errors.back()[s_index] += inner_input_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors.back().size(); s_index++) {
			state_errors.back()[s_index] += inner_input_state_output_errors[s_index];
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
}

void FinishedStep::existing_flat_activate(vector<vector<double>>& flat_vals,
										  vector<vector<double>>& s_input_vals,
										  vector<vector<double>>& state_vals,
										  double& predicted_score,
										  double& scale_factor,
										  FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
		s_input_vals.push_back(vector<double>());
		state_vals.push_back(flat_vals.begin());
		flat_vals.erase(flat_vals.begin());
	} else {
		for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
			FoldNetworkHistory* inner_input_input_history = new FoldNetworkHistory(this->inner_input_input_networks[i_index]);
			this->inner_input_input_networks[i_index]->activate_small(s_input_vals[this->inner_input_input_layer[i_index]],
																	  state_vals[this->inner_input_input_layer[i_index]],
																	  inner_input_input_history);
			history->inner_input_input_network_histories.push_back(inner_input_input_history);
			for (int s_index = 0; s_index < this->inner_input_input_sizes[i_index]; s_index++) {
				s_input_vals[this->inner_input_input_layer[i_index]+1].push_back(
					this->inner_input_input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->curr_inner_input_network);
		this->curr_inner_input_network->activate_small(s_input_vals.back(),
													   state_vals.back(),
													   inner_input_network_history);
		history->inner_input_network_history = inner_input_network_history;
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->curr_inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->existing_flat_activate(flat_vals,
											scope_input,
											scope_output,
											predicted_score,
											scale_factor,
											scope_history);
		this->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		FoldNetworkHistory* input_network_history = new FoldNetworkHistory(this->input_networks[i_index]);
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]],
													  input_network_history);
		history->input_network_histories.push_back(input_network_history);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_network);
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals,
										  score_network_history);
	history->score_network_history = score_network_history;
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_network);
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals,
													 compress_network_history);
			history->compress_network_history = compress_network_history;
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::existing_flat_backprop(vector<vector<double>>& s_input_errors,
										  vector<vector<double>>& state_errors,
										  double& predicted_score,
										  double predicted_score_error,
										  double& scale_factor,
										  double& scale_factor_error,
										  FinishedStepHistory* history) {
	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->backprop_subfold_errors_with_no_weight_change(
				state_errors.back(),
				history->compress_network_history);

			// don't pop last_s_input_errors
			state_errors.pop_back();
			for (int l_index = 1; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}

			for (int s_index = 0; s_index < (int)s_input_errors[s_input_errors.size()-this->compress_num_layers].size(); s_index++) {
				s_input_errors[s_input_errors.size()-this->compress_num_layers][s_index] += this->compress_network->s_input_input->errors[s_index];
				this->compress_network->s_input_input->errors[s_index] = 0.0;
			}
			for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
				for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
					state_errors[l_index][s_index] += this->compress_network->state_inputs[l_index]->errors[s_index];
					this->compress_network->state_inputs[l_index]->errors[s_index] = 0.0;
				}
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_errors.push_back(vector<double>(this->compressed_s_input_sizes[l_index], 0.0));
				state_errors.push_back(vector<double>(this->compressed_scope_sizes[l_index], 0.0));
			}
		}
	}

	scale_factor_error += this->score_update*predicted_score_error;

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	vector<double> score_errors{scale_factor*predicted_score_error};
	this->score_network->backprop_subfold_errors_with_no_weight_change(
		score_errors,
		history->score_network_history);
	for (int s_index = 0; s_index < (int)s_input_errors[s_input_index].size(); s_index++) {
		s_input_errors[s_input_index][s_index] += this->score_network->s_input_input->errors[s_index];
		this->score_network->s_input_input->errors[s_index] = 0.0;
	}
	for (int l_index = s_input_index; l_index < (int)state_errors.size(); l_index++) {
		for (int s_index = 0; s_index < (int)state_errors[l_index].size(); s_index++) {
			state_errors[l_index][s_index] += this->score_network->state_inputs[l_index]->errors[s_index];
			this->score_network->state_inputs[l_index]->errors[s_index] = 0.0;
		}
	}
	predicted_score -= scale_factor*this->score_update;

	for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
		vector<double> input_errors(this->input_sizes[i_index]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			input_errors[this->input_sizes[i_index]-1-s_index] = s_input_errors[this->input_layer[i_index]+1].back();
			s_input_errors[this->input_layer[i_index]+1].pop_back();
		}
		vector<double> s_input_output_errors;
		vector<double> state_output_errors;
		this->input_networks[i_index]->backprop_small_errors_with_no_weight_change(
			input_errors,
			s_input_output_errors,
			state_output_errors,
			history->input_network_histories[i_index]);
		for (int s_index = 0; s_index < (int)s_input_errors[this->input_layer[i_index]].size(); s_index++) {
			s_input_errors[this->input_layer[i_index]][s_index] += s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors[this->input_layer[i_index]].size(); s_index++) {
			state_errors[this->input_layer[i_index]][s_index] += state_output_errors[s_index];
		}
	}

	if (!this->is_inner_scope) {
		s_input_errors.pop_back();
		state_errors.pop_back();
	} else {
		vector<double> scope_input_errors = state_errors.back();
		state_errors.pop_back();

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->scope->existing_flat_backprop(scope_input_errors,
											scope_output_errors,
											predicted_score,
											predicted_score_error,
											scale_factor,
											scope_scale_factor_error,
											history->scope_history);

		scale_factor_error += this->scope_scale_mod*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod;

		vector<double> inner_input_s_input_output_errors;
		vector<double> inner_input_state_output_errors;
		this->inner_input_network->backprop_small_errors_with_no_weight_change(
			scope_output_errors,
			inner_input_s_input_output_errors,
			inner_input_state_output_errors,
			history->inner_input_network_history);
		for (int s_index = 0; s_index < (int)s_input_errors.back().size(); s_index++) {
			s_input_errors.back()[s_index] += inner_input_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)state_errors.back().size(); s_index++) {
			state_errors.back()[s_index] += inner_input_state_output_errors[s_index];
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
}

void FinishedStep::update_activate(vector<vector<double>>& flat_vals,
								   vector<vector<double>>& s_input_vals,
								   vector<vector<double>>& state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
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

		this->inner_input_network->activate_small(s_input_vals.back(),
												  state_vals.back());
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->update_activate(flat_vals,
									 scope_input,
									 scope_output,
									 predicted_score,
									 scale_factor,
									 scope_history);
		this->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_network);
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals,
										  score_network_history);
	history->score_network_history = score_network_history;
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals);
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::update_backprop(double& predicted_score,
								   double& next_predicted_score,
								   double target_val,
								   double& scale_factor,
								   FinishedStepHistory* history) {
	double predicted_score_error = target_val - predicted_score;

	vector<double> score_errors{scale_factor*predicted_score_error};
	this->score_network->backprop_subfold_weights_with_no_error_signal(
		score_errors,
		0.001,
		history->score_network_history);
	next_predicted_score = predicted_score;
	predicted_score -= scale_factor*this->score_update;

	if (!this->is_inner_scope) {
		// do nothing
	} else {
		scale_factor *= this->scope_scale_mod;

		this->scope->update_backprop(predicted_score,
									 next_predicted_score,
									 target_val,
									 scale_factor,
									 history->scope_history);

		scale_factor /= this->scope_scale_mod;
	}
}

void FinishedStep::existing_update_activate(vector<vector<double>>& flat_vals,
											vector<vector<double>>& s_input_vals,
											vector<vector<double>>& state_vals,
											double& predicted_score,
											double& scale_factor,
											FinishedStepHistory* history) {
	if (!this->is_inner_scope) {
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

		this->inner_input_network->activate_small(s_input_vals.back(),
												  state_vals.back());
		vector<double> scope_input(this->scope->num_inputs);	// also new s_input_vals
		for (int s_index = 0; s_index < this->scope->num_inputs; s_index++) {
			scope_input[s_index] = this->inner_input_network->output->acti_vals[s_index];
		}

		scale_factor *= this->scope_scale_mod;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scope);	// though not important as will not backprop
		this->scope->existing_update_activate(flat_vals,
											  scope_input,
											  scope_output,
											  predicted_score,
											  scale_factor,
											  scope_history);
		this->scope_history = scope_history;

		scale_factor /= this->scope_scale_mod;

		s_input_vals.push_back(scope_input);
		state_vals.push_back(scope_output);
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate_small(s_input_vals[this->input_layer[i_index]],
													  state_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	int s_input_index;
	if (this->compress_num_layers > 0) {
		s_input_index = s_input_vals.size()-this->compress_num_layers;
	} else {
		s_input_index = s_input_vals.size()-1;
	}
	this->score_network->activate_subfold(s_input_vals[s_input_index],
										  state_vals);
	history->score_update = this->score_network->output->acti_vals[0];
	predicted_score += scale_factor*this->score_network->output->acti_vals[0];

	if (this->compress_num_layers > 0) {
		if (this->active_compress) {
			this->compress_network->activate_subfold(s_input_vals[s_input_index],
													 state_vals);
			for (int l_index = 0; l_index < this->compress_num_layers-1; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
			// don't pop last s_input_vals
			state_vals.pop_back();
			state_vals.push_back(vector<double>(this->compress_new_size));
			for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
				state_vals.back()[s_index] = this->compress_network->output->acti_vals[s_index];
			}
		} else {
			for (int l_index = 0; l_index < this->compress_num_layers; l_index++) {
				s_input_vals.pop_back();
				state_vals.pop_back();
			}
		}
	}
}

void FinishedStep::existing_update_backprop(double& predicted_score,
											double predicted_score_error,
											double& scale_factor,
											double& scale_factor_error,
											FinishedStepHistory* history) {
	scale_factor_error += this->score_update*predicted_score_error;

	predicted_score -= scale_factor*this->score_update;

	if (!this->is_inner_scope) {
		// do nothing
	} else {
		scale_factor *= this->scope_scale_mod;

		double scope_scale_factor_error = 0.0;
		this->scope->existing_update_backprop(predicted_score,
											  predicted_score_error,
											  scale_factor,
											  scope_scale_factor_error,
											  history->scope_history);

		scale_factor_error += this->scope_scale_mod*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod;
	}
}
