#include "branch_path.h"

#include <iostream>

using namespace std;



void BranchPath::activate(vector<vector<double>>& flat_vals,
						  double starting_score,
						  int starting_branch_index,
						  vector<double>& starting_combined_state_vals,
						  vector<double>& s_input_vals,
						  vector<double>& output_state_vals,
						  double& predicted_score,
						  double& scale_factor) {
	vector<double> local_state_vals;

	// start
	if (this->is_branch[0]) {
		vector<double> output_state_vals;
		this->branches[0]->activate(flat_vals,
									starting_score,
									starting_combined_state_vals,
									s_input_vals,
									output_state_vals,
									predicted_score,
									scale_factor);
		local_state_vals = output_state_vals;
	} else {
		// starting_score already scaled
		predicted_score += starting_score;

		if (this->compress_sizes[0] > 0) {
			if (this->active_compress[0]) {
				this->compress_networks[0]->activate_small(starting_combined_state_vals,
														   s_input_vals);
				int compress_new_size = (int)starting_combined_state_vals.size() - this->compress_sizes[0];
				local_state_vals.reserve(compress_new_size);
				for (int s_index = 0; s_index < compress_new_size; s_index++) {
					local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals = vector<double>(starting_combined_state_vals.begin(),
					starting_combined_state_vals.end()-this->compress_sizes[0]);
			}
		} else {
			local_state_vals = starting_combined_state_vals;
		}
	}

	// mid
	for (int a_index = 1; a_index < (int)this->scopes.size(); a_index++) {
		vector<double> new_state_vals;

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			new_state_vals = flat_vals.begin();
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate(local_state_vals,
																	   input);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			predicted_score += scale_factor*this->scope_average_mod[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			scope_scope->activate(flat_vals,
								  temp_new_s_input_vals,
								  scope_output,
								  predicted_score,
								  scale_factor);

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals = scope_output;
		}

		if (this->need_process[a_index]) {
			vector<double> combined_state_vals = local_state_vals;
			combined_state_vals.insert(combined_state_vals.end(),
				new_state_vals.begin(), new_state_vals.end());

			if (this->is_branch[a_index]) {
				vector<double> output_state_vals;
				this->branches[a_index]->activate(combined_state_vals,
												  input,
												  output_state_vals,
												  flat_vals,
												  predicted_score,
												  scale_factor);
				local_state_vals = output_state_vals;
			} else {
				this->score_networks[a_index]->activate_small(combined_state_vals,
															  inputs);
				predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

				if (this->active_compress[a_index]) {
					// compress 2 layers, add 1 layer
					this->compress_networks[a_index]->activate_small(combined_state_vals,
																	 inputs);

					local_state_vals.clear();
					local_state_vals.reserve(this->compress_new_sizes[a_index]);
					for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
						local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
					}
				}
				// else just let new_state_vals go
			}
		}
	}

	this->end_input_network->activate_small(local_state_vals,
											local_s_input_vals);
	output_state_vals.reserve(this->end_input_size);
	for (int i_index = 0; i_index < this->end_input_size; i_index++) {
		output_state_vals.push_back(this->end_input_network->output->acti_vals[i_index]);
	}
}

