#include "branch.h"

#include <iostream>
#include <limits>

using namespace std;



void Branch::explore_on_path_activate_score(vector<double>& local_s_input_vals,
											vector<double>& local_state_vals,
											double& scale_factor,
											int& explore_phase,
											BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	if (explore_phase == EXPLORE_PHASE_FLAT) {
		if (!this->passed_branch_score) {
			FoldNetworkHistory* branch_score_network_history = new FoldNetworkHistory(this->branch_score_network);
			this->branch_score_network->activate_small(local_s_input_vals,
													   local_state_vals,
													   branch_score_network_history);
			history->branch_score_network_history = branch_score_network_history;
			history->branch_score_update = this->branch_score_network->output->acti_vals[0];
		}

		FoldNetworkHistory* best_history = NULL
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			if (this->is_branch[b_index]) {
				FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
				this->score_networks[b_index]->activate_small(local_s_input_vals,
															  local_state_vals,
															  curr_history);
				double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
				if (curr_score > best_score) {
					best_score = curr_score;
					best_index = b_index;
					if (best_history != NULL) {
						delete best_history;
					}
					best_history = curr_history;
				} else {
					delete curr_history;
				}
			}
		}
		history->score_network_history = best_history;
	} else {
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			if (this->is_branch[b_index]) {
				this->score_networks[b_index]->activate_small(local_s_input_vals,
															  local_state_vals);
				double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
				if (curr_predicted_score > best_score) {
					best_score = curr_score;
					best_index = b_index;
				}
			}
		}
	}
	history->best_score = best_score;
	history->best_index = best_index;
}

void Branch::explore_off_path_activate_score(vector<double>& local_s_input_vals,
											 vector<double>& local_state_vals,
											 double& scale_factor,
											 int& explore_phase,
											 BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	if (explore_phase == EXPLORE_PHASE_FLAT) {
		if (!this->passed_branch_score) {
			FoldNetworkHistory* branch_score_network_history = new FoldNetworkHistory(this->branch_score_network);
			this->branch_score_network->activate_small(local_s_input_vals,
													   local_state_vals,
													   branch_score_network_history);
			history->branch_score_network_history = branch_score_network_history;
			history->branch_score_update = this->branch_score_network->output->acti_vals[0];
		}

		FoldNetworkHistory* best_history = NULL
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
			this->score_networks[b_index]->activate_small(local_s_input_vals,
														  local_state_vals,
														  curr_history);
			double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
			if (curr_score > best_score) {
				best_score = curr_score;
				best_index = b_index;
				if (best_history != NULL) {
					delete best_history;
				}
				best_history = curr_history;
			} else {
				delete curr_history;
			}
		}
		history->score_network_history = best_history;
	} else {
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			this->score_networks[b_index]->activate_small(local_s_input_vals,
														  local_state_vals);
			double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
			if (curr_predicted_score > best_score) {
				best_score = curr_score;
				best_index = b_index;
			}
		}
	}
	history->best_score = best_score;
	history->best_index = best_index;
}

void Branch::explore_on_path_activate(vector<vector<double>>& flat_vals,
									  vector<double>& local_s_input_vals,
									  vector<double>& local_state_vals,
									  double& predicted_score,
									  double& scale_factor,
									  int& explore_phase,
									  BranchHistory* history) {
	BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
	this->branches[history->best_index]->explore_on_path_activate(flat_vals,
																  history->best_score,
																  local_s_input_vals,
																  local_state_vals,
																  predicted_score,
																  scale_factor,
																  explore_phase,
																  branch_path_history);
	history->branch_path_history = branch_path_history;

	scale_factor *= this->end_scale_mods[history->best_index];
}

