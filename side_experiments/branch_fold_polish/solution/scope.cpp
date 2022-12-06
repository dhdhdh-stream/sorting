#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"

using namespace std;



void Scope::explore_on_path_activate(vector<vector<double>>& flat_vals,
									 vector<double>& local_s_input_vals,	// i.e., input
									 vector<double>& local_state_vals,		// i.e., output
									 double& predicted_score,
									 double& scale_factor,
									 int& explore_phase,
									 ScopeHistory* history) {
	int a_index = 1;

	// start
	if (!this->is_inner_scope[0]) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
		// safe for starting action to be NO-OP -- still have full flexibility after
	} else {
		// for start, if inner scope, scope_input is first local_s_input_vals
		vector<double> scope_input(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);
		// TODO: check if this always holds (should always hold if scope construction sequential/correct)

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scopes[0]);
		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
			this->scopes[0]->explore_on_path_activate(flat_vals,
													  scope_input,
													  scope_output,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  scope_history);
		} else {
			this->scopes[0]->explore_off_path_activate(flat_vals,
													   scope_input,
													   scope_output,
													   predicted_score,
													   scale_factor,
													   explore_phase,
													   scope_history);
		}
		history->scope_histories[0] = scope_history;

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	if (this->step_types[0] == STEP_TYPE_STEP) {
		// explore_phase == EXPLORE_PHASE_NONE
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals);
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];
		
		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->explore_on_path_activate(existing_score,
														 flat_vals,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 explore_fold_history);
			history->explore_fold_history = explore_fold_history;

			explore_phase = EXPLORE_PHASE_FLAT;

			a_index = this->explore_end_non_inclusive;
		} else {
			history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
			predicted_score += existing_score;

			if (this->active_compress[0]) {
				// cannot be last action
				// compress 1 layer, add 1 layer
				// explore_phase == EXPLORE_PHASE_NONE
				this->compress_networks[0]->activate_small(local_s_input_vals,
														   local_state_vals);

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[0]);
				for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
					local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
				}
			}
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
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
			FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->explore_on_path_activate(branch_history->best_score,
														 flat_vals,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 explore_fold_history);
			history->explore_fold_history = explore_fold_history;

			explore_phase = EXPLORE_PHASE_FLAT;

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
			// if is also last action, extended local_state_vals will still be set correctly
		}
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		// explore_phase == EXPLORE_PHASE_NONE
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals);
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
			this->explore_fold->explore_on_path_activate(existing_score,
														 flat_vals,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 explore_fold_history);
			history->explore_fold_history = explore_fold_history;

			explore_phase = EXPLORE_PHASE_FLAT;

			a_index = this->explore_end_non_inclusive;
		} else {
			history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[0]);
			this->folds[0]->explore_off_path_activate(flat_vals,
													  existing_score,
													  local_s_input_vals,
													  local_state_vals,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  fold_history);
			history->fold_histories[0] = fold_history;
		}
	}

	// mid
	while (a_index < this->scopes.size()-1) {
		if (!this->is_inner_scope[a_index]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
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
					this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																				 local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				this->scopes[a_index]->explore_on_path_activate(flat_vals,
																temp_new_s_input_vals,
																scope_output,
																predicted_score,
																scale_factor,
																explore_phase,
																scope_history);
			} else {
				this->scopes[a_index]->explore_off_path_activate(flat_vals,
																 temp_new_s_input_vals,
																 scope_output,
																 predicted_score,
																 scale_factor,
																 explore_phase,
																 scope_history);
			}
			history->scope_histories[a_index] = scope_history;

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
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

				FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->explore_on_path_activate(existing_score,
															 flat_vals,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 explore_fold_history);
				history->explore_fold_history = explore_fold_history;

				explore_phase = EXPLORE_PHASE_FLAT;

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

					local_state_vals.clear();
					local_state_vals.reserve(this->compress_new_sizes[a_index]);
					for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
						local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
					}
				} else {
					local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[a_index], local_state_vals.end());
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
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
				FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->explore_on_path_activate(branch_history->best_score,
															 flat_vals,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 explore_fold_history);
				history->explore_fold_history = explore_fold_history;

				explore_phase = EXPLORE_PHASE_FLAT;

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
			// this->step_types[a_index] == STEP_TYPE_FOLD
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

				FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->explore_on_path_activate(existing_score,
															 flat_vals,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 explore_fold_history);
				history->explore_fold_history = explore_fold_history;

				explore_phase = EXPLORE_PHASE_FLAT;

				a_index = this->explore_end_non_inclusive-1;	// account for increment at end
			} else {
				history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
				// predicted_score updated in fold

				FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
				this->folds[a_index]->explore_off_path_activate(flat_vals,
																existing_score,
																local_s_input_vals,
																local_state_vals,
																predicted_score,
																scale_factor,
																explore_phase,
																fold_history);
				history->fold_histories[a_index] = fold_history;
			}
		}

		a_index++;
	}

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[this->scopes.size()-1].size(); i_index++) {
				if (explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[this->scopes.size()-1][i_index]);
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate_small(local_s_input_vals,
																							   local_state_vals,
																							   inner_input_network_history);
					history->inner_input_network_histories[this->scopes.size()-1].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate(local_s_input_vals,
																						 local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[this->scopes.size()-1]);
			if (this->explore_index_inclusive == this->scopes.size()-1
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				this->scopes[this->scopes.size()-1]->explore_on_path_activate(flat_vals,
																			  temp_new_s_input_vals,
																			  scope_output,
																			  predicted_score,
																			  scale_factor,
																			  explore_phase,
																			  scope_history);
			} else {
				this->scopes[this->scopes.size()-1]->explore_off_path_activate(flat_vals,
																			   temp_new_s_input_vals,
																			   scope_output,
																			   predicted_score,
																			   scale_factor,
																			   explore_phase,
																			   scope_history);
			}
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
}

