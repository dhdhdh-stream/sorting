#include "branch_path.h"

#include <iostream>

using namespace std;



void BranchPath::explore_on_path_activate(vector<vector<double>>& flat_vals,
										  double starting_score,		// matters when start is not branch
										  vector<double>& local_s_input_vals,
										  vector<double>& local_state_vals,	// i.e., combined initially
										  double& predicted_score,
										  double& scale_factor,
										  int& explore_phase,
										  Fold*& flat_ref,
										  BranchPathHistory* history) {
	int a_index = 1;

	// start
	if (this->is_branch[0]) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
			this->branches[0]->explore_on_path_activate_score(local_s_input_vals,
															  local_state_vals,
															  scale_factor,
															  explore_phase,
															  branch_history);
		} else {
			this->branches[0]->explore_off_path_activate_score(local_s_input_vals,
															   local_state_vals,
															   scale_factor,
															   explore_phase,
															   branch_history);
		}

		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			FoldHistory* fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->activate(branch_history->best_score,
										 flat_vals,
										 local_s_input_vals,
										 local_state_vals,
										 predicted_score,
										 scale_factor,
										 explore_phase,
										 fold_history);
			flat_ref = this->explore_fold;
			history->fold_history = fold_history;

			a_index = this->explore_end_non_inclusive;

			delete branch_history;
		} else {
			if (this->explore_index_inclusive == 0
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[0]->explore_on_path_activate(flat_vals,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															explore_phase,
															flat_ref,
															branch_history);
			} else {
				this->branches[0]->explore_off_path_activate(flat_vals,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 explore_phase,
															 branch_history);
			}
			history->branch_histories[0] = branch_history;
		}
	} else {
		// starting_score already scaled

		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			FoldHistory* fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->activate(starting_score,
										 flat_vals,
										 local_s_input_vals,
										 local_state_vals,
										 predicted_score,
										 scale_factor,
										 explore_phase,
										 fold_history);
			flat_ref = this->explore_fold;
			history->fold_history = fold_history;

			a_index = this->explore_end_non_inclusive;
		} else {
			// wait until on branch_path to update predicted_score
			history->score_updates[0] = starting_score;
			predicted_score += starting_score;

			if (this->active_compress[0]) {
				// explore_phase == EXPLORE_PHASE_NONE
				this->compress_networks[0]->activate_small(local_s_input_vals,
														   local_state_vals);
				int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[0];
				local_state_vals.clear();
				local_state_vals.reserve(compress_new_size);
				for (int s_index = 0; s_index < compress_new_size; s_index++) {
					local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals.erase(local_state_vals.end()-this->compress_sizes[0], local_state_vals.end());
			}
		}
	}

	// mid
	while (a_index < this->scopes.size()) {
		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				if (explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[a_index][i_index]);
					this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																				 local_state_vals,
																				 inner_input_network_history);
					history->inner_input_network_histories[a_index].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[a_index][i_index]->activate(local_s_input_vals,
																		   local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
			predicted_score += scale_factor*scope_average_mod_val;

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(scope_scope);
			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				scope_scope->explore_on_path_activate(flat_vals,
													  temp_new_s_input_vals,
													  scope_output,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  flat_ref,
													  scope_history);
			} else {
				scope_scope->explore_off_path_activate(flat_vals,
													   temp_new_s_input_vals,
													   scope_output,
													   predicted_score,
													   scale_factor,
													   explore_phase,
													   scope_history);
			}
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->is_branch[a_index]) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[a_index]->explore_on_path_activate_score(local_s_input_vals,
																		local_state_vals,
																		scale_factor,
																		explore_phase,
																		branch_history);
			} else {
				this->branches[a_index]->explore_off_path_activate_score(local_s_input_vals,
																		 local_state_vals,
																		 scale_factor,
																		 explore_phase,
																		 branch_history);
			}

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				FoldHistory* fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->activate(branch_history->best_score,
											 flat_vals,
											 local_s_input_vals,
											 local_state_vals,
											 predicted_score,
											 scale_factor,
											 explore_phase,
											 fold_history);
				flat_ref = this->explore_fold;
				history->fold_history = fold_history;

				a_index = this->explore_end_non_inclusive-1;	// account for increment at end

				delete branch_history;
			} else {
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_activate(flat_vals,
																	  local_s_input_vals,
																	  local_state_vals,
																	  predicted_score,
																	  scale_factor,
																	  explore_phase,
																	  flat_ref,
																	  branch_history);
				} else {
					this->branches[a_index]->explore_off_path_activate(flat_vals,
																	   local_s_input_vals,
																	   local_state_vals,
																	   predicted_score,
																	   scale_factor,
																	   explore_phase,
																	   branch_history);
				}
				history->branch_histories[a_index] = branch_history;
			}
		} else {
			FoldNetworkHistory* score_network_history = NULL;
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals,
															  score_network_history);
			} else {
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals);
			}
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				// explore_phase != EXPLORE_PHASE_FLAT, so don't need to delete score_network_history

				FoldHistory* fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->activate(existing_score,
											 flat_vals,
											 local_s_input_vals,
											 local_state_vals,
											 predicted_score,
											 scale_factor,
											 explore_phase,
											 fold_history);
				flat_ref = this->explore_fold;
				history->fold_history = fold_history;

				a_index = this->explore_end_non_inclusive-1;	// account for increment at end
			} else {
				history->score_network_histories[a_index] = score_network_history;
				history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
				predicted_score += existing_score;

				if (this->active_compress[a_index]) {
					// compress 2 layers, add 1 layer
					if (explore_phase == EXPLORE_PHASE_FLAT) {
						FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[a_index]);
						this->compress_networks[a_index]->activate_small(local_s_input_vals,
																		 local_state_vals,
																		 compress_network_history);
						history->compress_network_histories[a_index] = compress_network_history;
					} else {
						this->compress_networks[a_index]->activate_small(local_s_input_vals,
																		 local_state_vals);
					}

					int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[a_index];
					local_state_vals.clear();
					local_state_vals.reserve(compress_new_size);
					for (int s_index = 0; s_index < compress_new_size; s_index++) {
						local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
					}
				} else {
					local_state_vals.erase(local_state_vals.end()-this->compress_sizes[a_index], local_state_vals.end());
				}
			}
		}

		a_index++;
	}

	// explore_phase == EXPLORE_PHASE_FLAT
	FoldNetworkHistory* end_input_network_history = new FoldNetworkHistory(this->end_input_network);
	this->end_input_network->activate_small(local_s_input_vals,
											local_state_vals,
											end_input_network_history);
	history->end_input_network_history = end_input_network_history;
	local_state_vals.clear();
	local_state_vals.reserve(this->end_input_size);
	for (int i_index = 0; i_index < this->end_input_size; i_index++) {
		local_state_vals.push_back(this->end_input_network->output->acti_vals[i_index]);
	}
}