void Branch::explore_off_path_activate(vector<vector<double>>& flat_vals,
									   vector<double>& local_s_input_vals,
									   vector<double>& local_state_vals,
									   double& predicted_score,
									   double& scale_factor,
									   int& explore_phase,
									   BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[history->best_index]->explore_off_path_activate(flat_vals,
																	   history->best_score,
																	   local_s_input_vals,
																	   local_state_vals,
																	   predicted_score,
																	   scale_factor,
																	   explore_phase,
																	   branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[history->best_index]);
		this->folds[history->best_index]->explore_off_path_activate(flat_vals,
																	history->best_score,
																	local_s_input_vals,
																	local_state_vals,
																	predicted_score,
																	scale_factor,
																	explore_phase,
																	fold_history);
		history->fold_history = fold_history;
	}

	scale_factor *= this->end_scale_mods[history->best_index];
}

void Branch::explore_on_path_backprop(vector<double>& local_state_errors,
									  double& predicted_score,
									  double target_val,
									  double& scale_factor,
									  double& scale_factor_error,
									  BranchHistory* history) {
	scale_factor /= this->end_scale_mods[history->best_index];
	scale_factor_error *= this->end_scale_mods[history->best_index];

	this->branches[history->best_index]->explore_on_path_backprop(local_state_errors,
																  predicted_score,
																  target_val,
																  scale_factor,
																  scale_factor_error,
																  history->branch_path_history);

	return;
}

void Branch::explore_off_path_backprop(vector<double>& local_s_input_errors,
									   vector<double>& local_state_errors,
									   double& predicted_score,
									   double target_val,
									   double& scale_factor,
									   double& scale_factor_error,
									   BranchHistory* history) {
	scale_factor /= this->end_scale_mods[history->best_index];
	scale_factor_error *= this->end_scale_mods[history->best_index];

	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->explore_off_path_backprop(local_s_input_errors,
																	   local_state_errors,
																	   predicted_score,
																	   target_val,
																	   scale_factor,
																	   scale_factor_error,
																	   history->branch_path_history);
	} else {
		this->folds[history->best_index]->explore_off_path_backprop(local_s_input_errors,
																	local_state_errors,
																	predicted_score,
																	target_val,
																	scale_factor,
																	scale_factor_error,
																	history->fold_history);
	}

	// predicted_score already modified to before branch value in branch_path
	double score_predicted_score = predicted_score + history->best_score;
	double score_predicted_score_error = target_val - score_predicted_score;

	double score_update = history->best_score/scale_factor;
	scale_factor_error += score_update*score_predicted_score_error;

	vector<double> score_errors{scale_factor*score_predicted_score_error}
	vector<double> score_s_input_output_errors;
	vector<double> score_state_output_errors;
	this->score_networks[history->best_index][history->best_sub_index]->backprop_small_errors_with_no_weight_change(
		score_errors,
		score_s_input_output_errors,
		score_state_output_errors,
		history->score_network_history);
	// use output sizes as might not have used all inputs
	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
		local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
	}
	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
		local_state_errors[s_index] += score_state_output_errors[s_index];
	}

	// score_networks don't update predicted_score

	if (!this->passed_branch_score) {
		double branch_score_predicted_score = predicted_score + scale_factor*history->branch_score_update;
		double branch_score_predicted_score_error = target_val - branch_score_predicted_score;

		scale_factor_error += history->branch_score_update*branch_score_predicted_score_error;

		vector<double> branch_score_errors{scale_factor*branch_score_predicted_score_error};
		vector<double> branch_score_s_input_output_errors;
		vector<double> branch_score_state_output_errors;
		this->branch_score_network->backprop_small_errors_with_no_weight_change(
			branch_score_errors,
			branch_score_s_input_output_errors,
			branch_score_state_output_errors,
			history->branch_score_network_history);
		for (int s_index = 0; s_index < (int)branch_score_s_input_output_errors.size(); s_index++) {
			local_s_input_errors[s_index] += branch_score_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)branch_score_state_output_errors.size(); s_index++) {
			local_state_errors[s_index] += branch_score_state_output_errors[s_index];
		}
	}
}