void Scope::explore_off_path_activate(vector<vector<double>>& flat_vals,
									  vector<double>& local_s_input_vals,	// i.e., input
									  vector<double>& local_state_vals,		// i.e., output
									  double& predicted_score,
									  double& scale_factor,
									  int& explore_phase,
									  ScopeHistory* history) {
	// start
	if (!this->is_inner_scope[0]) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
	} else {
		// for start, if inner scope, scope_input is first local_s_input_vals
		vector<double> scope_input(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scopes[0]);
		this->scopes[0]->explore_off_path_activate(flat_vals,
												   scope_input,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   explore_phase,
												   scope_history);
		history->scope_histories[0] = scope_history;

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	if (this->step_types[0] == STEP_TYPE_STEP) {
		if (explore_phase == EXPLORE_PHASE_FLAT) {
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
			this->score_networks[0]->activate_small(local_s_input_vals,
													local_state_vals,
													score_network_history);
			history->score_network_histories[0] = score_network_history;
		} else {
			this->score_networks[0]->activate_small(local_s_input_vals,
													local_state_vals);
		}
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->active_compress[0]) {
			// cannot be last action
			// compress 1 layer, add 1 layer
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

			local_state_vals.clear();
			local_state_vals.reserve(this->compress_new_sizes[0]);
			for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
		// no else case as cannot be simple pop for start
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
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
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		if (explore_phase == EXPLORE_PHASE_FLAT) {
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
			this->score_networks[0]->activate_small(local_s_input_vals,
													local_state_vals,
													score_network_history);
			history->score_network_histories[0] = score_network_history;
		} else {
			this->score_networks[0]->activate_small(local_s_input_vals,
													local_state_vals);
		}
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->explore_off_path_activate(flat_vals,
												  existing_score,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  explore_phase,
												  fold_history);
		history->fold_histories[0] = fold_history;
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size()-1; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
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
					this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																				 local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->explore_off_path_activate(flat_vals,
															 temp_new_s_input_vals,
															 scope_output,
															 predicted_score,
															 scale_factor,
															 explore_phase,
															 scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
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

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[a_index]);
				for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[a_index], local_state_vals.end());
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
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
			// this->step_types[a_index] == STEP_TYPE_FOLD
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
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->explore_off_path_activate(flat_vals,
															existing_score,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															explore_phase,
															fold_history);
			history->fold_histories[a_index] = fold_history;
		}
	}

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[this->scopes.size()-1].size(); i_index++) {
				if (explore_phase == EXPLORE_PHASE_FLAT) {
					FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[this->scopes.size()-1][i_index]);
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate_small(local_s_input_vals,
																							   local_state_vals,
																							   inner_input_network_history);
					history->inner_input_network_histories[this->scopes.size()-1].push_back(inner_input_network_history);
				} else {
					this->inner_input_networks[this->scopes.size()-1][i_index]->activate(local_s_input_vals,
																						 local_state_vals);
				}
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[this->scopes.size()-1]);
			this->scopes[this->scopes.size()-1]->explore_off_path_activate(flat_vals,
																		   temp_new_s_input_vals,
																		   scope_output,
																		   predicted_score,
																		   scale_factor,
																		   explore_phase,
																		   scope_history);
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
}

