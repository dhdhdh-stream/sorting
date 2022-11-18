#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"

using namespace std;



void Scope::activate(vector<vector<double>>& flat_vals,
					 vector<double>& input,
					 vector<double>& output,
					 double& predicted_score,
					 double& scale_factor) {
	// start
	vector<double> local_state_vals;
	// local_state_vals will be output
	// local_s_input_vals is input

	if (this->scopes[0]->type == SCOPE_TYPE_BASE) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
	} else {
		// this->scopes[0]->type == SCOPE_TYPE_SCOPE
		Scope* scope_scope = (Scope*)this->scopes[0];

		vector<double> scope_input(scope_scope->num_inputs);
		for (int i_index = 0; i_index < scope_scope->num_inputs; i_index++) {
			scope_input[i_index] = inputs[this->start_score_input_input_indexes[i_index]];
		}

		predicted_score += this->scope_average_mod[0];

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		scope_scope->activate(flat_vals,
							  scope_input,
							  scope_output,
							  predicted_score,
							  scale_factor);

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	// this->need_process[0] == true
	if (this->is_branch[0]) {
		vector<double> output_state_vals;
		this->branches[0]->activate(local_state_vals,
									input,
									output_state_vals,
									flat_vals,
									predicted_score,
									scale_factor);
		local_state_vals = output_state_vals;
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		this->score_networks[0]->activate_small(local_state_vals,
												inputs);
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->compress_sizes[0] > 0) {
			// this->active_compress[0] == true
			// cannot be last action
			// compress 1 layer, add 1 layer
			this->compress_networks[0]->activate_small(local_state_vals,
													   inputs);

			int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[0];
			local_state_vals.clear();
			local_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
	}

	// mid
	for (int a_index = 1; a_index < (int)this->scopes.size()-1; a_index++) {
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

			predicted_score += this->scope_average_mod[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;

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

					int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[a_index];
					local_state_vals.clear();
					local_state_vals.reserve(compress_new_size);
					for (int s_index = 0; s_index < compress_new_size; s_index++) {
						local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
					}
				}
				// else just let new_state_vals go
			}
		}
	}

	// end
	if (this->scopes.size() > 1) {
		// this->need_process[this->scopes.size()-1] == true
		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[this->scopes.size()-1].size(); i_index++) {
				this->inner_input_networks[this->scopes.size()-1][i_index]->activate(local_state_vals,
																					 input);
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			predicted_score += scale_factor*this->scope_average_mod[this->scopes.size()-1];

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			scope_scope->activate(flat_vals,
								  scope_input,
								  scope_output,
								  predicted_score,
								  scale_factor);

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}

	output = local_state_vals;
}

void Scope::existing_backprop(vector<double> input_errors,
							  vector<double>& output_errors,
							  double& predicted_score,
							  double predicted_score_error,
							  double& scale_factor,
							  double new_scale_factor,
							  double& new_scale_factor_error) {
	vector<double> local_state_errors = input_errors;
	vector<double> local_s_input_errors(this->num_inputs, 0.0);

	// end
	if (this->scopes.size() > 1) {
		// this->need_process[this->scopes.size()-1] == true
		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
				local_state_errors.end());
		} else {
			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

			vector<double> scope_input_errors(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());

			// don't need to pass/backprop scope_average_mod/scope_scale_mod as not changing them
			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			scope_scope->existing_backprop(scope_input_errors,
										   scope_output_errors,
										   predicted_score,
										   predicted_score_error,
										   scale_factor,
										   new_scale_factor,
										   new_scale_factor_error);

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			predicted_score -= scale_factor*this->scope_average_mod[this->scopes.size()-1];

			for (int i_index = (int)this->inner_input_networks[this->scopes.size()-1].size()-1; i_index >= 0; i_index--) {
				vector<double> inner_input_errors(this->inner_input_sizes[this->scopes.size()-1][i_index]);
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					inner_input_errors[this->inner_input_sizes[this->scopes.size()-1][i_index]-1-s_index] = scope_output_errors.back();
					scope_output_errors.pop_back();
				}
				vector<double> inner_input_state_output_errors;
				vector<double> inner_input_s_input_output_errors;
				this->inner_input_networks[this->scopes.size()-1][i_index]->backprop_small_errors_with_no_weight_change(
					inner_input_errors,
					inner_input_state_output_errors,
					inner_input_s_input_output_errors);
				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
				}
				// use output sizes as compress_networks might not have used all inputs
				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
					local_s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
				}
			}
		}
	}

	// mid
	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
		if (this->need_process[a_index]) {
			if (this->is_branch[a_index]) {
				vector<double> branch_state_output_errors;
				this->branches[a_index]->existing_backprop(local_state_errors,
														   branch_state_output_errors,
														   local_s_input_errors,
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
					// use output sizes as compress_networks might not have used all inputs
					for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
						local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
					}
				} else {
					for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
						local_state_errors.push_back(0.0);
					}
				}

				// with only predicted_score_error, don't worry about average_factor yet

				double score_add_w_o_new_scale = 

				vector<double> score_errors{predicted_score_error};
				vector<double> score_state_output_errors;
				vector<double> score_s_input_output_errors;
				this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
					score_errors,
					score_state_output_errors,
					score_s_input_output_errors);
			}
		}
	}
}

