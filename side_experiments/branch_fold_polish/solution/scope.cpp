	#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"

using namespace std;



void Scope::explore_on_path_activate(vector<vector<double>>& flat_vals,
									 vector<double>& input,
									 vector<double>& output,
									 double& predicted_score,
									 double& scale_factor,
									 int& explore_phase,
									 Fold*& flat_ref,
									 ScopeHistory* history) {
	int a_index = 1;

	// start
	vector<double> local_state_vals;
	// local_state_vals will be output
	// local_s_input_vals is input

	// TODO: starting action is NO-OP
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

		double scope_average_mod_val = this->scope_average_mod[0]->output->constants[0];
		predicted_score += scale_factor*scope_average_mod_val;

		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(scope_scope);
		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
			scope_scope->explore_on_path_activate(flat_vals,
												  scope_input,
												  scope_output,
												  predicted_score,
												  scale_factor,
												  explore_phase,
												  flat_ref,
												  scope_history);
		} else {
			scope_scope->explore_off_path_activate(flat_vals,
												   scope_input,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   explore_phase,
												   scope_history);
		}
		history->scope_histories[0] = scope_history;

		scale_factor /= scope_scale_mod_val;

		local_state_vals = scope_output;
	}

	if (this->is_branch[0]) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		double existing_score;
		this->branches[0]->explore_activate_score(existing_score,
												  input,
												  local_state_vals,
												  scale_factor,
												  explore_phase,
												  branch_history);

		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			delete branch_history;

			vector<double> output_state_vals;
			FoldHistory* fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->activate(existing_score,
										 flat_vals,
										 input,
										 local_state_vals,
										 output_state_vals,
										 predicted_score,
										 scale_factor,
										 explore_phase,
										 fold_history);
			flat_ref = this->explore_fold;
			history->fold_history = fold_history;
			local_state_vals = output_state_vals;

			a_index = this->explore_end_non_inclusive;
		} else {
			vector<double> output_state_vals;
			if (this->explore_index_inclusive == 0
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[0]->explore_on_path_activate(flat_vals,
															input,
															local_state_vals,
															output_state_vals,
															predicted_score,
															scale_factor,
															explore_phase,
															flat_ref,
															branch_history);
			} else {
				this->branches[0]->explore_off_path_activate(flat_vals,
															 input,
															 local_state_vals,
															 output_state_vals,
															 predicted_score,
															 scale_factor,
															 explore_phase,
															 branch_history);
			}
			history->branch_histories[0] = branch_history;
			local_state_vals = output_state_vals;
			// if is also last action, extended local_state_vals will still be set correctly
		}
	} else {
		// explore_phase == EXPLORE_PHASE_NONE
		this->score_networks[0]->activate_small(inputs,
												local_state_vals);
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];
		
		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			vector<double> output_state_vals;
			FoldHistory* fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->activate(existing_score,
										 flat_vals,
										 input,
										 local_state_vals,
										 output_state_vals,
										 predicted_score,
										 scale_factor,
										 explore_phase,
										 fold_history);
			flat_ref = this->explore_fold;
			history->fold_history = fold_history;
			local_state_vals = output_state_vals;

			a_index = this->explore_end_non_inclusive;
		} else {
			history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
			predicted_score += existing_score;

			if (this->active_compress[0]) {
				// cannot be last action
				// compress 1 layer, add 1 layer
				// explore_phase == EXPLORE_PHASE_NONE
				this->compress_networks[0]->activate_small(inputs,
														   local_state_vals);

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
	while (a_index < this->scopes.size()-1) {
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
				if (this->explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[a_index][i_index]);
					this->inner_input_networks[a_index][i_index]->activate_small(input,
																				 local_state_vals,
																				 inner_input_network_history);
					history->inner_input_network_histories[a_index].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[a_index][i_index]->activate_small(input,
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
													  scope_input,
													  scope_output,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  flat_ref,
													  scope_history);
			} else {
				scope_scope->explore_off_path_activate(flat_vals,
													   scope_input,
													   scope_output,
													   predicted_score,
													   scale_factor,
													   explore_phase,
													   scope_history);
			}
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			local_state_vals = scope_output;
		}

		vector<double> combined_state_vals = local_state_vals;
		combined_state_vals.insert(combined_state_vals.end(),
			new_state_vals.begin(), new_state_vals.end());

		if (this->is_branch[a_index]) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			double existing_score;
			this->branches[a_index]->explore_activate_score(existing_score,
															input,
															local_state_vals,
															scale_factor,
															explore_phase,
															branch_history);

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				delete branch_history;

				vector<double> output_state_vals;
				FoldHistory* fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->activate(existing_score,
											 flat_vals,
											 input,
											 local_state_vals,
											 output_state_vals,
											 predicted_score,
											 scale_factor,
											 explore_phase,
											 fold_history);
				flat_ref = this->explore_fold;
				history->fold_history = fold_history;
				local_state_vals = output_state_vals;

				a_index = this->explore_end_non_inclusive;
			} else {
				vector<double> output_state_vals;
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_activate(flat_vals,
																	  input,
																	  local_state_vals,
																	  output_state_vals,
																	  predicted_score,
																	  scale_factor,
																	  explore_phase,
																	  flat_ref,
																	  branch_history);
				} else {
					this->branches[a_index]->explore_off_path_activate(flat_vals,
																	   input,
																	   local_state_vals,
																	   output_state_vals,
																	   predicted_score,
																	   scale_factor,
																	   explore_phase,
																	   branch_history);
				}
				history->branch_histories[a_index] = branch_history;
				local_state_vals = output_state_vals;
			}
		} else {
			FoldNetworkHistory* score_network_history = NULL;
			if (this->explore_phase == EXPLORE_PHASE_FLAT) {
				score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
				this->score_networks[a_index]->activate_small(inputs,
															  local_state_vals,
															  score_network_history);
			} else {
				this->score_networks[a_index]->activate_small(inputs,
															  local_state_vals);
			}
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				// this->explore_phase != EXPLORE_PHASE_FLAT, so don't need to delete score_network_history

				vector<double> output_state_vals;
				FoldHistory* fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->activate(existing_score,
											 flat_vals,
											 input,
											 local_state_vals,
											 output_state_vals,
											 predicted_score,
											 scale_factor,
											 explore_phase,
											 fold_history);
				flat_ref = this->explore_fold;
				history->fold_history = fold_history;
				local_state_vals = output_state_vals;

				a_index = this->explore_end_non_inclusive;
			} else {
				history->score_network_histories[a_index] = score_network_history;
				history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
				predicted_score += existing_score;

				if (this->active_compress[a_index]) {
					// compress 2 layers, add 1 layer
					if (this->explore_phase == EXPLORE_PHASE_FLAT) {
						FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[a_index]);
						this->compress_networks[a_index]->activate_small(inputs,
																		 local_state_vals,
																		 compress_network_history);
						history->compress_network_histories[a_index] = compress_network_history;
					} else {
						this->compress_networks[a_index]->activate_small(inputs,
																		 combined_state_vals);
					}

					local_state_vals.clear();
					local_state_vals.reserve(this->compress_new_sizes[a_index]);
					for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
						local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
					}
				}
				// else just let new_state_vals go
			}
		}

		a_index++;
	}

	// end
	if (this->scopes.size() > 1) {
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
				if (this->explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[this->scopes.size()-1][i_index]);
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate_small(input,
																							   local_state_vals,
																							   inner_input_network_history);
					history->inner_input_network_histories[this->scopes.size()-1].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate(input,
																						 local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
			predicted_score += scale_factor*scope_average_mod_val;

			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(scope_scope);
			if (this->explore_index_inclusive == this->scopes.size()-1
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				scope_scope->explore_on_path_activate(flat_vals,
													  scope_input,
													  scope_output,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  flat_ref,
													  scope_history);
			} else {
				scope_scope->explore_off_path_activate(flat_vals,
													   scope_input,
													   scope_output,
													   predicted_score,
													   scale_factor,
													   explore_phase,
													   scope_history);
			}
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= scope_scale_mod_val;

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}

	output = local_state_vals;
}