void Scope::explore_on_path_backprop(vector<double>& local_state_errors,	// i.e., input_errors
									 double& predicted_score,
									 double target_val,
									 double& scale_factor,
									 double& scale_factor_error,
									 ScopeHistory* history) {
	// don't need to output local_s_input_errors on path but for explore_off_path_backprop
	vector<double> local_s_input_errors(this->num_inputs, 0.0);

	int a_index;
	if (this->explore_end_non_inclusive == this->scopes.size()
			&& this->explore_type == EXPLORE_TYPE_NEW) {
		a_index = this->explore_index_inclusive;
	} else {
		a_index = this->scopes.size()-1;
	}

	// end
	if (this->scopes.size() > 1 && a_index == this->scopes.size()-1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
				local_state_errors.end());
		} else {
			vector<double> scope_input_errors(local_state_errors.end()-this->scopes[this->scopes.size()-1]->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-this->scopes[this->scopes.size()-1]->num_outputs,
				local_state_errors.end());

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			if (this->explore_index_inclusive == this->scopes.size()-1
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				// on_path doesn't need scope_output_errors
				
				scale_factor_error /= this->scope_scale_mod[this->scopes.size()-1];

				this->scopes[this->scopes.size()-1]->explore_on_path_backprop(scope_input_errors,
																			  predicted_score,
																			  target_val,
																			  scale_factor,
																			  scale_factor_error,
																			  history->scope_histories[this->scopes.size()-1]);

				return;
			} else {
				vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
				double scope_scale_factor_error = 0.0;
				this->scopes[this->scopes.size()-1]->explore_off_path_backprop(scope_input_errors,
																			   scope_output_errors,
																			   predicted_score,
																			   target_val,
																			   scale_factor,
																			   scope_scale_factor_error,
																			   history->scope_histories[this->scopes.size()-1]);

				scale_factor_error += this->scope_scale_mod[this->scopes.size()-1]*scope_scale_factor_error;

				scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

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
			this->explore_fold->explore_on_path_backprop(local_state_errors,
														 predicted_score,
														 target_val,
														 scale_factor,
														 scale_factor_error,
														 history->explore_fold_history);

			return;
		} else {
			if (this->step_types[a_index] == STEP_TYPE_STEP) {
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
					// this->compress_original_sizes[a_index] > this->compress_new_sizes[a_index]
					int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
					for (int c_index = 0; c_index < compress_size; c_index++) {
						local_state_errors.push_back(0.0);
					}
				}

				double predicted_score_error = target_val - predicted_score;

				scale_factor_error += this->score_updates[a_index]*predicted_score_error;

				// have to include scale_factor as it can change the sign of the gradient
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

				predicted_score -= scale_factor*this->score_updates[a_index];
			} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_backprop(local_state_errors,
																	  predicted_score,
																	  target_val,
																	  scale_factor,
																	  scale_factor_error,
																	  history->branch_histories[a_index]);

					return;
				} else {
					this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
																	   local_state_errors,
																	   predicted_score,
																	   target_val,
																	   scale_factor,
																	   scale_factor_error,
																	   history->branch_histories[a_index]);
				}
			} else {
				// this->step_types[a_index] == STEP_TYPE_FOLD
				this->folds[a_index]->explore_off_path_backprop(local_s_input_errors,
																local_state_errors,
																predicted_score,
																target_val,
																scale_factor,
																scale_factor_error,
																history->fold_histories[a_index]);

				// predicted_score already modified to before fold value in fold
				double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
				double score_predicted_score_error = target_val - score_predicted_score;

				scale_factor_error += this->score_updates[a_index]*score_predicted_score_error;

				// have to include scale_factor as it can change the sign of the gradient
				vector<double> score_errors{scale_factor*score_predicted_score_error};
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

		if (!this->is_inner_scope[a_index]) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			vector<double> scope_input_errors(local_state_errors.end()-this->scopes[a_index]->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-this->scopes[a_index]->num_outputs,
				local_state_errors.end());

			scale_factor *= this->scope_scale_mod[a_index];

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				scale_factor_error /= this->scope_scale_mod[a_index];

				this->scopes[a_index]->explore_on_path_backprop(scope_input_errors,
																predicted_score,
																target_val,
																scale_factor,
																scale_factor_error,
																history->scope_histories[a_index]);

				return;
			} else {
				vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
				double scope_scale_factor_error = 0.0;
				this->scopes[a_index]->explore_off_path_backprop(scope_input_errors,
																 scope_output_errors,
																 predicted_score,
																 target_val,
																 scale_factor,
																 scope_scale_factor_error,
																 history->scope_histories[a_index]);

				scale_factor_error += this->scope_scale_mod[a_index]*scope_scale_factor_error;

				scale_factor /= this->scope_scale_mod[a_index];

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
		this->explore_fold->explore_on_path_backprop(local_state_errors,
													 predicted_score,
													 target_val,
													 scale_factor,
													 scale_factor_error,
													 history->explore_fold_history);

		return;
	} else {
		if (this->step_types[0] == STEP_TYPE_STEP) {
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
				// this->compress_original_sizes[0] > this->compress_new_sizes[0]
				int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
				for (int c_index = 0; c_index < compress_size; c_index++) {
					local_state_errors.push_back(0.0);
				}
			}

			double predicted_score_error = target_val - predicted_score;

			scale_factor_error += this->score_updates[0]*predicted_score_error;

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

			predicted_score -= scale_factor*this->score_updates[0];
		} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
			if (this->explore_index_inclusive == 0
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[0]->explore_on_path_backprop(local_state_errors,
															predicted_score,
															target_val,
															scale_factor,
															scale_factor_error,
															history->branch_histories[0]);

				return;
			} else {
				this->branches[0]->explore_off_path_backprop(local_s_input_errors,
															 local_state_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 scale_factor_error,
															 history->branch_histories[0]);
			}
		} else {
			// this->step_types[0] == STEP_TYPE_FOLD
			this->folds[0]->explore_off_path_backprop(local_s_input_errors,
													  local_state_errors,
													  predicted_score,
													  target_val,
													  scale_factor,
													  scale_factor_error,
													  history->fold_histories[0]);

			// predicted_score already modified to before fold value in fold
			double score_predicted_score = predicted_score + scale_factor*history->score_updates[0];
			double score_predicted_score_error = target_val - score_predicted_score;

			scale_factor_error += this->score_updates[0]*score_predicted_score_error;

			vector<double> score_errors{scale_factor*score_predicted_score_error};
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

	// this->is_inner_scope[0] == true

	// scope_input_errors is just local_state_errors at start

	scale_factor *= this->scope_scale_mod[0];

	// this->explore_index_inclusive == 0 && this->explore_type == EXPLORE_TYPE_INNER_SCOPE

	scale_factor_error /= this->scope_scale_mod[0];

	this->scopes[0]->explore_on_path_backprop(local_state_errors,
											  predicted_score,
											  target_val,
											  scale_factor,
											  scale_factor_error,
											  history->scope_histories[0]);

	return;
}