void BranchPath::explore_off_path_activate(vector<vector<double>>& flat_vals,
										   double starting_score,		// matters when start is not branch
										   vector<double>& local_s_input_vals,
										   vector<double>& local_state_vals,	// i.e., combined initially
										   double& predicted_score,
										   double& scale_factor,
										   int& explore_phase,
										   BranchPathHistory* history) {
	// start
	if (this->is_branch[0]) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->explore_off_path_activate_score(local_s_input_vals,
														   local_state_vals,
														   scale_factor,
														   explore_phase,
														   branch_history);

		this->branches[0]->explore_off_path_activate(flat_vals,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 explore_phase,
													 branch_history);
		history->branch_histories[0] = branch_history;
	} else {
		// starting_score already scaled

		// wait until on branch_path to update predicted_score
		history->score_updates[0] = starting_score;
		predicted_score += starting_score;

		if (this->active_compress[0]) {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[0]);
				this->compress_networks[0]->activate_small(local_s_input_vals,
														   local_state_vals,
														   compress_network_history);
				history->compress_network_histories[0] = compress_network_history;
			} else {
				this->compress_networks[0]->activate_small(local_s_input_vals,
														   local_state_vals);
			}

			int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[0];
			local_state_vals.clear();
			local_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		} else {
			local_state_vals.erase(local_state_vals.end()-this->compress_sizes[0], local_state_vals.end());
		}
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size(); a_index++) {
		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				if (explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[a_index][i_index]);
					this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																				 local_state_vals,
																				 inner_input_network_history);
					history->inner_input_network_histories[a_index].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[a_index][i_index]->activate(local_s_input_vals,
																		   local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
			predicted_score += scale_factor*scope_average_mod_val;

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(scope_scope);
			scope_scope->explore_off_path_activate(flat_vals,
												   temp_new_s_input_vals,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   explore_phase,
												   scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->is_branch[a_index]) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->explore_off_path_activate_score(local_s_input_vals,
																	 local_state_vals,
																	 scale_factor,
																	 explore_phase,
																	 branch_history);

			this->branches[a_index]->explore_off_path_activate(flat_vals,
															   local_s_input_vals,
															   local_state_vals,
															   predicted_score,
															   scale_factor,
															   explore_phase,
															   branch_history);
			history->branch_histories[a_index] = branch_history;
		} else {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals,
															  score_network_history);
				history->score_network_histories[a_index] = score_network_history;
			} else {
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals);
			}
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->active_compress[a_index]) {
				// compress 2 layers, add 1 layer
				if (explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[a_index]);
					this->compress_networks[a_index]->activate_small(local_s_input_vals,
																	 local_state_vals,
																	 compress_network_history);
					history->compress_network_histories[a_index] = compress_network_history;
				} else {
					this->compress_networks[a_index]->activate_small(local_s_input_vals,
																	 local_state_vals);
				}

				int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[a_index];
				local_state_vals.clear();
				local_state_vals.reserve(compress_new_size);
				for (int s_index = 0; s_index < compress_new_size; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals.erase(local_state_vals.end()-this->compress_sizes[a_index], local_state_vals.end());
			}
		}

		a_index++;
	}

	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* end_input_network_history = new FoldNetworkHistory(this->end_input_network);
		this->end_input_network->activate_small(local_s_input_vals,
												local_state_vals,
												end_input_network_history);
		history->end_input_network_history = end_input_network_history;
	} else {
		this->end_input_network->activate_small(local_s_input_vals,
												local_state_vals);
	}
	local_state_vals.clear();
	local_state_vals.reserve(this->end_input_size);
	for (int i_index = 0; i_index < this->end_input_size; i_index++) {
		local_state_vals.push_back(this->end_input_network->output->acti_vals[i_index]);
	}
}