void Scope::explore_off_path_activate(vector<vector<double>>& flat_vals,
									  vector<double>& input,
									  vector<double>& output,
									  double& predicted_score,
									  double& scale_factor,
									  int& explore_phase,
									  ScopeHistory* history) {
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

		double scope_average_mod_val = this->scope_average_mod[0]->output->constants[0];
		predicted_score += scale_factor*scope_average_mod_val;

		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(scope_scope);
		scope_scope->explore_off_path_activate(flat_vals,
											   scope_input,
											   scope_output,
											   predicted_score,
											   scale_factor,
											   explore_phase,
											   scope_history);
		history->scope_histories[0] = scope_history;

		scale_factor /= scope_scale_mod_val;

		local_state_vals = scope_output;
	}

	if (this->is_branch[0]) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		vector<double> output_state_vals;
		this->branches[0]->explore_off_path_activate(flat_vals,
													 input,
													 local_state_vals,
													 output_state_vals,
													 predicted_score,
													 scale_factor,
													 explore_phase,
													 branch_history);
		history->branch_histories[0] = branch_history;
		local_state_vals = output_state_vals;
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		if (explore_phase == EXPLORE_PHASE_FLAT) {
			// don't adjust score networks on EXPLORE_PHASE_NONE off path
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
			this->score_networks[0]->activate_small(inputs,
													local_state_vals,
													score_network_history);
			history->score_network_histories[0] = score_network_history;
		} else {
			// if explore_phase == EXPLORE_PHASE_NONE, only have to save score for off path
			this->score_networks[0]->activate_small(inputs,
													local_state_vals);
		}
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->active_compress[0]) {
			// cannot be last action
			// compress 1 layer, add 1 layer
			if (this->explore_phase == EXPLORE_PHASE_FLAT) {
				FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[0]);
				this->compress_networks[0]->activate_small(inputs,
														   local_state_vals);
				history->compress_network_histories[0] = compress_network_history;
			} else {
				this->compress_networks[0]->activate_small(inputs,
														   local_state_vals);
			}

			int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[0];
			local_state_vals.clear();
			local_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size()-1; a_index++) {
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
				if (this->explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[a_index][i_index]);
					this->inner_input_networks[a_index][i_index]->activate_small(input,
																				 local_state_vals,
																				 inner_input_network_history);
					history->inner_input_network_histories[a_index].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[a_index][i_index]->activate_small(input,
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
												   scope_input,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   explore_phase,
												   scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			local_state_vals = scope_output;
		}

		vector<double> combined_state_vals = local_state_vals;
		combined_state_vals.insert(combined_state_vals.end(),
			new_state_vals.begin(), new_state_vals.end());

		if (this->is_branch[a_index]) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			vector<double> output_state_vals;
			this->branches[a_index]->explore_off_path_activate(flat_vals,
															   input,
															   local_state_vals,
															   output_state_vals,
															   predicted_score,
															   scale_factor,
															   explore_phase,
															   branch_history);
			history->branch_histories[a_index] = branch_history;
			local_state_vals = output_state_vals;
		} else {
			if (explore_phase == EXPLORE_PHASE_FLAT) {
				// don't adjust score networks on EXPLORE_PHASE_NONE off path
				FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
				this->score_networks[a_index]->activate_small(inputs,
															  local_state_vals,
															  score_network_history);
				history->score_network_histories[a_index] = score_network_history;
			} else {
				// if explore_phase == EXPLORE_PHASE_NONE, only have to save score for off path
				this->score_networks[a_index]->activate_small(inputs,
															  combined_state_vals);
			}
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->active_compress[a_index]) {
				// compress 2 layers, add 1 layer
				if (this->explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[a_index]);
					this->compress_networks[a_index]->activate_small(inputs,
																	 local_state_vals,
																	 compress_network_history);
					history->compress_network_histories[a_index] = compress_network_history;
				} else {
					this->compress_networks[a_index]->activate_small(inputs,
																	 combined_state_vals);
				}

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[a_index]);
				for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			}
			// else just let new_state_vals go
		}
	}

	// end
	if (this->scopes.size() > 1) {
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
				if (this->explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[this->scopes.size()-1][i_index]);
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate_small(input,
																							   local_state_vals,
																							   inner_input_network_history);
					history->inner_input_network_histories[this->scopes.size()-1].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate(input,
																						 local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
			predicted_score += scale_factor*scope_average_mod_val;

			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(scope_scope);
			scope_scope->explore_off_path_activate(flat_vals,
												   scope_input,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   explore_phase,
												   scope_history);
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= scope_scale_mod_val;

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
 
	output = local_state_vals;
}

void Scope::explore_on_path_backprop(vector<double> input_errors,
									 double& predicted_score,
									 double target_val,
									 double& scale_factor,
									 double& average_factor_error,
									 double& scale_factor_error,
									 int& explore_phase,
									 ScopeHistory* history) {
	int a_index;
	if (this->explore_end_non_inclusive == this->scopes.size()
			&& this->explore_type == EXPLORE_TYPE_NEW) {
		a_index = this->explore_index_inclusive;
	} else {
		a_index = this->scopes.size()-1;
	}

	// don't need to output local_s_input_errors on path but for existing_full_backprop
	vector<double> local_s_input_errors(this->num_inputs, 0.0);
	vector<double> local_state_errors = input_errors;

	// end
	if (this->scopes.size() > 1 && a_index == this->scopes.size()-1) {
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

			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			if (this->explore_index_inclusive == this->scopes.size()-1
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				// on_path doesn't need scope_output_errors
				
				// average_factor_error doesn't need to be scaled
				scale_factor_error /= scope_scale_mod_val;

				scope_scope->explore_on_path_backprop(scope_input_errors,
													  predicted_score,
													  target_val,
													  scale_factor,
													  average_factor_error,
													  scale_factor_error,
													  history->scope_histories[this->scopes.size()-1]);

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
													   history->scope_histories[this->scopes.size()-1]);

				// mods are a helper for initial flat, so no need to modify after

				average_factor_error += scope_scale_mod_val*scope_average_factor_error;
				scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

				scale_factor /= scope_scale_mod_val;

				double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
				predicted_score -= scale_factor*scope_average_mod_val;

				for (int i_index = (int)this->inner_input_networks[this->scopes.size()-1].size()-1; i_index >= 0; i_index--) {
					vector<double> inner_input_errors(this->inner_input_sizes[this->scopes.size()-1][i_index]);
					for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
						inner_input_errors[this->inner_input_sizes[this->scopes.size()-1][i_index]-1-s_index] = scope_output_errors.back();
						scope_output_errors.pop_back();
					}
					vector<double> inner_input_s_input_output_errors;
					vector<double> inner_input_state_output_errors;
					this->inner_input_networks[this->scopes.size()-1][i_index]->backprop_small_errors_with_no_weight_change(
						inner_input_errors,
						inner_input_s_input_output_errors,
						inner_input_state_output_errors,
						history->inner_input_network_histories[this->scopes.size()-1][i_index]);
					// don't need to output local_s_input_errors on path
					for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
						local_state_errors[s_index] += inner_input_state_output_errors[s_index];
					}
				}
			}
		}

		if (this->explore_end_non_inclusive == this->scopes.size()-1
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			a_index = this->explore_index_inclusive;
		} else {
			a_index--;
		}
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
					vector<double> branch_state_output_errors;
					this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
																	   local_state_errors,
																	   branch_state_output_errors,
																	   predicted_score,
																	   target_val,
																	   scale_factor,
																	   average_factor_error,
																	   scale_factor_error,
																	   history->branch_histories[a_index]);
					local_state_errors = branch_state_output_errors;
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

				// scale_factor actually has no impact as it's a constant, but adding is proper gradient
				vector<double> score_errors{scale_factor*predicted_score_error};
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
			}
		}

		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
			Scope* scope_scope = (Scope*)this->scopes[a_index];

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				// average_factor_error doesn't need to be scaled
				scale_factor_error /= scope_scale_mod_val;

				scope_scope->explore_on_path_backprop(local_state_errors,
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
				scope_scope->explore_off_path_backprop(local_state_errors,
													   scope_output_errors,
													   predicted_score,
													   target_val,
													   scale_factor,
													   scope_average_factor_error,
													   scope_scale_factor_error,
													   history->scope_histories[a_index]);

				// mods are a helper for initial flat, so no need to modify after

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
				vector<double> branch_state_output_errors;
				this->branches[0]->explore_off_path_backprop(local_s_input_errors,
															 local_state_errors,
															 branch_state_output_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 average_factor_error,
															 scale_factor_error,
															 history->branch_histories[0]);
				local_state_errors = branch_state_output_errors;
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

			double predicted_score_error = target_val - predicted_score;

			average_factor_error += predicted_score_error;

			scale_factor_error += this->score_updates[0]*predicted_score_error;

			// scale_factor actually has no impact as it's a constant, but adding is proper gradient
			vector<double> score_errors{scale_factor*predicted_score_error};
			vector<double> score_s_input_output_errors;
			vector<double> score_state_output_errors;
			this->score_networks[0]->backprop_small_errors_with_no_weight_change(
				score_errors,
				score_s_input_output_errors,
				score_state_output_errors,
				history->score_network_histories[0]);
			// don't need to output local_s_input_errors on path
			for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
				local_state_errors[s_index] += score_state_output_errors[s_index];
			}
		}
	}

	if (this->scopes[0]->type == SCOPE_TYPE_BASE) {
		// do nothing
	} else {
		// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
		Scope* scope_scope = (Scope*)this->scopes[0];

		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		// this->explore_index_inclusive == 0 && this->explore_type == EXPLORE_TYPE_INNER_SCOPE

		// average_factor_error doesn't need to be scaled
		scale_factor_error /= scope_scale_mod_val;

		scope_scope->explore_on_path_backprop(local_state_errors,
											  predicted_score,
											  target_val,
											  scale_factor,
											  average_factor_error,
											  scale_factor_error,
											  history->scope_histories[0]);

		return;
	}
}