void Scope::explore_off_path_backprop(vector<double>& local_state_errors,	// i.e., input_errors
									  vector<double>& local_s_input_errors,	// i.e., output_errors
									  double& predicted_score,
									  double target_val,
									  double& scale_factor,
									  double& scale_factor_error,
									  ScopeHistory* history) {
	local_s_input_errors = vector<double>(this->num_inputs, 0.0);

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
				local_state_errors.end());
		} else {
			vector<double> scope_input_errors(local_state_errors.end()-this->scopes[this->scopes.size()-1]->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-this->scopes[this->scopes.size()-1]->num_outputs,
				local_state_errors.end());

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			double scope_scale_factor_error = 0.0;
			this->scopes[this->scopes.size()-1]->explore_off_path_backprop(scope_input_errors,
																		   scope_output_errors,
																		   predicted_score,
																		   target_val,
																		   scale_factor,
																		   scope_scale_factor_error,
																		   history->scope_histories[this->scopes.size()-1]);

			scale_factor_error += this->scope_scale_mod[this->scopes.size()-1]*scope_scale_factor_error;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

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

	// mid
	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
		if (this->step_types[a_index] == STEP_TYPE_STEP) {
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
				// this->compress_original_sizes[a_index] > this->compress_new_sizes[a_index]
				int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
				for (int c_index = 0; c_index < compress_size; c_index++) {
					local_state_errors.push_back(0.0);
				}
			}

			double predicted_score_error = target_val - predicted_score;

			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

			vector<double> score_errors{scale_factor*predicted_score_error};
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
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
															   local_state_errors,
															   predicted_score,
															   target_val,
															   scale_factor,
															   scale_factor_error,
															   history->branch_histories[a_index]);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->folds[a_index]->explore_off_path_backprop(local_s_input_errors,
															local_state_errors,
															predicted_score,
															target_val,
															scale_factor,
															scale_factor_error,
															history->fold_histories[a_index]);

			// predicted_score already modified to before fold value in fold
			double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
			double score_predicted_score_error = target_val - score_predicted_score;

			scale_factor_error += this->score_updates[a_index]*score_predicted_score_error;

			vector<double> score_errors{scale_factor*score_predicted_score_error};
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
		}

		if (!this->is_inner_scope[a_index]) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			vector<double> scope_input_errors(local_state_errors.end()-this->scopes[a_index]->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-this->scopes[a_index]->num_outputs,
				local_state_errors.end());

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->explore_off_path_backprop(scope_input_errors,
															 scope_output_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 scope_scale_factor_error,
															 history->scope_histories[a_index]);

			scale_factor_error += this->scope_scale_mod[a_index]*scope_scale_factor_error;

			scale_factor /= this->scope_scale_mod[a_index];

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
	if (this->step_types[0] == STEP_TYPE_STEP) {
		if (this->active_compress[0]) {
			vector<double> compress_s_input_output_errors;
			vector<double> compress_state_output_errors;
			this->compress_networks[0]->backprop_small_errors_with_no_weight_change(
				local_state_errors,
				compress_s_input_output_errors,
				compress_state_output_errors,
				history->compress_network_histories[0]);
			// use output sizes as might not have used all inputs
			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
				local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
			}
			local_state_errors = compress_state_output_errors;
		}
		// else cannot be compress

		double predicted_score_error = target_val - predicted_score;

		scale_factor_error += this->score_updates[0]*predicted_score_error;

		vector<double> score_errors{scale_factor*predicted_score_error};
		vector<double> score_s_input_output_errors;
		vector<double> score_state_output_errors;
		this->score_networks[0]->backprop_small_errors_with_no_weight_change(
			score_errors,
			score_s_input_output_errors,
			score_state_output_errors,
			history->score_network_histories[0]);
		// use output sizes as might not have used all inputs
		for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
			local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
			local_state_errors[s_index] += score_state_output_errors[s_index];
		}

		predicted_score -= scale_factor*this->score_updates[0];
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		this->branches[0]->explore_off_path_backprop(local_s_input_errors,
													 local_state_errors,
													 predicted_score,
													 target_val,
													 scale_factor,
													 scale_factor_error,
													 history->branch_histories[0]);
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		this->folds[0]->explore_off_path_backprop(local_s_input_errors,
												  local_state_errors,
												  predicted_score,
												  target_val,
												  scale_factor,
												  scale_factor_error,
												  history->fold_histories[0]);

		// predicted_score already modified to before fold value in fold
		double score_predicted_score = predicted_score + scale_factor*history->score_updates[0];
		double score_predicted_score_error = target_val - score_predicted_score;

		scale_factor_error += this->score_updates[0]*score_predicted_score_error;

		vector<double> score_errors{scale_factor*score_predicted_score_error};
		vector<double> score_s_input_output_errors;
		vector<double> score_state_output_errors;
		this->score_networks[0]->backprop_small_errors_with_no_weight_change(
			score_errors,
			score_s_input_output_errors,
			score_state_output_errors,
			history->score_network_histories[0]);
		// use output sizes as might not have used all inputs
		for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
			local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
			local_state_errors[s_index] += score_state_output_errors[s_index];
		}
	}

	if (!this->is_inner_scope[0]) {
		// do nothing
	} else {
		// scope_input_errors is just local_state_errors at start

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->scopes[0]->explore_off_path_backprop(local_state_errors,
												   scope_output_errors,
												   predicted_score,
												   target_val,
												   scale_factor,
												   scope_scale_factor_error,
												   history->scope_histories[0]);

		scale_factor_error += this->scope_scale_mod[0]*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod[0];

		// for start, if inner scope, scope_input is first local_s_input_vals
		for (int i_index = 0; i_index < this->scopes[0]->num_inputs; i_index++) {
			local_s_input_errors[i_index] += scope_output_errors[i_index];
		}
	}
}

