#include "branch.h"

#include <iostream>
#include <limits>

using namespace std;



void Branch::activate(vector<vector<double>>& flat_vals,
					  vector<double>& input_state_vals,
					  vector<double>& s_input_vals,
					  vector<double>& output_state_vals,
					  double& predicted_score,
					  double& scale_factor) {
	double best_score = numeric_limits<double>::lowest();
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		this->score_networks[b_index]->activate_small(input_state_vals,
													  s_input_vals);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
		if (curr_predicted_score > best_score) {
			best_score = curr_score;
			this->best_index = b_index;
		}
	}

	predicted_score += curr_predicted_score;

	vector<double> scope_state_vals;
	if (this->compress_sizes[this->best_index] > 0) {
		if (this->active_compress[this->best_index]) {
			this->compress_networks[this->best_index]->activate_small(input_state_vals,
																	  s_input_vals);
			int compress_new_size = (int)input_state_vals.size() - this->compress_sizes[this->best_index];
			scope_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				scope_state_vals.push_back(this->compress_networks[this->best_index]->output->acti_vals[s_index]);
			}
		} else {
			scope_state_vals = vector<double>(input_state_vals.begin(),
				input_state_vals.end()-this->compress_sizes[this->best_index]);
		}
	} else {
		scope_state_vals = input_state_vals;
	}

	this->branches[this->best_index]->activate(flat_vals,
											   scope_state_vals,
											   s_input_vals,
											   output_state_vals,
											   predicted_score,
											   scale_factor);

	predicted_score += scale_factor*this->end_averages_mods[this->best_index];
	scale_factor *= this->end_scale_mods[this->best_index];
}

void Branch::existing_backprop(vector<double> state_input_errors,
							   vector<double>& state_output_errors,
							   vector<double>& s_input_errors,
							   double& predicted_score,
							   double predicted_score_error,
							   double& scale_factor,
							   double new_scale_factor,
							   double& new_scale_factor_error) {
	scale_factor /= this->end_scale_mods[this->best_index];
	predicted_score -= scale_factor*this->end_averages_mods[this->best_index];
	// end_mods don't need to be accounted for in backprop as not changing them

	vector<double> branch_state_output_errors;
	this->branches[this->best_index]->existing_backprop(state_input_errors,
														s_input_errors,
														branch_state_output_errors,
														predited_score,
														predicted_score_error,
														scale_factor,
														new_scale_factor,
														new_scale_factor_error);

	if (this->compress_sizes[this->best_index] > 0) {
		int state_output_errors_size = branch_state_output_errors.size()+this->compress_sizes[this->best_index];
		state_output_errors = vector<double>(state_output_errors_size, 0.0);
		if (this->active_compress[this->best_index]) {
			vector<double> compress_state_output_errors;
			vector<double> compress_s_input_output_errors
			this->compress_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
				branch_state_output_errors,
				compress_state_output_errors,
				compress_s_input_output_errors);
			for (int s_index = 0; s_index < (int)compress_state_output_errors.size(); s_index++) {
				state_output_errors[s_index] += compress_state_output_errors[s_index];
			}
			// use output sizes as might not have used all inputs
			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
				s_input_errors[s_index] += compress_s_input_output_errors[s_index];
			}
		} else {
			for (int s_index = 0; s_index < (int)branch_state_output_errors.size(); s_index++) {
				// leave compressed s_input errors initialized at 0
				state_output_errors[s_index] += branch_state_output_errors[s_index];
			}
		}
	} else {
		state_output_errors = branch_state_output_errors;
	}

	// with only predicted_score_error, don't worry about average_factor yet

	double score_add_w_o_new_scale = scale_factor/new_scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
	new_scale_factor_error += score_add_w_o_new_scale*predicted_score_error;

	vector<double> score_errors{scale_factor*predicted_score_error}
	vector<double> score_state_output_errors;
	vector<double> score_s_input_output_errors;
	this->score_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
		score_errors,
		score_state_output_errors,
		score_s_input_output_errors);
	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
		state_output_errors[s_index] += score_state_output_errors[s_index];
	}
	// use output sizes as might not have used all inputs
	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
		s_input_errors[s_index] += score_s_input_output_errors[s_index];
	}

	predicted_score -= scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
}