// void Scope::existing_full_backprop(vector<double> input_errors,
// 								   vector<double>& output_errors,
// 								   double& predicted_score,
// 								   double target_val,
// 								   double& scale_factor,
// 								   double& average_factor_error,
// 								   double& scale_factor_error,
// 								   ScopeHistory* history) {
// 	vector<double> local_s_input_errors(this->num_inputs, 0.0);	// i.e., output_errors
// 	vector<double> local_state_errors = input_errors;

// 	// end
// 	if (this->scopes.size() > 1) {
// 		// this->need_process[this->scopes.size()-1] == true
// 		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
// 			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
// 				local_state_errors.end());
// 		} else {
// 			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
// 			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

// 			vector<double> scope_input_errors(local_state_errors.end()-scope_scope->num_outputs,
// 				local_state_errors.end());
// 			local_state_errors.erase(local_state_errors.end()-scope_scope->num_outputs,
// 				local_state_errors.end());

// 			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
// 			scale_factor *= scope_scale_mod_val;

// 			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
// 			double scope_average_factor_error = 0.0;
// 			double scope_scale_factor_error = 0.0;
// 			scope_scope->existing_full_backprop(scope_input_errors,
// 												scope_output_errors,
// 												predicted_score,
// 												target_val,
// 												scale_factor,
// 												scope_average_factor_error,
// 												scope_scale_factor_error,
// 												history->scope_histories[this->scopes.size()-1]);