void Scope::existing_flat_activate(vector<vector<double>>& flat_vals,
								   vector<double>& local_s_input_vals,	// i.e., input
								   vector<double>& local_state_vals,	// i.e., output
								   double& predicted_score,
								   double& scale_factor,
								   ScopeHistory* history) {
	// start
	if (!this->is_inner_scope[0]) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
	} else {
		// for start, if inner scope, scope_input is first local_s_input_vals
		vector<double> scope_input(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scopes[0]);
		this->scopes[0]->existing_flat_activate(flat_vals,
												scope_input,
												scope_output,
												predicted_score,
												scale_factor,
												scope_history);
		history->scope_histories[0] = scope_history;

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	if (this->step_types[0] == STEP_TYPE_STEP) {
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals,
												score_network_history);
		history->score_network_histories[0] = score_network_history;
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->active_compress[0]) {
			// cannot be last action
			// compress 1 layer, add 1 layer
			FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[0]);
			this->compress_networks[0]->activate_small(local_s_input_vals,
													   local_state_vals,
													   compress_network_history);
			history->compress_network_histories[0] = compress_network_history;

			local_state_vals.clear();
			local_state_vals.reserve(this->compress_new_sizes[0]);
			for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->existing_flat_activate(flat_vals,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  branch_history);
		history->branch_histories[0] = branch_history;
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals,
												score_network_history);
		history->score_network_histories[0] = score_network_history;
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->existing_flat_activate(flat_vals,
											   existing_score,
											   local_s_input_vals,
											   local_state_vals,
											   predicted_score,
											   scale_factor,
											   fold_history);
		history->fold_histories[0] = fold_history;
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size()-1; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[a_index][i_index]);
				this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																			 local_state_vals,
																			 inner_input_network_history);
				history->inner_input_network_histories[a_index].push_back(inner_input_network_history);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->existing_flat_activate(flat_vals,
														  temp_new_s_input_vals,
														  scope_output,
														  predicted_score,
														  scale_factor,
														  scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals,
														  score_network_history);
			history->score_network_histories[a_index] = score_network_history;
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->active_compress[a_index]) {
				// compress 2 layers, add 1 layer
				FoldNetworkHistory* compress_network_history = new FoldNetworkHistory(this->compress_networks[a_index]);
				this->compress_networks[a_index]->activate_small(local_s_input_vals,
																 local_state_vals,
																 compress_network_history);
				history->compress_network_histories[a_index] = compress_network_history;

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[a_index]);
				for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[a_index], local_state_vals.end());
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->existing_flat_activate(flat_vals,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															branch_history);
			history->branch_histories[a_index] = branch_history;
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals,
														  score_network_history);
			history->score_network_histories[a_index] = score_network_history;
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->existing_flat_activate(flat_vals,
														 existing_score,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 fold_history);
			history->fold_histories[a_index] = fold_history;
		}
	}

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[this->scopes.size()-1].size(); i_index++) {
				FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[this->scopes.size()-1][i_index]);
				this->inner_input_networks[this->scopes.size()-1][i_index]->activate_small(input,
																						   local_state_vals,
																						   inner_input_network_history);
				history->inner_input_network_histories[this->scopes.size()-1].push_back(inner_input_network_history);
				for (int s_index = 0; s_index < this->inner_input_sizes[this->scopes.size()-1][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[this->scopes.size()-1][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[this->scopes.size()-1]);
			this->scopes[this->scopes.size()-1]->existing_flat_activate(flat_vals,
																		temp_new_s_input_vals,
																		scope_output,
																		predicted_score,
																		scale_factor,
																		scope_history);
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
}

void Scope::existing_flat_backprop(vector<double>& local_state_errors,		// input_errors
								   vector<double>& local_s_input_errors,	// output_errors
								   double& predicted_score,
								   double predicted_score_error,
								   double& scale_factor,
								   double& scale_factor_error,
								   ScopeHistory* history) {
	local_s_input_errors = vector<double>(this->num_inputs, 0.0);

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[this->scopes.size()-1],
				local_state_errors.end());
		} else {
			vector<double> scope_input_errors(local_state_errors.end()-this->scopes[this->scopes.size()-1]->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-this->scopes[this->scopes.size()-1]->num_outputs,
				local_state_errors.end());

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			double scope_scale_factor_error = 0.0;
			this->scopes[this->scopes.size()-1]->existing_flat_backprop(scope_input_errors,
																		scope_output_errors,
																		predicted_score,
																		predicted_score_error,
																		scale_factor,
																		scope_scale_factor_error,
																		history->scope_histories[this->scopes.size()-1]);

			scale_factor_error += this->scope_scale_mod[this->scopes.size()-1]*scope_scale_factor_error;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

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

	// mid
	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
		if (this->step_types[a_index] == STEP_TYPE_STEP) {
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
				// this->compress_original_sizes[a_index] > this->compress_new_sizes[a_index]
				int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
				for (int c_index = 0; c_index < compress_size; c_index++) {
					local_state_errors.push_back(0.0);
				}
			}

			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

			vector<double> score_errors{scale_factor*predicted_score_error};
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
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->branches[a_index]->existing_flat_backprop(local_s_input_errors,
															local_state_errors,
															predicted_score,
															predicted_score_error,
															scale_factor,
															scale_factor_error,
															history->branch_histories[a_index]);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->folds[a_index]->existing_flat_backprop(local_s_input_errors,
														 local_state_errors,
														 predicted_score,
														 predicted_score_error,
														 scale_factor,
														 scale_factor_error,
														 history->fold_histories[a_index]);

			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

			vector<double> score_errors{scale_factor*predicted_score_error};
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

			// predicted_score updated in fold
		}

		if (!this->is_inner_scope[a_index]) {
			local_state_errors.erase(local_state_errors.end()-this->obs_sizes[a_index],
				local_state_errors.end());
		} else {
			vector<double> scope_input_errors(local_state_errors.end()-this->scopes[a_index]->num_outputs,
				local_state_errors.end());
			local_state_errors.erase(local_state_errors.end()-this->scopes[a_index]->num_outputs,
				local_state_errors.end());

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output_errors;
			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->existing_flat_backprop(scope_input_errors,
														  scope_output_errors,
														  predicted_score,
														  predicted_score_error,
														  scale_factor,
														  scope_scale_factor_error,
														  history->scope_histories[a_index]);

			scale_factor_error += this->scope_scale_mod[a_index]*scope_scale_factor_error;

			scale_factor /= this->scope_scale_mod[a_index];

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
	if (this->step_types[0] == STEP_TYPE_STEP) {
		if (this->active_compress[0]) {
			vector<double> compress_s_input_output_errors;
			vector<double> compress_state_output_errors;
			this->compress_networks[0]->backprop_small_errors_with_no_weight_change(
				local_state_errors,
				compress_s_input_output_errors,
				compress_state_output_errors,
				history->compress_network_histories[0]);
			// use output sizes as might not have used all inputs
			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
				local_s_input_errors[s_index] += compress_s_input_output_errors[s_index];
			}
			local_state_errors = compress_state_output_errors;
		}
		// else cannot be compress

		scale_factor_error += this->score_updates[0]*predicted_score_error;

		vector<double> score_errors{scale_factor*predicted_score_error};
		vector<double> score_s_input_output_errors;
		vector<double> score_state_output_errors;
		this->score_networks[0]->backprop_small_errors_with_no_weight_change(
			score_errors,
			score_s_input_output_errors,
			score_state_output_errors,
			history->score_network_histories[0]);
		// use output sizes as might not have used all inputs
		for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
			local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
			local_state_errors[s_index] += score_state_output_errors[s_index];
		}

		predicted_score -= scale_factor*this->score_updates[0];
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		this->branches[0]->existing_flat_backprop(local_s_input_errors,
												  local_state_errors,
												  predicted_score,
												  predicted_score_error,
												  scale_factor,
												  scale_factor_error,
												  history->branch_histories[0]);
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		this->folds[0]->existing_flat_backprop(local_s_input_errors,
											   local_state_errors,
											   predicted_score,
											   predicted_score_error,
											   scale_factor,
											   scale_factor_error,
											   history->fold_histories[0]);

		scale_factor_error += this->score_updates[0]*predicted_score_error;

		vector<double> score_errors{scale_factor*predicted_score_error};
		vector<double> score_s_input_output_errors;
		vector<double> score_state_output_errors;
		this->score_networks[0]->backprop_small_errors_with_no_weight_change(
			score_errors,
			score_s_input_output_errors,
			score_state_output_errors,
			history->score_network_histories[0]);
		// use output sizes as might not have used all inputs
		for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
			local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
			local_state_errors[s_index] += score_state_output_errors[s_index];
		}

		// predicted_score updated in fold
	}

	if (!this->is_inner_scope[0]) {
		// do nothing
	} else {
		// scope_input_errors is just local_state_errors at start

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output_errors;
		double scope_scale_factor_error = 0.0;
		this->scopes[0]->existing_flat_backprop(local_state_errors,
												scope_output_errors,
												predicted_score,
												predicted_score_error,
												scale_factor,
												scope_scale_factor_error,
												history->scope_histories[0]);

		scale_factor_error += this->scope_scale_mod[0]*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod[0];

		// for start, if inner scope, scope_input is first local_s_input_vals
		for (int i_index = 0; i_index < this->scopes[0]->num_inputs; i_index++) {
			local_s_input_errors[i_index] += scope_output_errors[i_index];
		}
	}
}