void BranchPath::existing_backprop(vector<double> input_state_errors,
								   vector<double>& output_state_errors,
								   vector<double> s_input_errors,
								   double& predicted_score,
								   double predicted_score_error,
								   double& scale_factor,
								   double new_scale_factor,
								   double& new_scale_factor_error) {
	vector<double> local_state_errors;	// i.e., end_state_output_errors
	vector<double> end_s_input_output_errors;
	this->end_input_network->backprop_small_errors_with_no_weight_change(
		input_state_errors,
		local_state_errors,
		end_s_input_output_errors);
	for (int s_index = 0; s_index < (int)end_s_input_output_errors.size(); s_index++) {
		s_input_errors[s_index] += end_s_input_output_errors[s_index];
	}

	for (int a_index = (int)this->scopes.size()-1; a_index >= 0; a_index--) {
		if (this->need_process[a_index]) {
			if (this->is_branch[a_index]) {
				vector<double> branch_state_output_errors;
				this->branches[a_index]->existing_backprop(local_state_errors,
														   branch_state_output_errors,
														   s_input_errors,
														   predicted_score_error,
														   scale_factor,
														   new_scale_factor,
														   new_scale_factor_error);
				local_state_errors = branch_state_output_errors;
			} else {
				if (this->active_compress[a_index]) {
					vector<double> compress_state_output_errors;
					vector<double> compress_s_input_output_errors;
					this->compress_networks[a_index]->backprop_small_errors_with_no_weight_change(
						local_state_errors,
						compress_state_output_errors,
						compress_s_input_output_errors);
					local_state_errors = compress_state_output_errors;
					// use output sizes as might not have used all inputs
					for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
						s_input_errors[s_index] += compress_s_input_output_errors[s_index];
					}
				} else {
					for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
						local_state_errors.push_back(0.0);
					}
				}

				// with only predicted_score_error, don't worry about average_factor yet

				double score_add_w_o_new_scale = scale_factor/new_scale_factor*this->score_networks[a_index]->output->acti_vals[0];
				new_scale_factor_error += score_add_w_o_new_scale*predicted_score_error;

				vector<double> score_errors{scale_factor*predicted_score_error};
				vector<double> score_state_output_errors;
				vector<double> score_s_input_output_errors;
				this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
					score_errors,
					score_state_output_errors,
					score_s_input_output_errors);
				for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
					local_state_errors[s_index] += score_state_output_errors[s_index];
				}
				// use output sizes as might not have used all inputs
				for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
					s_input_errors[s_index] += score_s_input_output_errors[s_index];
				}
			}
		}

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output_errors;
			scope_scope->existing_backprop(local_state_errors,
										   scope_output_errors,
										   predicted_score,
										   predicted_score_error,
										   scale_factor,
										   new_scale_factor,
										   new_scale_factor_error);

			scale_factor /= this->scope_scale_mod[a_index];

			predicted_score -= scale_factor*this->scope_average_mod[a_index];

			for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = scope_output_errors.back();
					scope_output_errors.pop_back();
				}
				vector<double> inner_input_state_output_errors;
				vector<double> inner_input_s_input_output_errors;
				this->inner_input_networks[a_index][i_index]->backprop_small_errors_with_no_weight_change(
					inner_input_errors,
					inner_input_state_output_errors,
					inner_input_s_input_output_errors);
				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
				}
				// use output sizes as might not have used all inputs
				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
					s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
				}
			}
		}
	}

	output_state_errors = local_state_errors;
}

void BranchPath::explore_activate(vector<vector<double>>& flat_vals,
								  vector<double>& local_state_vals,
								  vector<double>& local_s_input_vals,
								  vector<double>& output_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  double& new_scale_factor) {
	int a_index = 0;

	while (a_index < (int)this->scopes.size()) {
		vector<double> new_state_vals;

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			new_state_vals = flat_vals.begin();
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate(local_state_vals,
																	   input);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			predicted_score += scale_factor*this->scope_average_mod[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			scope_scope->explore_activate(flat_vals,
										  temp_new_s_input_vals,
										  scope_output,
										  predicted_score,
										  scale_factor,
										  new_scale_factor);

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals = scope_output;
		}

		if (this->need_process[a_index]) {
			vector<double> combined_state_vals = local_state_vals;
			combined_state_vals.insert(combined_state_vals.end(),
				new_state_vals.begin(), new_state_vals.end());

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				// TODO: combine into branch, so that score can be compared
				vector<double> output_state_vals;
				this->explore_fold->activate(flat_vals,
											 combined_state_vals,
											 input,
											 output_state_vals,
											 predicted_score,
											 scale_factor,
											 new_scale_factor);

				a_index = this->explore_end_non_inclusive-1;	// minus by 1 for increment
			} else {
				if (this->is_branch[a_index]) {
					vector<double> output_state_vals;
					this->branches[a_index]->explore_activate(flat_vals,
															  combined_state_vals,
															  input,
															  output_state_vals,
															  predicted_score,
															  scale_factor,
															  new_scale_factor);
					local_state_vals = output_state_vals;
				} else {
					this->score_networks[a_index]->activate_small(combined_state_vals,
																  inputs);
					predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

					if (this->active_compress[a_index]) {
						// compress 2 layers, add 1 layer
						this->compress_networks[a_index]->activate_small(combined_state_vals,
																		 inputs);

						local_state_vals.clear();
						local_state_vals.reserve(this->compress_new_sizes[a_index]);
						for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
							local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
						}
					}
					// else just let new_state_vals go
				}
			}
		}

		a_index++;
	}

	this->end_input_network->activate_small(local_state_vals,
											local_s_input_vals);
	output_state_vals.reserve(this->end_input_size);
	for (int i_index = 0; i_index < this->end_input_size; i_index++) {
		output_state_vals.push_back(this->end_input_network->output->acti_vals[i_index]);
	}
}