// 			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 			scale_factor /= scope_scale_mod_val;

// 			double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
// 			predicted_score -= scale_factor*scope_average_mod_val;

// 			for (int i_index = (int)this->inner_input_networks[this->scopes.size()-1].size()-1; i_index >= 0; i_index--) {
// 				vector<double> inner_input_errors(this->inner_input_sizes[this->scopes.size()-1][i_index]);
// 				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
// 					inner_input_errors[this->inner_input_sizes[this->scopes.size()-1][i_index]-1-s_index] = scope_output_errors.back();
// 					scope_output_errors.pop_back();
// 				}
// 				vector<double> inner_input_s_input_output_errors;
// 				vector<double> inner_input_state_output_errors;
// 				this->inner_input_networks[this->scopes.size()-1][i_index]->backprop_small_errors_with_no_weight_change(
// 					inner_input_errors,
// 					inner_input_s_input_output_errors,
// 					inner_input_state_output_errors,
// 					history->inner_input_network_histories[this->scopes.size()-1][i_index]);
// 				// use output sizes as might not have used all inputs
// 				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
// 					local_s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
// 				}
// 				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
// 					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
// 				}
// 			}
// 		}
// 	}

// 	// mid
// 	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
// 		if (this->is_branch[a_index]) {
// 			vector<double> branch_state_output_errors;
// 			this->branches[a_index]->existing_full_backprop(local_s_input_errors,
// 															local_state_errors,
// 															branch_state_output_errors,
// 															predicted_score,
// 															target_val,
// 															scale_factor,
// 															average_factor_error,
// 															scale_factor_error,
// 															history->branch_histories[a_index]);
// 			local_state_errors = branch_state_output_errors;

// 			double branch_scale_mod_val = this->branch_scale_mod[a_index]->output->constants[0];
// 			scale_factor /= branch_scale_mod_val;

// 			double branch_average_mod_val = this->branch_average_mod[a_index]->output->constants[0];
// 			predicted_score -= scale_factor*branch_average_mod_val;

// 			average_factor_error *= branch_scale_mod_val;
// 			average_scale_error *= branch_scale_mod_val;
// 		} else {
// 			if (this->active_compress[a_index]) {
// 				vector<double> compress_s_input_output_errors;
// 				vector<double> compress_state_output_errors;
// 				this->compress_networks[a_index]->backprop_small_errors_with_no_weight_change(
// 					local_state_errors,
// 					compress_s_input_output_errors,
// 					compress_state_output_errors,
// 					history->compress_network_histories[a_index]);
// 				// use output sizes as might not have used all inputs
// 				for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
// 					local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
// 				}
// 				local_state_errors = compress_state_output_errors;
// 			} else {
// 				for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
// 					local_state_errors.push_back(0.0);
// 				}
// 			}

// 			double predicted_score_error = target_val - predicted_score;

// 			average_factor_error += predicted_score_error;

// 			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

// 			vector<double> score_errors{scale_factor*predicted_score_error};
// 			vector<double> score_s_input_output_errors;
// 			vector<double> score_state_output_errors;
// 			this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
// 				score_errors,
// 				score_s_input_output_errors,
// 				score_state_output_errors,
// 				history->score_network_histories[a_index]);
// 			// use output sizes as might not have used all inputs
// 			for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
// 				local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
// 			}
// 			for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
// 				local_state_errors[s_index] += score_state_output_errors[s_index];
// 			}
// 		}

// 		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
// 			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
// 				local_state_errors.end());
// 		} else {
// 			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
// 			Scope* scope_scope = (Scope*)this->scopes[a_index];

// 			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
// 			scale_factor *= scope_scale_mod_val;

// 			vector<double> scope_output_errors;
// 			double scope_average_factor_error = 0.0;
// 			double scope_scale_factor_error = 0.0;
// 			scope_scope->existing_full_backprop(local_state_errors,
// 												scope_output_errors,
// 												predicted_score,
// 												target_val,
// 												scale_factor,
// 												scope_average_factor_error,
// 												scope_scale_factor_error,
// 												history->scope_histories[a_index]);

// 			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 			scale_factor /= scope_scale_mod_val;

// 			double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
// 			predicted_score -= scale_factor*scope_average_mod_val;