void Scope::explore_activate(vector<vector<double>>& flat_vals,
							 vector<double>& input,
							 vector<double>& output,
							 double& predicted_score,
							 double& scale_factor,
							 double& new_scale_factor) {
	int a_index = 1;

	// start
	vector<double> local_state_vals;
	// local_state_vals will be output
	// local_s_input_vals is input

	if (this->scopes[0]->type == SCOPE_TYPE_BASE) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
	} else {
		// this->scopes[0]->type == SCOPE_TYPE_SCOPE
		Scope* scope_scope = (Scope*)this->scopes[0];

		vector<double> scope_input(scope_scope->num_inputs);
		for (int i_index = 0; i_index < scope_scope->num_inputs; i_index++) {
			scope_input[i_index] = inputs[this->start_score_input_input_indexes[i_index]];
		}

		predicted_score += new_scale_factor*this->scope_average_mod[0];

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		scope_scope->explore_activate(flat_vals,
									  scope_input,
									  scope_output,
									  predicted_score,
									  scale_factor,
									  new_scale_factor);

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	// this->need_process[0] == true
	if (this->explore_index_inclusive == 0 && this->explore_in == false) {
		vector<double> output_state_vals;
		this->explore_fold->activate(flat_vals,
									 local_state_vals,
									 input,
									 output_state_vals,
									 predicted_score,
									 scale_factor,
									 new_scale_factor);

		a_index = explore_end_non_inclusive;
	} else {
		if (this->is_branch[0]) {
			vector<double> output_state_vals;
			this->branches[0]->explore_activate(flat_vals,
												local_state_vals,
												input,
												output_state_vals,
												predicted_score,
												scale_factor,
												new_scale_factor);
			local_state_vals = output_state_vals;
			// if is also last action, extended local_state_vals will still be set correctly
		} else {
			this->score_networks[0]->activate_small(local_state_vals,
													inputs);
			predicted_score += new_scale_factor*scale_factor*this->score_networks[0]->output->acti_vals[0];

			if (this->compress_sizes[0] > 0) {
				// this->active_compress[0] == true
				// cannot be last action
				// compress 1 layer, add 1 layer
				this->compress_networks[0]->activate_small(local_state_vals,
														   inputs);

				int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[0];
				local_state_vals.clear();
				local_state_vals.reserve(compress_new_size);
				for (int s_index = 0; s_index < compress_new_size; s_index++) {
					local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
				}
			}
		}
	}

	// mid
	while (a_index < (int)this->scopes.size()-1) {
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

			predicted_score += new_scale_factor*this->scope_average_mod[a_index];

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;

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

			if (this->explore_index_inclusive == a_index && this->explore_in == false) {
				vector<double> output_state_vals;
				this->explore_fold->activate(flat_vals,
											 local_state_vals,
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
					predicted_score += new_scale_factor*scale_factor*this->score_networks[a_index]->output->acti_vals[0];

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

	// end
	if (this->scopes.size() > 1) {
		// this->need_process[this->scopes.size()-1] == true
		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[this->scopes.size()-1].size(); i_index++) {
				this->inner_input_networks[this->scopes.size()-1][i_index]->activate(local_state_vals,
																					 input);
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			predicted_score += new_scale_factor*this->scope_average_mod[this->scopes.size()-1];

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			scope_scope->explore_activate(flat_vals,
										  scope_input,
										  scope_output,
										  predicted_score,
										  scale_factor,
										  new_scale_factor);

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}

	output = local_state_vals;
}

void Scope::explore_backprop(vector<double> input_errors,
							 vector<double>& output_errors,
							 double& predicted_score,
							 double target_val,
							 double& scale_factor,
							 double& new_average_factor_error,
							 double new_scale_factor,
							 double& new_scale_factor_error) {
	vector<double> local_state_errors = input_errors;
	vector<double> local_s_input_errors(this->num_inputs, 0.0);

	// end
	if (this->scopes.size() > 1) {
		// this->need_process[this->scopes.size()-1] == true
		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
				local_state_errors.end());
		} else {
			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

			vector<double> scope_input_errors(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());

			// don't need to pass/backprop scope_average_mod/scope_scale_mod as not changing them
			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			scope_scope->explore_backprop(scope_input_errors,
										  scope_output_errors,
										  predicted_score,
										  target_val,
										  scale_factor,
										  new_average_factor_error,
										  new_scale_factor,
										  new_scale_factor_error);

			if (this->explore_index_inclusive == this->scopes.size()-1) {
				// this->explore_in == true
				return;
			}

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			predicted_score -= new_scale_factor*this->scope_average_mod[this->scopes.size()-1];

			for (int i_index = (int)this->inner_input_networks[this->scopes.size()-1].size()-1; i_index >= 0; i_index--) {
				vector<double> inner_input_errors(this->inner_input_sizes[this->scopes.size()-1][i_index]);
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					inner_input_errors[this->inner_input_sizes[this->scopes.size()-1][i_index]-1-s_index] = scope_output_errors.back();
					scope_output_errors.pop_back();
				}
				this->inner_input_networks[this->scopes.size()-1][i_index]->backprop_errors_with_no_weight_change(inner_input_errors);
				for (int s_index = 0; s_index < (int)local_state_errors.size(); s_index++) {
					local_state_errors[s_index] += this->inner_input_networks[this->scopes.size()-1][i_index]
						->state_inputs.back()->errors[s_index];
					this->inner_input_networks[this->scopes.size()-1][i_index]->state_inputs.back()->errors[s_index] = 0.0;
				}
				for (int s_index = 0; s_index < (int)local_s_input_errors.size(); s_index++) {
					local_s_input_errors[s_index] += this->inner_input_networks[this->scopes.size()-1][i_index]
						->s_input_input->errors[s_index];
					this->inner_input_networks[this->scopes.size()-1][i_index]->s_input_input->errors[s_index] = 0.0;
				}
			}
		}
	}

	// mid
	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
		if (this->need_process[a_index]) {
			if (this->explore_index_inclusive == a_index && this->explore_in == false) {
				this->explore_fold->backprop();

				return;
			}
		}
	}
}