void Scope::update_activate(vector<vector<double>>& flat_vals,
							vector<double>& local_s_input_vals,	// i.e., input
							vector<double>& local_state_vals,	// i.e., output
							double& predicted_score,
							double& scale_factor,
							ScopeHistory* history) {
	// start
	if (!this->is_inner_scope[0]) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
	} else {
		// for start, if inner scope, scope_input is first local_s_input_vals
		vector<double> scope_input(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scopes[0]);
		this->scopes[0]->update_activate(flat_vals,
										 scope_input,
										 scope_output,
										 predicted_score,
										 scale_factor,
										 scope_history);
		history->scope_histories[0] = scope_history;

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	if (this->step_types[0] == STEP_TYPE_STEP) {
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals,
												score_network_history);
		history->score_network_histories[0] = score_network_history;
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->active_compress[0]) {
			// cannot be last action
			// compress 1 layer, add 1 layer
			this->compress_networks[0]->activate_small(local_s_input_vals,
													   local_state_vals);

			local_state_vals.clear();
			local_state_vals.reserve(this->compress_new_sizes[0]);
			for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->update_activate(flat_vals,
										   local_s_input_vals,
										   local_state_vals,
										   predicted_score,
										   scale_factor,
										   branch_history);
		history->branch_histories[0] = branch_history;
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[0]);
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals,
												score_network_history);
		history->score_network_histories[0] = score_network_history;
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->update_activate(flat_vals,
										existing_score,
										local_s_input_vals,
										local_state_vals,
										predicted_score,
										scale_factor,
										fold_history);
		history->fold_histories[0] = fold_history;
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size()-1; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																			 local_state_vals);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->update_activate(flat_vals,
												   temp_new_s_input_vals,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals,
														  score_network_history);
			history->score_network_histories[a_index] = score_network_history;
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->active_compress[a_index]) {
				// compress 2 layers, add 1 layer
				this->compress_networks[a_index]->activate_small(local_s_input_vals,
																 local_state_vals);

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[a_index]);
				for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[a_index], local_state_vals.end());
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->update_activate(flat_vals,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 branch_history);
			history->branch_histories[a_index] = branch_history;
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals,
														  score_network_history);
			history->score_network_histories[a_index] = score_network_history;
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->update_activate(flat_vals,
												  existing_score,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  fold_history);
			history->fold_histories[a_index] = fold_history;
		}
	}

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
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

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[this->scopes.size()-1]);
			this->scopes[this->scopes.size()-1]->update_activate(flat_vals,
																 temp_new_s_input_vals,
																 scope_output,
																 predicted_score,
																 scale_factor,
																 scope_history);
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
}