// 			for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 				vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
// 				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 					inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = scope_output_errors.back();
// 					scope_output_errors.pop_back();
// 				}
// 				vector<double> inner_input_s_input_output_errors;
// 				vector<double> inner_input_state_output_errors;
// 				this->inner_input_networks[a_index][i_index]->backprop_small_errors_with_no_weight_change(
// 					inner_input_errors,
// 					inner_input_s_input_output_errors,
// 					inner_input_state_output_errors,
// 					history->inner_input_network_histories[a_index][i_index]);
// 				// use output sizes as might not have used all inputs
// 				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
// 					local_s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
// 				}
// 				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
// 					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
// 				}
// 			}
// 		}
// 	}

// 	// start
// 	if (this->is_branch[0]) {
// 		vector<double> branch_state_output_errors;
// 		this->branches[0]->existing_full_backprop(local_s_input_errors,
// 												  local_state_errors,
// 												  branch_state_output_errors,
// 												  predicted_score,
// 												  target_val,
// 												  scale_factor,
// 												  average_factor_error,
// 												  scale_factor_error,
// 												  history->branch_histories[0]);
// 		local_state_errors = branch_state_output_errors;

// 		double branch_scale_mod_val = this->branch_scale_mod[0]->output->constants[0];
// 		scale_factor /= branch_scale_mod_val;

// 		double branch_average_mod_val = this->branch_average_mod[0]->output->constants[0];
// 		predicted_score -= scale_factor*branch_average_mod_val;

// 		average_factor_error *= branch_scale_mod_val;
// 		average_scale_error *= branch_scale_mod_val;
// 	} else {
// 		if (this->active_compress[0]) {
// 			vector<double> compress_s_input_output_errors;
// 			vector<double> compress_state_output_errors;
// 			this->compress_networks[0]->backprop_small_errors_with_no_weight_change(
// 				local_state_errors,
// 				compress_s_input_output_errors,
// 				compress_state_output_errors,
// 				history->compress_network_histories[0]);
// 			// use output sizes as might not have used all inputs
// 			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
// 				local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
// 			}
// 			local_state_errors = compress_state_output_errors;
// 		}
// 		// else cannot be compress

// 		double predicted_score_error = target_val - predicted_score;

// 		average_factor_error += predicted_score_error;

// 		scale_factor_error += this->score_updates[0]*predicted_score_error;

// 		vector<double> score_errors{scale_factor*predicted_score_error};
// 		vector<double> score_s_input_output_errors;
// 		vector<double> score_state_output_errors;
// 		this->score_networks[0]->backprop_small_errors_with_no_weight_change(
// 			score_errors,
// 			score_s_input_output_errors,
// 			score_state_output_errors,
// 			history->score_network_histories[0]);
// 		// use output sizes as might not have used all inputs
// 		for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
// 			local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
// 		}
// 		for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
// 			local_state_errors[s_index] += score_state_output_errors[s_index];
// 		}
// 	}

// 	if (this->scopes[0]->type == SCOPE_TYPE_BASE) {
// 		// do nothing
// 	} else {
// 		// this->scopes[0]->type == SCOPE_TYPE_SCOPE
// 		Scope* scope_scope = (Scope*)this->scopes[0];

// 		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
// 		scale_factor *= scope_scale_mod_val;

// 		vector<double> scope_output_errors;
// 		double scope_average_factor_error = 0.0;
// 		double scope_scale_factor_error = 0.0;
// 		scope_scope->existing_full_backprop(local_state_errors,
// 											scope_output_errors,
// 											predicted_score,
// 											target_val,
// 											scale_factor,
// 											scope_average_factor_error,
// 											scope_scale_factor_error,
// 											history->scope_histories[0]);

// 		average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 		scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 		scale_factor /= scope_scale_mod_val;

// 		double scope_average_mod_val = this->scope_average_mod[0]->output->constants[0];
// 		predicted_score -= scale_factor*scope_average_mod_val;

// 		for (int i_index = 0; i_index < scope_scope->num_inputs; i_index++) {
// 			local_s_input_errors[this->start_score_input_input_indexes[i_index]] += scope_output_errors[i_index];
// 		}
// 	}

// 	output_errors = local_s_input_errors;
// }

// void Scope::existing_score_backprop(double& predicted_score,
// 									double target_val,
// 									double& scale_factor,
// 									double& average_factor_error,
// 									double& scale_factor_error,
// 									ScopeHistory* history) {
// 	// end
// 	if (this->scopes.size() > 1) {
// 		// this->need_process[this->scopes.size()-1] == true
// 		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
// 			// do nothing
// 		} else {
// 			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
// 			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

// 			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
// 			scale_factor *= scope_scale_mod_val;

// 			double scope_average_factor_error = 0.0;
// 			double scope_scale_factor_error = 0.0;
// 			scope_scope->existing_score_backprop(predicted_score,
// 												 target_val,
// 												 scale_factor,
// 												 scope_average_factor_error,
// 												 scope_scale_factor_error,
// 												 history->scope_histories[this->scopes.size()-1]);

// 			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 			scale_factor /= scope_scale_mod_val;

// 			double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
// 			predicted_score -= scale_factor*scope_average_mod_val;
// 		}
// 	}

// 	// mid
// 	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
// 		if (this->is_branch[a_index]) {
// 			this->branches[a_index]->existing_score_backprop(predicted_score,
// 															 target_val,
// 															 scale_factor,
// 															 average_factor_error,
// 															 scale_factor_error,
// 															 history->branch_histories[a_index]);

// 			double branch_scale_mod_val = this->branch_scale_mod[a_index]->output->constants[0];
// 			scale_factor /= branch_scale_mod_val;

// 			double branch_average_mod_val = this->branch_average_mod[a_index]->output->constants[0];
// 			predicted_score -= scale_factor*branch_average_mod_val;

// 			average_factor_error *= branch_scale_mod_val;
// 			average_scale_error *= branch_scale_mod_val;
// 		} else {
// 			double predicted_score_error = target_val - predicted_score;

// 			average_factor_error += predicted_score_error;