void BranchPath::explore_on_path_backprop(vector<double>& local_state_errors,	// i.e., input_errors
										  double& predicted_score,
										  double target_val,
										  double& scale_factor,
										  double& average_factor_error,
										  double& scale_factor_error,
										  BranchPathHistory* history) {
	// don't need to output local_s_input_errors on path but for explore_off_path_backprop
	vector<double> local_s_input_errors(this->num_inputs, 0.0);

	vector<double> end_s_input_output_errors;
	vector<double> end_state_output_errors;
	this->end_input_network->backprop_small_errors_with_no_weight_change(
		local_state_errors,
		end_s_input_output_errors,
		end_state_output_errors,
		history->end_input_network_history);
	// don't need to output local_s_input_errors on path
	local_state_vals = end_state_output_errors;

	int a_index;
	if (this->explore_end_non_inclusive == this->scopes.size()
			&& this->explore_type == EXPLORE_TYPE_NEW) {
		a_index = this->explore_index_inclusive;
	} else {
		a_index = this->scopes.size()-1;
	}

	// mid
	while (a_index >= 1) {
		if (this->explore_index_inclusive == a_index
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			this->explore_fold->backprop(local_state_errors,
										 predicted_score,
										 target_val,
										 scale_factor,
										 average_factor_error,
										 scale_factor_error,
										 history->fold_history);

			return;
		} else {
			if (this->is_branch[a_index]) {
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_backprop(local_state_errors,
																	  predicted_score,
																	  target_val,
																	  scale_factor,
																	  average_factor_error,
																	  scale_factor_error,
																	  history->branch_histories[a_index]);

					return;
				} else {
					this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
																	   local_state_errors,
																	   predicted_score,
																	   target_val,
																	   scale_factor,
																	   average_factor_error,
																	   scale_factor_error,
																	   history->branch_histories[a_index]);
				}
			} else {
				if (this->active_compress[a_index]) {
					vector<double> compress_s_input_output_errors;
					vector<double> compress_state_output_errors;
					this->compress_networks[a_index]->backprop_small_errors_with_no_weight_change(
						local_state_errors,
						compress_s_input_output_errors,
						compress_state_output_errors,
						history->compress_network_histories[a_index]);
					// don't need to output local_s_input_errors on path
					local_state_errors = compress_state_output_errors;
				} else {
					for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
						local_state_errors.push_back(0.0);
					}
				}

				double predicted_score_error = target_val - predicted_score;

				average_factor_error += predicted_score_error;

				scale_factor_error += this->score_updates[a_index]*predicted_score_error;

				vector<double> score_errors{predicted_score_error};
				vector<double> score_s_input_output_errors;
				vector<double> score_state_output_errors;
				this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
					score_errors,
					score_s_input_output_errors,
					score_state_output_errors,
					history->score_network_histories[a_index]);
				// don't need to output local_s_input_errors on path
				for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
					local_state_errors[s_index] += score_state_output_errors[s_index];
				}

				predicted_score -= scale_factor*this->score_updates[a_index];
			}
		}

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			vector<double> scope_input_errors(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				// average_factor_error doesn't need to be scaled
				scale_factor_error /= scope_scale_mod_val;

				scope_scope->explore_on_path_backprop(scope_input_errors,
													  predicted_score,
													  target_val,
													  scale_factor,
													  average_factor_error,
													  scale_factor_error,
													  history->scope_histories[a_index]);

				return;
			} else {
				vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
				double scope_average_factor_error = 0.0;
				double scope_scale_factor_error = 0.0;
				scope_scope->explore_off_path_backprop(scope_input_errors,
													   scope_output_errors,
													   predicted_score,
													   target_val,
													   scale_factor,
													   scope_average_factor_error,
													   scope_scale_factor_error,
													   history->scope_histories[a_index]);

				average_factor_error += scope_scale_mod_val*scope_average_factor_error;
				scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

				scale_factor /= scope_scale_mod_val;

				double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
				predicted_score -= scale_factor*scope_average_mod_val;

				for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = scope_output_errors.back();
						scope_output_errors.pop_back();
					}
					vector<double> inner_input_s_input_output_errors;
					vector<double> inner_input_state_output_errors;
					this->inner_input_networks[a_index][i_index]->backprop_small_errors_with_no_weight_change(
						inner_input_errors,
						inner_input_s_input_output_errors,
						inner_input_state_output_errors,
						history->inner_input_network_histories[a_index][i_index]);
					// don't need to output local_s_input_errors on path
					for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
						local_state_errors[s_index] += inner_input_state_output_errors[s_index];
					}
				}
			}
		}

		if (this->explore_end_non_inclusive == a_index
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			a_index = this->explore_index_inclusive;
		} else {
			a_index--;
		}
	}

	// start
	if (this->explore_index_inclusive == 0
			&& this->explore_type == EXPLORE_TYPE_NEW) {
		this->explore_fold->backprop(local_state_errors,
									 predicted_score,
									 target_val,
									 scale_factor,
									 average_factor_error,
									 scale_factor_error,
									 history->fold_history);

		return;
	} else {
		if (this->is_branch[0]) {
			if (this->explore_index_inclusive == 0
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[0]->explore_on_path_backprop(local_state_errors,
															predicted_score,
															target_val,
															scale_factor,
															average_factor_error,
															scale_factor_error,
															history->branch_histories[0]);

				return;
			} else {
				this->branches[0]->explore_off_path_backprop(local_s_input_errors,
															 local_state_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 average_factor_error,
															 scale_factor_error,
															 history->branch_histories[0]);
			}
		} else {
			if (this->active_compress[0]) {
				vector<double> compress_s_input_output_errors;
				vector<double> compress_state_output_errors;
				this->compress_networks[0]->backprop_small_errors_with_no_weight_change(
					local_state_errors,
					compress_s_input_output_errors,
					compress_state_output_errors,
					history->compress_network_histories[0]);
				// don't need to output local_s_input_errors on path
				local_state_errors = compress_state_output_errors;
			} else {
				for (int c_index = 0; c_index < this->compress_sizes[0]; c_index++) {
					local_state_errors.push_back(0.0);
				}
			}

			// start step score errors handled in branch

			predicted_score -= scale_factor*this->score_updates[0];
		}
	}
}