void Branch::existing_flat_activate(vector<vector<double>>& flat_vals,
									vector<double>& local_s_input_vals,
									vector<double>& local_state_vals,
									double& predicted_score,
									double& scale_factor,
									BranchHistory* history) {
	// don't activate branch_score as will not backprop it

	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	FoldNetworkHistory* best_history = NULL
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
		this->score_networks[b_index]->activate_small(local_s_input_vals,
													  local_state_vals,
													  curr_history);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
		if (curr_score > best_score) {
			best_score = curr_score;
			best_index = b_index;
			if (best_history != NULL) {
				delete best_history;
			}
			best_history = curr_history;
		} else {
			delete curr_history;
		}
	}
	history->score_network_history = best_history;

	history->best_score = best_score;
	history->best_index = best_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->existing_flat_activate(flat_vals,
														   best_score,
														   local_s_input_vals,
														   local_state_vals,
														   predicted_score,
														   scale_factor,
														   branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->existing_flat_activate(flat_vals,
														best_score,
														local_s_input_vals,
														local_state_vals,
														predicted_score,
														scale_factor,
														fold_history);
		history->fold_history = fold_history;
	}

	scale_factor *= this->end_scale_mods[history->best_index];
}

void Branch::existing_flat_backprop(vector<double>& local_s_input_errors,
									vector<double>& local_state_errors,
									double& predicted_score,
									double predicted_score_error,
									double& scale_factor,
									double& scale_factor_error,
									BranchHistory* history) {
	scale_factor /= this->end_scale_mods[history->best_index];
	scale_factor_error *= this->end_scale_mods[history->best_index];

	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->existing_flat_backprop(local_s_input_errors,
																	local_state_errors,
																	predicted_score,
																	predicted_score_error,
																	scale_factor,
																	scale_factor_error,
																	history->branch_path_history);
	} else {
		this->folds[history->best_index]->existing_flat_backprop(local_s_input_errors,
																 local_state_errors,
																 predicted_score,
																 predicted_score_error,
																 scale_factor,
																 scale_factor_error,
																 history->fold_history);
	}

	// TODO: score_network may not have direct connection to predicted_score_error, so examine if is OK

	double score_update = history->best_score/scale_factor;
	scale_factor_error += score_update*predicted_score_error;

	vector<double> score_errors{scale_factor*predicted_score_error}
	vector<double> score_s_input_output_errors;
	vector<double> score_state_output_errors;
	this->score_networks[history->best_index]->backprop_small_errors_with_no_weight_change(
		score_errors,
		score_s_input_output_errors,
		score_state_output_errors,
		history->score_network_history);
	// use output sizes as might not have used all inputs
	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
		local_s_input_errors[s_index] += score_s_input_output_errors[s_index];
	}
	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
		local_state_errors[s_index] += score_state_output_errors[s_index];
	}

	// score_networks don't update predicted_score

	// branch_score_network has no direct connection to predicted_score_error (and has no impact for existing_flat)
}

void Branch::update_activate(vector<vector<double>>& flat_vals,
							 vector<double>& local_s_input_vals,
							 vector<double>& local_state_vals,
							 double& predicted_score,
							 double& scale_factor,
							 BranchHistory* history) {
	if (!this->passed_branch_score) {
		FoldNetworkHistory* branch_score_network_history = new FoldNetworkHistory(this->branch_score_network);
		this->branch_score_network->activate_small(local_s_input_vals,
												   local_state_vals,
												   branch_score_network_history);
		history->branch_score_network_history = branch_score_network_history;
		history->branch_score_update = this->branch_score_network->output->acti_vals[0];
	}

	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	FoldNetworkHistory* best_history = NULL
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
		this->score_networks[b_index]->activate_small(local_s_input_vals,
													  local_state_vals,
													  curr_history);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
		if (curr_score > best_score) {
			best_score = curr_score;
			best_index = b_index;
			if (best_history != NULL) {
				delete best_history;
			}
			best_history = curr_history;
		} else {
			delete curr_history;
		}
	}
	history->score_network_history = best_history;

	history->best_score = best_score;
	history->best_index = best_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->update_activate(flat_vals,
													best_score,
													local_s_input_vals,
													local_state_vals,
													predicted_score,
													scale_factor,
													branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->update_activate(flat_vals,
												 best_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 fold_history);
		history->fold_history = fold_history;
	}

	scale_factor *= this->end_scale_mods[history->best_index];
}

