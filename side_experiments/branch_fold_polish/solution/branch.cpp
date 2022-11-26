#include "branch.h"

#include <iostream>
#include <limits>

using namespace std;



void Branch::explore_on_path_activate_score(vector<double>& s_input_vals,
											vector<double>& input_state_vals,
											double& scale_factor,
											int& explore_phase,
											BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	if (explore_phase == EXPLORE_PHASE_FLAT) {
		if (!passed_branch_score) {
			FoldNetworkHistory* branch_score_network_history = new FoldNetworkHistory(this->branch_score_network);
			this->branch_score_network->activate_small(s_input_vals,
													   input_state_vals,
													   branch_score_network_history);
			history->branch_score_network_history = branch_score_network_history;
		}

		FoldNetworkHistory* best_history = NULL
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			if (this->is_branch[b_index]) {
				FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
				this->score_networks[b_index]->activate_small(s_input_vals,
															  input_state_vals,
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
				this->score_networks[b_index]->activate_small(input_state_vals,
																	   s_input_vals);
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

void Branch::explore_off_path_activate_score(vector<double>& s_input_vals,
											 vector<double>& input_state_vals,
											 double& scale_factor,
											 int& explore_phase,
											 BranchHistory* history) {
	// *** HERE *** //
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	double best_sub_index = -1;
	if (explore_phase == EXPLORE_PHASE_FLAT) {
		FoldNetworkHistory* best_history = NULL
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
				FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index][n_index]);
				this->score_networks[b_index][n_index]->activate_small(s_input_vals,
																	   input_state_vals,
																	   curr_history);
				double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
				if (curr_score > best_score) {
					best_score = curr_score;
					best_index = b_index;
					best_sub_index = n_index;
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
			for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
				this->score_networks[b_index][n_index]->activate_small(input_state_vals,
																	   s_input_vals);
				double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
				if (curr_predicted_score > best_score) {
					best_score = curr_score;
					best_index = b_index;
					best_sub_index = n_index;
				}
			}
		}
	}
	history->best_score = best_score;
	history->best_index = best_index;
	history->best_sub_index = best_sub_index;
}

void Branch::explore_on_path_activate(vector<vector<double>>& flat_vals,
									  vector<double>& s_input_vals,
									  vector<double>& input_state_vals,
									  vector<double>& output_state_vals,
									  double& predicted_score,
									  double& scale_factor,
									  int& explore_phase,
									  Fold*& flat_ref,
									  BranchHistory* history) {
	BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
	this->branches[history->best_index]->explore_on_path_activate(flat_vals,
																  history->best_score,
																  history->best_sub_index,
																  s_input_vals,
																  input_state_vals,
																  output_state_vals,
																  predicted_score,
																  scale_factor,
																  explore_phase,
																  flat_ref,
																  branch_path_history);
	history->branch_path_history = branch_path_history;
}

void Branch::explore_off_path_activate(vector<vector<double>>& flat_vals,
									   vector<double>& s_input_vals,
									   vector<double>& input_state_vals,
									   vector<double>& output_state_vals,
									   double& predicted_score,
									   double& scale_factor,
									   int& explore_phase,
									   BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[history->best_index]->explore_off_path_activate(flat_vals,
																	   history->best_score,
																	   history->best_sub_index,
																	   s_input_vals,
																	   input_state_vals,
																	   output_state_vals,
																	   predicted_score,
																	   scale_factor,
																	   explore_phase,
																	   branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[history->best_index]);
		this->folds[history->best_index]->activate(history->best_score,	// doesn't matter
												   flat_vals,
												   s_input_vals,
												   input_state_vals,
												   output_state_vals,
												   predicted_score,
												   scale_factor,
												   explore_phase,
												   fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::explore_on_path_backprop(vector<double> state_input_errors,
									  double& predicted_score,
									  double target_val,
									  double& scale_factor,
									  double& average_factor_error,
									  double& scale_factor_error,
									  BranchHistory* history) {
	this->branches[history->best_index]->explore_on_path_backprop(state_input_errors,
																  predited_score,
																  target_val,
																  scale_factor,
																  average_factor_error,
																  scale_factor_error,
																  history->branch_path_history);

	return;
}

void Branch::explore_off_path_backprop(vector<double>& s_input_errors,
									   vector<double> state_input_errors,
									   vector<double>& state_output_errors,
									   double& predicted_score,
									   double target_val,
									   double& scale_factor,
									   double& average_factor_error,
									   double& scale_factor_error,
									   BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->explore_off_path_backprop(s_input_errors,
																	   state_input_errors,
																	   state_output_errors,
																	   predited_score,
																	   target_val,
																	   scale_factor,
																	   average_factor_error,
																	   scale_factor_error,
																	   history->branch_path_history);
	} else {
		this->folds[history->best_index]->explore_off_path_backprop(s_input_errors,
																	state_input_errors,
																	state_output_errors,
																	predicted_score,
																	target_val,
																	scale_factor,
																	average_factor_error,
																	scale_factor_error,
																	history->fold_history);
	}

	double predicted_score_error = target_val - predicted_score;

	average_factor_error += predicted_score_error;

	scale_factor_error += history->best_score*predicted_score_error;

	vector<double> score_errors{scale_factor*predicted_score_error}
	vector<double> score_s_input_output_errors;
	vector<double> score_state_output_errors;
	this->score_networks[history->best_index][history->best_sub_index]->backprop_small_errors_with_no_weight_change(
		score_errors,
		score_s_input_output_errors,
		score_state_output_errors,
		history->score_network_history);
	// use output sizes as might not have used all inputs
	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
		s_input_errors[s_index] += score_s_input_output_errors[s_index];
	}
	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
		state_output_errors[s_index] += score_state_output_errors[s_index];
	}

	predicted_score -= scale_factor*history->best_score;
}