void BranchPath::explore_off_path_backprop(vector<double>& local_state_errors,		// i.e., input_errors
										   vector<double>* local_s_input_errors,	// i.e., output_errors
										   double& predicted_score,
										   double target_val,
										   double& scale_factor,
										   double& average_factor_error,
										   double& scale_factor_error,
										   BranchPathHistory* history) {
	local_s_input_errors = vector<double>(this->num_inputs, 0.0);

	vector<double> end_s_input_output_errors;
	vector<double> end_state_output_errors;
	this->end_input_network->backprop_small_errors_with_no_weight_change(
		local_state_errors,
		end_s_input_output_errors,
		end_state_output_errors,
		history->end_input_network_history);
	// don't need to output local_s_input_errors on path
	local_state_vals = end_state_output_errors;

	// mid
	for (int a_index = this->scopes.size()-1; a_index >= 1; a_index--) {
		if (this->is_branch[a_index]) {
			this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
															   local_state_errors,
															   predicted_score,
															   target_val,
															   scale_factor,
															   average_factor_error,
															   scale_factor_error,
															   history->branch_histories[a_index]);
		} else {
			if (this->active_compress[a_index]) {
				vector<double> compress_s_input_output_errors;
				vector<double> compress_state_output_errors;
				this->compress_networks[a_index]->backprop_small_errors_with_no_weight_change(
					local_state_errors,
					compress_s_input_output_errors,
					compress_state_output_errors,
					history->compress_network_histories[a_index]);
				// use output sizes as might not have used all inputs
				for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
					local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
				}
				local_state_errors = compress_state_output_errors;
			} else {
				for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
					local_state_errors.push_back(0.0);
				}
			}

			double predicted_score_error = target_val - predicted_score;

			average_factor_error += predicted_score_error;

			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

			vector<double> score_errors{predicted_score_error};
			vector<double> score_s_input_output_errors;
			vector<double> score_state_output_errors;
			this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
				score_errors,
				score_s_input_output_errors,
				score_state_output_errors,
				history->score_network_histories[a_index]);
			// use output sizes as might not have used all inputs
			for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
				local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
			}
			for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
				local_state_errors[s_index] += score_state_output_errors[s_index];
			}

			predicted_score -= scale_factor*this->score_updates[a_index];
		}

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			vector<double> scope_input_errors(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-scope_scope->num_outputs,
				local_state_errors.end());

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			double scope_average_factor_error = 0.0;
			double scope_scale_factor_error = 0.0;
			scope_scope->explore_off_path_backprop(scope_input_errors,
												   scope_output_errors,
												   predicted_score,
												   target_val,
												   scale_factor,
												   scope_average_factor_error,
												   scope_scale_factor_error,
												   history->scope_histories[a_index]);

			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

			double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
			predicted_score -= scale_factor*scope_average_mod_val;

			for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
				vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = scope_output_errors.back();
					scope_output_errors.pop_back();
				}
				vector<double> inner_input_s_input_output_errors;
				vector<double> inner_input_state_output_errors;
				this->inner_input_networks[a_index][i_index]->backprop_small_errors_with_no_weight_change(
					inner_input_errors,
					inner_input_s_input_output_errors,
					inner_input_state_output_errors,
					history->inner_input_network_histories[a_index][i_index]);
				// use output sizes as might not have used all inputs
				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
					local_s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
				}
				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
				}
			}
		}
	}

	// start
	// *** HERE *** //
	if (this->explore_index_inclusive == 0
			&& this->explore_type == EXPLORE_TYPE_NEW) {
		this->explore_fold->backprop(local_state_errors,
									 predicted_score,
									 target_val,
									 scale_factor,
									 average_factor_error,
									 scale_factor_error,
									 history->fold_history);

		return;
	} else {
		if (this->is_branch[0]) {
			if (this->explore_index_inclusive == 0
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[0]->explore_on_path_backprop(local_state_errors,
															predicted_score,
															target_val,
															scale_factor,
															average_factor_error,
															scale_factor_error,
															history->branch_histories[0]);

				return;
			} else {
				this->branches[0]->explore_off_path_backprop(local_s_input_errors,
															 local_state_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 average_factor_error,
															 scale_factor_error,
															 history->branch_histories[0]);
			}
		} else {
			if (this->active_compress[0]) {
				vector<double> compress_s_input_output_errors;
				vector<double> compress_state_output_errors;
				this->compress_networks[0]->backprop_small_errors_with_no_weight_change(
					local_state_errors,
					compress_s_input_output_errors,
					compress_state_output_errors,
					history->compress_network_histories[0]);
				// don't need to output local_s_input_errors on path
				local_state_errors = compress_state_output_errors;
			} else {
				for (int c_index = 0; c_index < this->compress_sizes[0]; c_index++) {
					local_state_errors.push_back(0.0);
				}
			}

			// start step score errors handled in branch

			predicted_score -= scale_factor*this->score_updates[0];
		}
	}
}