void Scope::update_backprop(double& predicted_score,
							double& next_predicted_score,	// for ending misguess from outside
							double target_val,
							double& scale_factor,
							ScopeHistory* history) {
	// end
	if (this->scopes.size() > 1) {
		double misguess = target_val - next_predicted_score;
		this->average_misguesses[this->scopes.size()-1] = 0.999*this->average_misguesses[this->scopes.size()-1] + 0.001*misguess;

		if (!this->is_inner_scope[this->scopes.size()-1]) {
			// do nothing
		} else {
			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			this->scopes[this->scopes.size()-1]->update_backprop(predicted_score,
																 next_predicted_score,
																 target_val,
																 scale_factor,
																 history->scope_histories[this->scopes.size()-1]);

			// mods are a helper for initial flat, so no need to modify after

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];
		}
	}

	// mid
	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
		double misguess = target_val - predicted_score;
		this->average_misguesses[a_index] = 0.999*this->average_misguesses[a_index] + 0.001*misguess;

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			double predicted_score_error = target_val - predicted_score;

			vector<double> score_errors{scale_factor*predicted_score_error};
			this->score_networks[a_index]->backprop_small_weights_with_no_error_signal(
				score_errors,
				0.001,
				history->score_network_histories[a_index]);

			next_predicted_score = predicted_score;
			predicted_score -= scale_factor*this->score_updates[a_index];
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->branches[a_index]->update_backprop(predicted_score,
													 next_predicted_score,
													 target_val,
													 scale_factor,
													 history->branch_histories[a_index]);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->folds[a_index]->update_backprop(predicted_score,
												  next_predicted_score,
												  target_val,
												  scale_factor,
												  history->fold_histories[a_index]);

			// predicted_score already modified to before fold value in fold
			double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
			double score_predicted_score_error = target_val - score_predicted_score;

			vector<double> score_errors{scale_factor*score_predicted_score_error};
			this->score_networks[a_index]->backprop_small_weights_with_no_error_signal(
				score_errors,
				0.001,
				history->score_network_histories[a_index]);
		}

		if (!this->is_inner_scope[a_index]) {
			// do nothing
		} else {
			scale_factor *= this->scope_scale_mod[a_index];

			this->scopes[a_index]->update_backprop(predicted_score,
												   next_predicted_score,
												   target_val,
												   scale_factor,
												   history->scope_histories[a_index]);

			// mods are a helper for initial flat, so no need to modify after

			scale_factor /= this->scope_scale_mod[a_index];
		}
	}

	// start
	double misguess = target_val - predicted_score;
	this->average_misguesses[0] = 0.999*this->average_misguesses[0] + 0.001*misguess;

	if (this->step_types[0] == STEP_TYPE_STEP) {
		double predicted_score_error = target_val - predicted_score;

		vector<double> score_errors{scale_factor*predicted_score_error};
		this->score_networks[0]->backprop_small_weights_with_no_error_signal(
			score_errors,
			0.001,
			history->score_network_histories[0]);

		next_predicted_score = predicted_score;
		predicted_score -= scale_factor*this->score_updates[0];
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		this->branches[0]->update_backprop(predicted_score,
										   next_predicted_score,
										   target_val,
										   scale_factor,
										   history->branch_histories[0]);
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		this->folds[0]->update_backprop(predicted_score,
										next_predicted_score,
										target_val,
										scale_factor,
										history->fold_histories[0]);

		// predicted_score already modified to before fold value in fold
		double score_predicted_score = predicted_score + scale_factor*history->score_updates[0];
		double score_predicted_score_error = target_val - score_predicted_score;

		vector<double> score_errors{scale_factor*score_predicted_score_error};
		this->score_networks[0]->backprop_small_weights_with_no_error_signal(
			score_errors,
			0.001,
			history->score_network_histories[0]);
	}

	if (!this->is_inner_scope[0]) {
		// do nothing
	} else {
		scale_factor *= this->scope_scale_mod[0];

		this->scopes[0]->update_backprop(predicted_score,
										 next_predicted_score,
										 target_val,
										 scale_factor,
										 history->scope_histories[0]);

		// mods are a helper for initial flat, so no need to modify after

		scale_factor /= this->scope_scale_mod[0];
	}
}

void Scope::existing_update_activate(vector<vector<double>>& flat_vals,
									 vector<double>& local_s_input_vals,	// i.e., input
									 vector<double>& local_state_vals,	// i.e., output
									 double& predicted_score,
									 double& scale_factor,
									 ScopeHistory* history) {
	// start
	if (!this->is_inner_scope[0]) {
		local_state_vals = flat_vals.begin();
		flat_vals.erase(flat_vals.begin());
	} else {
		// for start, if inner scope, scope_input is first local_s_input_vals
		vector<double> scope_input(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);

		scale_factor *= this->scope_scale_mod[0];

		vector<double> scope_output;
		ScopeHistory* scope_history = new ScopeHistory(this->scopes[0]);
		this->scopes[0]->existing_update_activate(flat_vals,
												  scope_input,
												  scope_output,
												  predicted_score,
												  scale_factor,
												  scope_history);
		history->scope_histories[0] = scope_history;

		scale_factor /= this->scope_scale_mod[0];

		local_state_vals = scope_output;
	}

	if (this->step_types[0] == STEP_TYPE_STEP) {
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals);
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		predicted_score += scale_factor*this->score_networks[0]->output->acti_vals[0];

		if (this->active_compress[0]) {
			// cannot be last action
			// compress 1 layer, add 1 layer
			this->compress_networks[0]->activate_small(local_s_input_vals,
													   local_state_vals);

			local_state_vals.clear();
			local_state_vals.reserve(this->compress_new_sizes[0]);
			for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->existing_update_activate(flat_vals,
													local_s_input_vals,
													local_state_vals,
													predicted_score,
													scale_factor,
													branch_history);
		history->branch_histories[0] = branch_history;
		// if is also last action, extended local_state_vals will still be set correctly
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		this->score_networks[0]->activate_small(local_s_input_vals,
												local_state_vals);
		history->score_updates[0] = this->score_networks[0]->output->acti_vals[0];
		double existing_score = scale_factor*this->score_networks[0]->output->acti_vals[0];
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->existing_update_activate(flat_vals,
												 existing_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 fold_history);
		history->fold_histories[0] = fold_history;
	}

	// mid
	for (int a_index = 1; a_index < this->scopes.size()-1; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
			// temp_new_s_input_vals only used as scope_input
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> temp_new_s_input_vals;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																			 local_state_vals);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					temp_new_s_input_vals.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			scale_factor *= this->scope_scale_mod[a_index];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->existing_update_activate(flat_vals,
															temp_new_s_input_vals,
															scope_output,
															predicted_score,
															scale_factor,
															scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= this->scope_scale_mod[a_index];

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals);
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			if (this->active_compress[a_index]) {
				// compress 2 layers, add 1 layer
				this->compress_networks[a_index]->activate_small(local_s_input_vals,
																 local_state_vals);

				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[a_index]);
				for (int s_index = 0; s_index < this->compress_new_sizes[a_index]; s_index++) {
					local_state_vals.push_back(this->compress_networks[a_index]->output->acti_vals[s_index]);
				}
			} else {
				local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[a_index], local_state_vals.end());
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->existing_update_activate(flat_vals,
															  local_s_input_vals,
															  local_state_vals,
															  predicted_score,
															  scale_factor,
															  branch_history);
			history->branch_histories[a_index] = branch_history;
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals);
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->existing_update_activate(flat_vals,
														   existing_score,
														   local_s_input_vals,
														   local_state_vals,
														   predicted_score,
														   scale_factor,
														   fold_history);
			history->fold_histories[a_index] = fold_history;
		}
	}

	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			local_state_vals.insert(local_state_vals.end(),
				flat_vals.begin().begin(), flat_vals.begin().end());
			flat_vals.erase(flat_vals.begin());
		} else {
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

			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[this->scopes.size()-1]);
			this->scopes[this->scopes.size()-1]->existing_update_activate(flat_vals,
																		  temp_new_s_input_vals,
																		  scope_output,
																		  predicted_score,
																		  scale_factor,
																		  scope_history);
			history->scope_histories[this->scopes.size()-1] = scope_history;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];

			// append to local_state_vals for outer
			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}
	}
}