// 			scale_factor_error += this->score_updates[a_index]*predicted_score_error;
// 		}

// 		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
// 			// do nothing
// 		} else {
// 			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
// 			Scope* scope_scope = (Scope*)this->scopes[a_index];

// 			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
// 			scale_factor *= scope_scale_mod_val;

// 			double scope_average_factor_error = 0.0;
// 			double scope_scale_factor_error = 0.0;
// 			scope_scope->existing_score_backprop(predicted_score,
// 												 target_val,
// 												 scale_factor,
// 												 scope_average_factor_error,
// 												 scope_scale_factor_error,
// 												 history->scope_histories[a_index]);

// 			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 			scale_factor /= scope_scale_mod_val;

// 			double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
// 			predicted_score -= scale_factor*scope_average_mod_val;
// 		}
// 	}

// 	// start
// 	if (this->is_branch[0]) {
// 		this->branches[0]->existing_score_backprop(predicted_score,
// 												   target_val,
// 												   scale_factor,
// 												   average_factor_error,
// 												   scale_factor_error,
// 												   history->branch_histories[0]);

// 		double branch_scale_mod_val = this->branch_scale_mod[0]->output->constants[0];
// 		scale_factor /= branch_scale_mod_val;

// 		double branch_average_mod_val = this->branch_average_mod[0]->output->constants[0];
// 		predicted_score -= scale_factor*branch_average_mod_val;

// 		average_factor_error *= branch_scale_mod_val;
// 		average_scale_error *= branch_scale_mod_val;
// 	} else {
// 		double predicted_score_error = target_val - predicted_score;

// 		average_factor_error += predicted_score_error;

// 		scale_factor_error += this->score_updates[0]*predicted_score_error;
// 	}

// 	if (this->scopes[0]->type == SCOPE_TYPE_BASE) {
// 		// do nothing
// 	} else {
// 		// this->scopes[0]->type == SCOPE_TYPE_SCOPE
// 		Scope* scope_scope = (Scope*)this->scopes[0];

// 		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
// 		scale_factor *= scope_scale_mod_val;

// 		double scope_average_factor_error = 0.0;
// 		double scope_scale_factor_error = 0.0;
// 		scope_scope->existing_score_backprop(predicted_score,
// 											 target_val,
// 											 scale_factor,
// 											 scope_average_factor_error,
// 											 scope_scale_factor_error,
// 											 history->scope_histories[0]);

// 		average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 		scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 		scale_factor /= scope_scale_mod_val;

// 		double scope_average_mod_val = this->scope_average_mod[0]->output->constants[0];
// 		predicted_score -= scale_factor*scope_average_mod_val;
// 	}
// }

void Scope::update_activate(vector<vector<double>>& flat_vals,
							vector<double>& input,
							vector<double>& output,
							double& predicted_score,
							double& scale_factor,
							ScopeHistory* history) {
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

		double scope_average_mod_val = this->scope_average_mod[0]->output->constants[0];
		predicted_score += scale_factor*scope_average_mod_val;

		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
		scale_factor *= scope_scale_mod_val;

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(scope_scope);
		scope_scope->update_activate(flat_vals,
									 scope_input,
									 scope_output,
									 predicted_score,
									 scale_factor,
									 scope_history);
		history->scope_histories[0] = scope_history;

		scale_factor /= scope_scale_mod_val;

		local_state_vals = scope_output;
	}

	if (this->is_branch[0]) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		vector<double> output_state_vals;
		this->branches[0]->update_activate(flat_vals,
										   input,
										   local_state_vals,
										   output_state_vals,
										   predicted_score,
										   scale_factor,
										   branch_history);
		history->branch_histories[0] = branch_history;
		local_state_vals = output_state_vals;
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		// don't adjust score networks on EXPLORE_PHASE_NONE off path
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
		this->score_networks[0]->activate_small(inputs,
												local_state_vals,
												score_network_history);
		history->score_network_histories[0] = score_network_history;
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->active_compress[0]) {
			// cannot be last action
			// compress 1 layer, add 1 layer
			this->compress_networks[0]->activate_small(inputs,
													   local_state_vals);

			int compress_new_size = (int)local_state_vals.size() - this->compress_sizes[0];
			local_state_vals.clear();
			local_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size()-1; a_index++) {
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
				this->inner_input_networks[a_index][i_index]->activate_small(input,
																			 local_state_vals);
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
			scope_scope->update_activate(flat_vals,
										 scope_input,
										 scope_output,
										 predicted_score,
										 scale_factor,
										 scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			local_state_vals = scope_output;
		}

		vector<double> combined_state_vals = local_state_vals;
		combined_state_vals.insert(combined_state_vals.end(),
			new_state_vals.begin(), new_state_vals.end());

		if (this->is_branch[a_index]) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			vector<double> output_state_vals;
			this->branches[a_index]->update_activate(flat_vals,
													 input,
													 local_state_vals,
													 output_state_vals,
													 predicted_score,
													 scale_factor,
													 branch_history);
			history->branch_histories[a_index] = branch_history;
			local_state_vals = output_state_vals;
		} else {
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
			this->score_networks[a_index]->activate_small(inputs,
														  local_state_vals,
														  score_network_history);
			history->score_network_histories[a_index] = score_network_history;
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->active_compress[a_index]) {
				// compress 2 layers, add 1 layer
				this->compress_networks[a_index]->activate_small(inputs,
																 combined_state_vals);

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[a_index]);
				for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			}
			// else just let new_state_vals go
		}
	}

	// end
	if (this->scopes.size() > 1) {
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
				this->inner_input_networks[this->scopes.size()-1][i_index]->activate(input,
																					 local_state_vals);
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
			predicted_score += scale_factor*scope_average_mod_val;

			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(scope_scope);
			scope_scope->update_activate(flat_vals,
										 scope_input,
										 scope_output,
										 predicted_score,
										 scale_factor,
										 scope_history);
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= scope_scale_mod_val;

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
 
	output = local_state_vals;
}