void Branch::explore_activate(vector<vector<double>>& flat_vals,
							  vector<double>& input_state_vals,
							  vector<double>& s_input_vals,
							  vector<double>& output_state_vals,
							  double& predicted_score,
							  double& scale_factor,
							  double& new_scale_factor) {
	double best_score = numeric_limits<double>::lowest();
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		this->score_networks[b_index]->activate_small(input_state_vals,
													  s_input_vals);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
		if (curr_predicted_score > best_score) {
			best_score = curr_score;
			this->best_index = b_index;
		}
	}

	predicted_score += curr_predicted_score;

	vector<double> scope_state_vals;
	if (this->compress_sizes[this->best_index] > 0) {
		if (this->active_compress[this->best_index]) {
			this->compress_networks[this->best_index]->activate_small(input_state_vals,
																	  s_input_vals);
			int compress_new_size = (int)input_state_vals.size() - this->compress_sizes[this->best_index];
			scope_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				scope_state_vals.push_back(this->compress_networks[this->best_index]->output->acti_vals[s_index]);
			}
		} else {
			scope_state_vals = vector<double>(input_state_vals.begin(),
				input_state_vals.end()-this->compress_sizes[this->best_index]);
		}
	} else {
		scope_state_vals = input_state_vals;
	}

	this->branches[this->best_index]->activate(flat_vals,
											   scope_state_vals,
											   s_input_vals,
											   output_state_vals,
											   predicted_score,
											   scale_factor);

	predicted_score += scale_factor*this->end_averages_mods[this->best_index];
	scale_factor *= this->end_scale_mods[this->best_index];
}

void Branch::explore_backprop(vector<double> state_input_errors,
							  vector<double>& state_output_errors,
							  vector<double>& s_input_errors,
							  double& predicted_score,
							  double target_val,
							  double& scale_factor,
							  double& new_average_factor_error,
							  double new_scale_factor,
							  double& new_scale_factor_error) {
	scale_factor /= this->end_scale_mods[this->best_index];
	predicted_score -= scale_factor*this->end_averages_mods[this->best_index];
	// end_mods don't need to be accounted for in backprop as not changing them

	vector<double> branch_state_output_errors;
	this->branches[this->best_index]->explore_backprop(state_input_errors,
													   s_input_errors,
													   branch_state_output_errors,
													   predited_score,
													   target_val,
													   scale_factor,
													   new_average_factor_error,
													   new_scale_factor,
													   new_scale_factor_error);

	if (this->compress_sizes[this->best_index] > 0) {
		int state_output_errors_size = branch_state_output_errors.size()+this->compress_sizes[this->best_index];
		state_output_errors = vector<double>(state_output_errors_size, 0.0);
		if (this->active_compress[this->best_index]) {
			vector<double> compress_state_output_errors;
			vector<double> compress_s_input_output_errors
			this->compress_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
				branch_state_output_errors,
				compress_state_output_errors,
				compress_s_input_output_errors);
			for (int s_index = 0; s_index < (int)compress_state_output_errors.size(); s_index++) {
				state_output_errors[s_index] += compress_state_output_errors[s_index];
			}
			// use output sizes as might not have used all inputs
			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
				s_input_errors[s_index] += compress_s_input_output_errors[s_index];
			}
		} else {
			for (int s_index = 0; s_index < (int)branch_state_output_errors.size(); s_index++) {
				// leave compressed s_input errors initialized at 0
				state_output_errors[s_index] += branch_state_output_errors[s_index];
			}
		}
	} else {
		state_output_errors = branch_state_output_errors;
	}

	double predicted_score_error = target_val - predicted_score;

	new_average_factor_error += predicted_score_error;

	double score_add_w_o_new_scale = scale_factor/new_scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
	new_scale_factor_error += score_add_w_o_new_scale*predicted_score_error;

	vector<double> score_errors{scale_factor*predicted_score_error}
	vector<double> score_state_output_errors;
	vector<double> score_s_input_output_errors;
	this->score_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
		score_errors,
		score_state_output_errors,
		score_s_input_output_errors);
	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
		state_output_errors[s_index] += score_state_output_errors[s_index];
	}
	// use output sizes as might not have used all inputs
	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
		s_input_errors[s_index] += score_s_input_output_errors[s_index];
	}

	predicted_score -= scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
}