void Branch::existing_flat_activate(vector<vector<double>>& flat_vals,
									vector<double>& s_input_vals,
									vector<double>& input_state_vals,
									vector<double>& output_state_vals,
									double& predicted_score,
									double& scale_factor,
									BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	double best_sub_index = -1;
	FoldNetworkHistory* best_history = NULL
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
			FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index][n_index]);
			this->score_networks[b_index][n_index]->activate_small(s_input_vals,
																   input_state_vals,
																   curr_history);
			double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
			if (curr_score > best_score) {
				best_score = curr_score;
				best_index = b_index;
				best_sub_index = n_index;
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

	history->best_score = best_score;
	history->best_index = best_index;
	history->best_sub_index = best_sub_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->existing_flat_activate(flat_vals,
														   best_score,
														   best_sub_index,
														   s_input_vals,
														   input_state_vals,
														   output_state_vals,
														   predicted_score,
														   scale_factor,
														   branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->activate(best_score,	// doesn't matter
										  flat_vals,
										  s_input_vals,
										  input_state_vals,
										  output_state_vals,
										  predicted_score,
										  scale_factor,
										  fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::existing_flat_backprop(vector<double>& s_input_errors,
									vector<double> state_input_errors,
									vector<double>& state_output_errors,
									double& predicted_score,
									double predicted_score_error,
									double& scale_factor,
									double& average_factor_error,
									double& scale_factor_error,
									BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->explore_off_path_backprop(s_input_errors,
																	   state_input_errors,
																	   state_output_errors,
																	   predited_score,
																	   target_val,
																	   scale_factor,
																	   average_factor_error,
																	   scale_factor_error,
																	   history->branch_path_history);
	} else {
		this->folds[history->best_index]->explore_off_path_backprop(s_input_errors,
																	state_input_errors,
																	state_output_errors,
																	predicted_score,
																	target_val,
																	scale_factor,
																	average_factor_error,
																	scale_factor_error,
																	history->fold_history);
	}

	average_factor_error += predicted_score_error;

	scale_factor_error += history->best_score*predicted_score_error;

	vector<double> score_errors{scale_factor*predicted_score_error}
	vector<double> score_s_input_output_errors;
	vector<double> score_state_output_errors;
	this->score_networks[history->best_index][history->best_sub_index]->backprop_small_errors_with_no_weight_change(
		score_errors,
		score_s_input_output_errors,
		score_state_output_errors,
		history->score_network_history);
	// use output sizes as might not have used all inputs
	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
		s_input_errors[s_index] += score_s_input_output_errors[s_index];
	}
	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
		state_output_errors[s_index] += score_state_output_errors[s_index];
	}

	predicted_score -= scale_factor*history->best_score;
}

void Branch::update_activate(vector<vector<double>>& flat_vals,
							 vector<double>& s_input_vals,
							 vector<double>& input_state_vals,
							 vector<double>& output_state_vals,
							 double& predicted_score,
							 double& scale_factor,
							 BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	double best_sub_index = -1;
	FoldNetworkHistory* best_history = NULL
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
			FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index][n_index]);
			this->score_networks[b_index][n_index]->activate_small(s_input_vals,
																   input_state_vals,
																   curr_history);
			double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
			if (curr_score > best_score) {
				best_score = curr_score;
				best_index = b_index;
				best_sub_index = n_index;
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

	history->best_score = best_score;
	history->best_index = best_index;
	history->best_sub_index = best_sub_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->update_activate(flat_vals,
													best_score,
													best_sub_index,
													s_input_vals,
													input_state_vals,
													output_state_vals,
													predicted_score,
													scale_factor,
													branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->activate(best_score,	// doesn't matter
										  flat_vals,
										  s_input_vals,
										  input_state_vals,
										  output_state_vals,
										  predicted_score,
										  scale_factor,
										  fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::update_backprop(double& predicted_score,
							 double target_val,
							 double& scale_factor,
							 BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->update_backprop(predited_score,
															 target_val,
															 scale_factor,
															 history->branch_path_history);
	} else {
		this->folds[history->best_index]->update_backprop(predicted_score,
														  target_val,
														  scale_factor,
														  history->fold_history);
	}

	double predicted_score_error = target_val - predicted_score;

	vector<double> score_errors{scale_factor*predicted_score_error}
	this->score_networks[history->best_index][history->best_sub_index]->backprop_weights_with_no_error_signal(
		score_errors,
		0.001,
		history->score_network_history);

	predicted_score -= scale_factor*history->best_score;
}