// void Scope::existing_flat_backprop(vector<double> input_errors,
// 								   vector<double>& output_errors,
// 								   double& predicted_score,
// 								   double predicted_score_error,
// 								   double& scale_factor,
// 								   double& average_factor_error,
// 								   double& scale_factor_error,
// 								   ScopeHistory* history) {
// 	vector<double> local_s_input_errors(this->num_inputs, 0.0);	// i.e., output_errors
// 	vector<double> local_state_errors = input_errors;

// 	// end
// 	if (this->scopes.size() > 1) {
// 		// this->need_process[this->scopes.size()-1] == true
// 		if (this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_BASE) {
// 			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
// 				local_state_errors.end());
// 		} else {
// 			// this->scopes[this->scopes.size()-1]->type == SCOPE_TYPE_SCOPE
// 			Scope* scope_scope = (Scope*)this->scopes[this->scopes.size()-1];

// 			vector<double> scope_input_errors(local_state_errors.end()-scope_scope->num_outputs,
// 				local_state_errors.end());
// 			local_state_errors.erase(local_state_errors.end()-scope_scope->num_outputs,
// 				local_state_errors.end());

// 			double scope_scale_mod_val = this->scope_scale_mod[this->scopes.size()-1]->output->constants[0];
// 			scale_factor *= scope_scale_mod_val;

// 			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
// 			double scope_average_factor_error = 0.0;
// 			double scope_scale_factor_error = 0.0;
// 			scope_scope->existing_flat_backprop(scope_input_errors,
// 												scope_output_errors,
// 												predicted_score,
// 												predicted_score_error,
// 												scale_factor,
// 												scope_average_factor_error,
// 												scope_scale_factor_error,
// 												history->scope_histories[this->scopes.size()-1]);

// 			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 			scale_factor /= scope_scale_mod_val;

// 			double scope_average_mod_val = this->scope_average_mod[this->scopes.size()-1]->output->constants[0];
// 			predicted_score -= scale_factor*scope_average_mod_val;

// 			for (int i_index = (int)this->inner_input_networks[this->scopes.size()-1].size()-1; i_index >= 0; i_index--) {
// 				vector<double> inner_input_errors(this->inner_input_sizes[this->scopes.size()-1][i_index]);
// 				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
// 					inner_input_errors[this->inner_input_sizes[this->scopes.size()-1][i_index]-1-s_index] = scope_output_errors.back();
// 					scope_output_errors.pop_back();
// 				}
// 				vector<double> inner_input_s_input_output_errors;
// 				vector<double> inner_input_state_output_errors;
// 				this->inner_input_networks[this->scopes.size()-1][i_index]->backprop_small_errors_with_no_weight_change(
// 					inner_input_errors,
// 					inner_input_s_input_output_errors,
// 					inner_input_state_output_errors,
// 					history->inner_input_network_histories[this->scopes.size()-1][i_index]);
// 				// use output sizes as might not have used all inputs
// 				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
// 					local_s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
// 				}
// 				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
// 					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
// 				}
// 			}
// 		}
// 	}

// 	// mid
// 	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
// 		if (this->is_branch[a_index]) {
// 			vector<double> branch_state_output_errors;
// 			this->branches[a_index]->existing_flat_backprop(local_s_input_errors,
// 															local_state_errors,
// 															branch_state_output_errors,
// 															predicted_score,
// 															predicted_score_error,
// 															scale_factor,
// 															average_factor_error,
// 															scale_factor_error,
// 															history->branch_histories[a_index]);
// 			local_state_errors = branch_state_output_errors;

// 			// don't modify mods within existing

// 			double branch_scale_mod_val = this->branch_scale_mod[a_index]->output->constants[0];
// 			scale_factor /= branch_scale_mod_val;

// 			double branch_average_mod_val = this->branch_average_mod[a_index]->output->constants[0];
// 			predicted_score -= scale_factor*branch_average_mod_val;

// 			average_factor_error *= branch_scale_mod_val;
// 			average_scale_error *= branch_scale_mod_val;
// 		} else {
// 			if (this->active_compress[a_index]) {
// 				vector<double> compress_s_input_output_errors;
// 				vector<double> compress_state_output_errors;
// 				this->compress_networks[a_index]->backprop_small_errors_with_no_weight_change(
// 					local_state_errors,
// 					compress_s_input_output_errors,
// 					compress_state_output_errors,
// 					history->compress_network_histories[a_index]);
// 				// use output sizes as might not have used all inputs
// 				for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
// 					local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
// 				}
// 				local_state_errors = compress_state_output_errors;
// 			} else {
// 				for (int c_index = 0; c_index < this->compress_sizes[a_index]; c_index++) {
// 					local_state_errors.push_back(0.0);
// 				}
// 			}

// 			average_factor_error += predicted_score_error;

// 			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

// 			vector<double> score_errors{scale_factor*predicted_score_error};
// 			vector<double> score_s_input_output_errors;
// 			vector<double> score_state_output_errors;
// 			this->score_networks[a_index]->backprop_small_errors_with_no_weight_change(
// 				score_errors,
// 				score_s_input_output_errors,
// 				score_state_output_errors,
// 				history->score_network_histories[a_index]);
// 			// use output sizes as might not have used all inputs
// 			for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
// 				local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
// 			}
// 			for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
// 				local_state_errors[s_index] += score_state_output_errors[s_index];
// 			}
// 		}

// 		if (this->scopes[a_index]->type == SCOPE_TYPE_BASE) {
// 			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
// 				local_state_errors.end());
// 		} else {
// 			// this->scopes[a_index]->type == SCOPE_TYPE_SCOPE
// 			Scope* scope_scope = (Scope*)this->scopes[a_index];

// 			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
// 			scale_factor *= scope_scale_mod_val;