void Branch::update_backprop(double& predicted_score,
							 double target_val,
							 double& scale_factor,
							 BranchHistory* history) {
	scale_factor /= this->end_scale_mods[history->best_index];

	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->update_backprop(predicted_score,
															 target_val,
															 scale_factor,
															 history->branch_path_history);
	} else {
		this->folds[history->best_index]->update_backprop(predicted_score,
														  target_val,
														  scale_factor,
														  history->fold_history);

		if (this->folds[history->best_index]->state == STATE_DONE) {
			resolve_fold(history->best_index);
		}
	}

	// predicted_score already modified to before branch value in branch_path
	double score_predicted_score = predicted_score + history->best_score;
	double score_predicted_score_error = target_val - score_predicted_score;

	vector<double> score_errors{scale_factor*score_predicted_score_error}
	this->score_networks[history->best_index]->backprop_small_weights_with_no_error_signal(
		score_errors,
		0.001,
		history->score_network_history);

	// score_networks don't update predicted_score

	if (!this->passed_branch_score) {
		double branch_score_predicted_score = predicted_score + scale_factor*history->branch_score_update;
		double branch_score_predicted_score_error = target_val - branch_score_predicted_score;

		vector<double> branch_score_errors{scale_factor*branch_score_predicted_score_error};
		this->branch_score_network->backprop_small_weights_with_no_error_signal(
			branch_score_errors,
			0.001,
			history->branch_score_network_history);
	}
}

void Branch::existing_update_activate(vector<vector<double>>& flat_vals,
									  vector<double>& local_s_input_vals,	// i.e., input
									  vector<double>& local_state_vals,	// i.e., output
									  double& predicted_score,
									  double& scale_factor,
									  BranchHistory* history) {
	// don't activate branch_score as will not backprop it

	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		this->score_networks[b_index]->activate_small(local_s_input_vals,
													  local_state_vals);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
		if (curr_score > best_score) {
			best_score = curr_score;
			best_index = b_index;
		}
	}

	history->best_score = best_score;
	history->best_index = best_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->existing_update_activate(flat_vals,
															 best_score,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->existing_update_activate(flat_vals,
														  best_score,
														  local_s_input_vals,
														  local_state_vals,
														  predicted_score,
														  scale_factor,
														  fold_history);
		history->fold_history = fold_history;
	}

	scale_factor *= this->end_scale_mods[history->best_index];
}

void Branch::existing_update_backprop(double& predicted_score,
									  double predicted_score_error,
									  double& scale_factor,
									  double& scale_factor_error,
									  BranchHistory* history) {
	scale_factor /= this->end_scale_mods[history->best_index];
	scale_factor_error *= this->end_scale_mods[history->best_index];

	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->existing_update_backprop(predicted_score,
																	  predicted_score_error,
																	  scale_factor,
																	  scale_factor_error,
																	  history->branch_path_history);
	} else {
		this->folds[history->best_index]->existing_update_backprop(predicted_score,
																   predicted_score_error,
																   scale_factor,
																   scale_factor_error,
																   history->fold_history);
	}

	// TODO: score_network may not have direct connection to predicted_score_error, so examine if is OK
	double score_update = history->best_score/scale_factor;
	scale_factor_error += score_update*predicted_score_error;

	// score_networks don't update predicted_score
}

void Branch::resolve_fold(int b_index) {

}