void Scope::existing_update_backprop(double& predicted_score,
									 double predicted_score_error,
									 double& scale_factor,
									 double& scale_factor_error,
									 ScopeHistory* history) {
	// end
	if (this->scopes.size() > 1) {
		if (!this->is_inner_scope[this->scopes.size()-1]) {
			// do nothing
		} else {
			scale_factor *= this->scope_scale_mod[this->scopes.size()-1];

			double scope_scale_factor_error = 0.0;
			this->scopes[this->scopes.size()-1]->existing_update_backprop(predicted_score,
																		  predicted_score_error,
																		  scale_factor,
																		  scope_scale_factor_error,
																		  history->scope_histories[this->scopes.size()-1]);

			scale_factor_error += this->scope_scale_mod[this->scopes.size()-1]*scope_scale_factor_error;

			scale_factor /= this->scope_scale_mod[this->scopes.size()-1];
		}
	}

	// mid
	for (int a_index = (int)this->scopes.size()-2; a_index >= 1; a_index--) {
		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

			predicted_score -= scale_factor*this->score_updates[a_index];
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->branches[a_index]->existing_update_backprop(predicted_score,
															  predicted_score_error,
															  scale_factor,
															  scale_factor_error,
															  history->branch_histories[a_index]);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->folds[a_index]->existing_update_backprop(predicted_score,
														   predicted_score_error,
														   scale_factor,
														   scale_factor_error,
														   history->fold_histories[a_index]);

			scale_factor_error += this->score_updates[a_index]*predicted_score_error;

			// predicted_score updated in fold
		}

		if (!this->is_inner_scope[a_index]) {
			// do nothing
		} else {
			scale_factor *= this->scope_scale_mod[a_index];

			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->existing_update_backprop(predicted_score,
															predicted_score_error,
															scale_factor,
															scope_scale_factor_error,
															history->scope_histories[a_index]);

			scale_factor_error += this->scope_scale_mod[a_index]*scope_scale_factor_error;

			scale_factor /= this->scope_scale_mod[a_index];
		}
	}

	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		scale_factor_error += this->score_updates[0]*predicted_score_error;

		predicted_score -= scale_factor*this->score_updates[0];
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		this->branches[0]->existing_update_backprop(predicted_score,
													predicted_score_error,
													scale_factor,
													scale_factor_error,
													history->branch_histories[0]);
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		this->folds[0]->existing_update_backprop(predicted_score,
												 predicted_score_error,
												 scale_factor,
												 scale_factor_error,
												 history->fold_histories[0]);

		scale_factor_error += this->score_updates[0]*predicted_score_error;

		// predicted_score updated in fold
	}

	if (!this->is_inner_scope[0]) {
		// do nothing
	} else {
		scale_factor *= this->scope_scale_mod[0];

		double scope_scale_factor_error = 0.0;
		this->scopes[0]->existing_update_backprop(predicted_score,
												  predicted_score_error,
												  scale_factor,
												  scope_scale_factor_error,
												  history->scope_histories[0]);

		scale_factor_error += this->scope_scale_mod[0]*scope_scale_factor_error;

		scale_factor /= this->scope_scale_mod[0];
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->inner_input_networks = vector<vector<FoldNetworkHistory*>>(scope->sequence_length);
	this->scope_histories = vector<ScopeHistory*>(scope->sequence_length, NULL);
	this->branch_histories = vector<BranchHistory*>(scope->sequence_length, NULL);
	this->fold_histories = vector<FoldHistory*>(scope->sequence_length, NULL);
	this->score_network_histories = vector<FoldNetworkHistory*>(scope->sequence_length, NULL);
	this->score_updates = vector<double>(scope->sequence_length, 0.0);
	this->compress_network_histories = vector<FoldNetworkHistory*>(scope->sequence_length, NULL);

	this->explore_fold_history = NULL;
}