// 			vector<double> scope_output_errors;
// 			double scope_average_factor_error = 0.0;
// 			double scope_scale_factor_error = 0.0;
// 			scope_scope->existing_flat_backprop(local_state_errors,
// 												scope_output_errors,
// 												predicted_score,
// 												predicted_score_error,
// 												scale_factor,
// 												scope_average_factor_error,
// 												scope_scale_factor_error,
// 												history->scope_histories[a_index]);

// 			// don't modify mods within existing

// 			average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 			scale_factor /= scope_scale_mod_val;

// 			double scope_average_mod_val = this->scope_average_mod[a_index]->output->constants[0];
// 			predicted_score -= scale_factor*scope_average_mod_val;

// 			for (int i_index = (int)this->inner_input_networks[a_index].size()-1; i_index >= 0; i_index--) {
// 				vector<double> inner_input_errors(this->inner_input_sizes[a_index][i_index]);
// 				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
// 					inner_input_errors[this->inner_input_sizes[a_index][i_index]-1-s_index] = scope_output_errors.back();
// 					scope_output_errors.pop_back();
// 				}
// 				vector<double> inner_input_s_input_output_errors;
// 				vector<double> inner_input_state_output_errors;
// 				this->inner_input_networks[a_index][i_index]->backprop_small_errors_with_no_weight_change(
// 					inner_input_errors,
// 					inner_input_s_input_output_errors,
// 					inner_input_state_output_errors,
// 					history->inner_input_network_histories[a_index][i_index]);
// 				// use output sizes as might not have used all inputs
// 				for (int s_index = 0; s_index < (int)inner_input_s_input_output_errors.size(); s_index++) {
// 					local_s_input_errors[s_index] += inner_input_s_input_output_errors[s_index];
// 				}
// 				for (int s_index = 0; s_index < (int)inner_input_state_output_errors.size(); s_index++) {
// 					local_state_errors[s_index] += inner_input_state_output_errors[s_index];
// 				}
// 			}
// 		}
// 	}

// 	// start
// 	if (this->is_branch[0]) {
// 		vector<double> branch_state_output_errors;
// 		this->branches[0]->existing_flat_backprop(local_s_input_errors,
// 												  local_state_errors,
// 												  branch_state_output_errors,
// 												  predicted_score,
// 												  predicted_score_error,
// 												  scale_factor,
// 												  average_factor_error,
// 												  scale_factor_error,
// 												  history->branch_histories[0]);
// 		local_state_errors = branch_state_output_errors;

// 		// don't modify mods within existing

// 		double branch_scale_mod_val = this->branch_scale_mod[0]->output->constants[0];
// 		scale_factor /= branch_scale_mod_val;

// 		double branch_average_mod_val = this->branch_average_mod[0]->output->constants[0];
// 		predicted_score -= scale_factor*branch_average_mod_val;

// 		average_factor_error *= branch_scale_mod_val;
// 		average_scale_error *= branch_scale_mod_val;
// 	} else {
// 		if (this->active_compress[0]) {
// 			vector<double> compress_s_input_output_errors;
// 			vector<double> compress_state_output_errors;
// 			this->compress_networks[0]->backprop_small_errors_with_no_weight_change(
// 				local_state_errors,
// 				compress_s_input_output_errors,
// 				compress_state_output_errors,
// 				history->compress_network_histories[0]);
// 			// use output sizes as might not have used all inputs
// 			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
// 				local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
// 			}
// 			local_state_errors = compress_state_output_errors;
// 		}
// 		// else cannot be compress

// 		average_factor_error += predicted_score_error;

// 		scale_factor_error += this->score_updates[0]*predicted_score_error;

// 		vector<double> score_errors{scale_factor*predicted_score_error};
// 		vector<double> score_s_input_output_errors;
// 		vector<double> score_state_output_errors;
// 		this->score_networks[0]->backprop_small_errors_with_no_weight_change(
// 			score_errors,
// 			score_s_input_output_errors,
// 			score_state_output_errors,
// 			history->score_network_histories[0]);
// 		// use output sizes as might not have used all inputs
// 		for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
// 			local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
// 		}
// 		for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
// 			local_state_errors[s_index] += score_state_output_errors[s_index];
// 		}
// 	}

// 	if (this->scopes[0]->type == SCOPE_TYPE_BASE) {
// 		// do nothing
// 	} else {
// 		// this->scopes[0]->type == SCOPE_TYPE_SCOPE
// 		Scope* scope_scope = (Scope*)this->scopes[0];

// 		double scope_scale_mod_val = this->scope_scale_mod[0]->output->constants[0];
// 		scale_factor *= scope_scale_mod_val;

// 		vector<double> scope_output_errors;
// 		double scope_average_factor_error = 0.0;
// 		double scope_scale_factor_error = 0.0;
// 		scope_scope->existing_flat_backprop(local_state_errors,
// 											scope_output_errors,
// 											predicted_score,
// 											predicted_score_error,
// 											scale_factor,
// 											scope_average_factor_error,
// 											scope_scale_factor_error,
// 											history->scope_histories[0]);

// 		// don't modify mods within existing

// 		average_factor_error += scope_scale_mod_val*scope_average_factor_error;
// 		scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

// 		scale_factor /= scope_scale_mod_val;

// 		double scope_average_mod_val = this->scope_average_mod[0]->output->constants[0];
// 		predicted_score -= scale_factor*scope_average_mod_val;

// 		for (int i_index = 0; i_index < scope_scope->num_inputs; i_index++) {
// 			local_s_input_errors[this->start_score_input_input_indexes[i_index]] += scope_output_errors[i_index];
// 		}
// 	}

// 	output_errors = local_s_input_errors;
// }

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->inner_input_networks = vector<vector<FoldNetworkHistory*>>(scope->sequence_length);
	this->scope_histories = vector<ScopeHistory*>(scope->sequence_length, NULL);
	this->branch_histories = vector<BranchHistory*>(scope->sequence_length, NULL);
	this->score_network_histories = vector<FoldNetworkHistory*>(scope->sequence_length, NULL);
	this->compress_network_histories = vector<FoldNetworkHistory*>(scope->sequence_length, NULL);

	this->fold_history = NULL;
}