void BranchPath::explore_backprop(vector<double> input_state_errors,
								  vector<double>& output_state_errors,
								  vector<double> s_input_errors,
								  double& predicted_score,
								  double target_val,
								  double& scale_factor,
								  double& new_average_factor_error,
								  double new_scale_factor,
								  double& new_scale_factor_error) {
	vector<double> local_state_errors;	// i.e., end_state_output_errors
	vector<double> end_s_input_output_errors;
	this->end_input_network->backprop_small_errors_with_no_weight_change(
		input_state_errors,
		local_state_errors,
		end_s_input_output_errors);
	for (int s_index = 0; s_index < (int)end_s_input_output_errors.size(); s_index++) {
		s_input_errors[s_index] += end_s_input_output_errors[s_index];
	}

	for (int a_index = (int)this->scopes.size()-1; a_index >= 0; a_index--) {
		if (this->need_process[a_index]) {
			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				this->explore_fold->backprop(local_state_errors,
											 predicted_score,
											 target_val,
											 scale_factor,
											 new_average_factor_error,
											 new_scale_factor_error);

				return;
			} else {
				if (this->is_branch[a_index]) {
					vector<double> branch_state_output_errors;
					this->branches[a_index]->explore_backprop(local_state_errors,
															  branch_state_output_errors,
															  s_input_errors,
															  predicted_score,
															  target_val,
															  scale_factor,
															  new_average_factor_error,
															  new_scale_factor,
															  new_scale_factor_error);

					if (this->explore_index_inclusive == a_index
							&& this->explore_type == EXPLORE_TYPE_BRANCH) {
						return;
					}

					local_state_errors = branch_state_output_errors;
				} else {
					if (this->active_compress[a_index]) {
						vector<double> compress_state_output_errors;
						vector<double> compress_s_input_output_errors;
						this->compress_networks[a_index]->backprop_small_errors_with_no_weight_change(
							local_state_errors,
							compress_state_output_errors,
							compress_s_input_output_errors);
						local_state_errors = compress_state_output_errors;
						// use output sizes as might not have used all inputs
						for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
							s_input_errors[s_index] += compress_s_input_output_errors[s_index];
						}
					} else {
						for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
							local_state_errors.push_back(0.0);
						}
					}

					double predicted_score_error = target_val - predicted_score;

					new_average_factor_error += predicted_score_error;

					double score_add_w_o_new_scale = scale_factor/new_scale_factor*this->score_networks[a_index]->output->acti_vals[0];
					new_scale_factor_error += score_add_w_o_new_scale*predicted_score_error;

					vector<double> score_errors{scale_factor*predicted_score_error};
					vector<double> score_state_output_errors;
					vector<double> score_s_input_output_errors;
					this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
						score_errors,
						score_state_output_errors,
						score_s_input_output_errors);
					for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
						local_state_errors[s_index] += score_state_output_errors[s_index];
					}
					// use output sizes as might not have used all inputs
					for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
						s_input_errors[s_index] += score_s_input_output_errors[s_index];
					}
				}
			}
		}

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output_errors;
			scope_scope->explore_backprop(local_state_errors,
										  scope_output_errors,
										  predicted_score,
										  target_val,
										  scale_factor,
										  new_average_factor_error,
										  new_scale_factor,
										  new_scale_factor_error);

			if (this->explore_index_inclusive == a_index) {
				// this->explore_type == EXPLORE_TYPE_SCOPE
				return;
			}

			scale_factor /= this->scope_scale_mod[a_index];

			predicted_score -= scale_factor*this->scope_average_mod[a_index];

			for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = scope_output_errors.back();
					scope_output_errors.pop_back();
				}
				vector<double> inner_input_state_output_errors;
				vector<double> inner_input_s_input_output_errors;
				this->inner_input_networks[a_index][i_index]->backprop_small_errors_with_no_weight_change(
					inner_input_errors,
					inner_input_state_output_errors,
					inner_input_s_input_output_errors);
				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
				}
				// use output sizes as might not have used all inputs
				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
					s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
				}
			}
		}
	}

	output_state_errors = local_state_errors;
}
