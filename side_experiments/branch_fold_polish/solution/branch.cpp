#include "branch.h"

#include <iostream>
#include <limits>

using namespace std;



void Branch::explore_on_path_activate(vector<vector<double>>& flat_vals,
									  vector<double>& s_input_vals,
									  vector<double>& input_state_vals,
									  vector<double>& output_state_vals,
									  double& predicted_score,
									  double& scale_factor,
									  int& explore_phase,
									  BranchHistory* history) {
	// this->explore_phase == EXPLORE_PHASE_NONE
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
				if (best_history != NULL) {
					delete best_history;
				}
				best_history = curr_history;
				best_index = b_index;
				best_sub_index = n_index;
			} else {
				delete curr_history;
			}
		}
	}
	history->score_network_history = best_history;
	history->best_index = best_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->explore_activate(flat_vals,
													 best_score,
													 best_sub_index,
													 s_input_vals,
													 input_state_vals,
													 output_state_vals,
													 predicted_score,
													 scale_factor,
													 explore_phase,
													 branch_path_history);
		history->branch_history = branch_history;
	} else {
		// first fold on explore
		FoldHistory* fold_history = new FoldHistory(this->explore_fold);
		this->explore_fold->activate(flat_vals,
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

void Branch::existing_activate(vector<vector<double>>& flat_vals,
							   vector<double>& s_input_vals,
							   vector<double>& input_state_vals,
							   vector<double>& output_state_vals,
							   double& predicted_score,
							   double& scale_factor,
							   int& explore_phase,
							   BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	double best_sub_index = -1;
	if (this->explore_phase == EXPLORE_PHASE_FLAT) {
		// don't include EXPLORE_PHASE_NONE as don't modify score network off path
		FoldNetworkHistory* best_history = NULL
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			// can fold multiple at once
			// TODO: but make sure to flat one at a time
			for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
				FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index][n_index]);
				this->score_networks[b_index][n_index]->activate_small(s_input_vals,
																	   input_state_vals,
																	   curr_history);
				double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
				if (curr_score > best_score) {
					best_score = curr_score;
					if (best_history != NULL) {
						delete best_history;
					}
					best_history = curr_history;
					best_index = b_index;
					best_sub_index = n_index;
				} else {
					delete curr_history;
				}
			}
		}
		history->score_network_history = best_history;
	} else {
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			// can fold multiple at once
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
	history->best_index = best_index;
	// *** didn't finish changes here *** //

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->explore_activate(flat_vals,
													 best_score,
													 best_sub_index,
													 s_input_vals,
													 input_state_vals,
													 output_state_vals,
													 predicted_score,
													 scale_factor,
													 explore_phase,
													 branch_path_history);
		history->branch_history = branch_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->explore_fold);
		this->explore_fold->activate(flat_vals,
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

// void Branch::activate(vector<vector<double>>& flat_vals,
// 					  vector<double>& input_state_vals,
// 					  vector<double>& s_input_vals,
// 					  vector<double>& output_state_vals,
// 					  double& predicted_score,
// 					  double& scale_factor) {
// 	double best_score = numeric_limits<double>::lowest();
// 	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
// 		this->score_networks[b_index]->activate_small(input_state_vals,
// 													  s_input_vals);
// 		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
// 		if (curr_predicted_score > best_score) {
// 			best_score = curr_score;
// 			this->best_index = b_index;
// 		}
// 	}

// 	this->branches[this->best_index]->activate(flat_vals,
// 											   scope_state_vals,
// 											   s_input_vals,
// 											   output_state_vals,
// 											   predicted_score,
// 											   scale_factor);

// 	predicted_score += scale_factor*this->end_averages_mods[this->best_index];
// 	scale_factor *= this->end_scale_mods[this->best_index];
// }

// void Branch::activate(vector<vector<double>>& flat_vals,
// 					  double inherited_score,
// 					  double inherited_branch_index,
// 					  vector<double>& input_state_vals,
// 					  vector<double>& s_input_vals,
// 					  vector<double>& output_state_vals,
// 					  double& predicted_score,
// 					  double& scale_factor) {
// 	double best_score = inherited_score;
// 	this->best_index = 0;

// 	for (int b_index = 1; b_index < (int)this->branches.size(); b_index++) {
// 		this->score_networks[b_index]->activate_small(input_state_vals,
// 													  s_input_vals);
// 		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];
// 		if (curr_predicted_score > best_score) {
// 			best_score = curr_score;
// 			this->best_index = b_index;
// 		}
// 	}

// 	this->branches[this->best_index]->activate(flat_vals,
// 											   scope_state_vals,
// 											   s_input_vals,
// 											   output_state_vals,
// 											   predicted_score,
// 											   scale_factor);

// 	predicted_score += scale_factor*this->end_averages_mods[this->best_index];
// 	scale_factor *= this->end_scale_mods[this->best_index];
// }

// void Branch::existing_backprop(vector<double> state_input_errors,
// 							   vector<double>& state_output_errors,
// 							   vector<double>& s_input_errors,
// 							   double& predicted_score,
// 							   double predicted_score_error,
// 							   double& scale_factor,
// 							   double new_scale_factor,
// 							   double& new_scale_factor_error) {
// 	scale_factor /= this->end_scale_mods[this->best_index];
// 	predicted_score -= scale_factor*this->end_averages_mods[this->best_index];
// 	// end_mods don't need to be accounted for in backprop as not changing them

// 	vector<double> branch_state_output_errors;
// 	this->branches[this->best_index]->existing_backprop(state_input_errors,
// 														s_input_errors,
// 														branch_state_output_errors,
// 														predited_score,
// 														predicted_score_error,
// 														scale_factor,
// 														new_scale_factor,
// 														new_scale_factor_error);

// 	if (this->compress_sizes[this->best_index] > 0) {
// 		int state_output_errors_size = branch_state_output_errors.size()+this->compress_sizes[this->best_index];
// 		state_output_errors = vector<double>(state_output_errors_size, 0.0);
// 		if (this->active_compress[this->best_index]) {
// 			vector<double> compress_state_output_errors;
// 			vector<double> compress_s_input_output_errors
// 			this->compress_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
// 				branch_state_output_errors,
// 				compress_state_output_errors,
// 				compress_s_input_output_errors);
// 			for (int s_index = 0; s_index < (int)compress_state_output_errors.size(); s_index++) {
// 				state_output_errors[s_index] += compress_state_output_errors[s_index];
// 			}
// 			// use output sizes as might not have used all inputs
// 			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
// 				s_input_errors[s_index] += compress_s_input_output_errors[s_index];
// 			}
// 		} else {
// 			for (int s_index = 0; s_index < (int)branch_state_output_errors.size(); s_index++) {
// 				// leave compressed s_input errors initialized at 0
// 				state_output_errors[s_index] += branch_state_output_errors[s_index];
// 			}
// 		}
// 	} else {
// 		state_output_errors = branch_state_output_errors;
// 	}

// 	// with only predicted_score_error, don't worry about average_factor yet

// 	double score_add_w_o_new_scale = scale_factor/new_scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
// 	new_scale_factor_error += score_add_w_o_new_scale*predicted_score_error;

// 	vector<double> score_errors{scale_factor*predicted_score_error}
// 	vector<double> score_state_output_errors;
// 	vector<double> score_s_input_output_errors;
// 	this->score_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
// 		score_errors,
// 		score_state_output_errors,
// 		score_s_input_output_errors);
// 	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
// 		state_output_errors[s_index] += score_state_output_errors[s_index];
// 	}
// 	// use output sizes as might not have used all inputs
// 	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
// 		s_input_errors[s_index] += score_s_input_output_errors[s_index];
// 	}

// 	predicted_score -= scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
// }

// void Branch::explore_activate(vector<vector<double>>& flat_vals,
// 							  vector<double>& s_input_vals,
// 							  vector<double>& input_state_vals,
// 							  vector<double>& output_state_vals,
// 							  double& predicted_score,
// 							  double& scale_factor,
// 							  int& explore_phase,
// 							  BranchHistory* history) {
// 	predicted_score += scale_factor*this->start_average_mod;
// 	scale_factor *= this->start_scale_mod;

// 	double best_score = numeric_limits<double>::lowest();
// 	double best_index = -1;
// 	double best_sub_index = -1;
// 	if (this->explore_phase == EXPLORE_PHASE_LEARN) {
// 		FoldNetworkHistory* best_history = NULL
// 		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
// 			for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
// 				FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index][n_index]);
// 				this->score_networks[b_index][n_index]->activate_small(input_state_vals,
// 																	   s_input_vals,
// 																	   curr_history);
// 				double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
// 				if (curr_predicted_score > best_score) {
// 					best_score = curr_score;
// 					if (best_history != NULL) {
// 						delete best_history;
// 					}
// 					best_history = curr_history;
// 					best_index = b_index;
// 					best_sub_index = n_index;
// 				} else {
// 					delete curr_history;
// 				}
// 			}
// 		}
// 	} else {
// 		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
// 			for (int n_index = 0; n_index < (int)this->score_networks[b_index].size(); n_index++) {
// 				this->score_networks[b_index][n_index]->activate_small(input_state_vals,
// 																	   s_input_vals);
// 				double curr_score = scale_factor*this->score_networks[b_index][n_index]->output->acti_vals[0];
// 				if (curr_predicted_score > best_score) {
// 					best_score = curr_score;
// 					best_index = b_index;
// 					best_sub_index = n_index;
// 				}
// 			}
// 		}
// 	}

// 	if (this->explore_type == EXPLORE_TYPE_LOCAL) {
// 		FoldHistory* fold_history = new FoldHistory(this->explore_fold);
// 		this->explore_fold->activate(flat_vals,
// 									 s_input_vals,
// 									 input_state_vals,
// 									 output_state_vals,
// 									 predicted_score,
// 									 scale_factor,
// 									 explore_phase,
// 									 new_scale_factor,
// 									 fold_history);
// 		history->fold_history = fold_history;
// 	} else {
// 		if (this->explore_phase == EXPLORE_PHASE_LEARN) {
// 			BranchHistory* branch_history = new BranchHistory(this->branches[best_index]);
// 			this->branches[best_index]->explore_activate(flat_vals,
// 														 best_score,
// 														 best_sub_index,
// 														 s_input_vals,
// 														 input_state_vals,
// 														 output_state_vals,
// 														 predicted_score,
// 														 scale_factor,
// 														 explore_phase,
// 														 new_scale_factor,
// 														 branch_history);
// 			history->branch_history = branch_history;
// 		} else {
// 			this->branches[best_index]->activate(flat_vals,
// 												 best_score,
// 												 best_sub_index,
// 												 s_input_vals,
// 												 input_state_vals,
// 												 output_state_vals,
// 												 predicted_score,
// 												 scale_factor);
// 		}
// 	}

// 	predicted_score += scale_factor*this->end_averages_mods[best_index];
// 	scale_factor *= this->end_scale_mods[best_index];
// }

// void Branch::explore_backprop(vector<double> state_input_errors,
// 							  vector<double>& state_output_errors,
// 							  vector<double>& s_input_errors,
// 							  double& predicted_score,
// 							  double target_val,
// 							  double& scale_factor,
// 							  // TODO: it's possible to keep a single value to update all
// 							  // only update branch and ancestors when backproping, as that is where scores are improved (everywhere else is an adjustment)
// 							  double& average_factor_error,
// 							  double& scale_factor_error) {
// 	scale_factor /= this->end_scale_mods[this->best_index];
// 	predicted_score -= scale_factor*this->end_averages_mods[this->best_index];
// 	// end_mods don't need to be accounted for in backprop as not changing them

// 	vector<double> branch_state_output_errors;
// 	this->branches[this->best_index]->explore_backprop(state_input_errors,
// 													   s_input_errors,
// 													   branch_state_output_errors,
// 													   predited_score,
// 													   target_val,
// 													   scale_factor,
// 													   new_average_factor_error,
// 													   new_scale_factor,
// 													   new_scale_factor_error);

// 	if (this->compress_sizes[this->best_index] > 0) {
// 		int state_output_errors_size = branch_state_output_errors.size()+this->compress_sizes[this->best_index];
// 		state_output_errors = vector<double>(state_output_errors_size, 0.0);
// 		if (this->active_compress[this->best_index]) {
// 			vector<double> compress_state_output_errors;
// 			vector<double> compress_s_input_output_errors
// 			this->compress_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
// 				branch_state_output_errors,
// 				compress_state_output_errors,
// 				compress_s_input_output_errors);
// 			for (int s_index = 0; s_index < (int)compress_state_output_errors.size(); s_index++) {
// 				state_output_errors[s_index] += compress_state_output_errors[s_index];
// 			}
// 			// use output sizes as might not have used all inputs
// 			for (int s_index = 0; s_index < (int)compress_s_input_output_errors.size(); s_index++) {
// 				s_input_errors[s_index] += compress_s_input_output_errors[s_index];
// 			}
// 		} else {
// 			for (int s_index = 0; s_index < (int)branch_state_output_errors.size(); s_index++) {
// 				// leave compressed s_input errors initialized at 0
// 				state_output_errors[s_index] += branch_state_output_errors[s_index];
// 			}
// 		}
// 	} else {
// 		state_output_errors = branch_state_output_errors;
// 	}

// 	double predicted_score_error = target_val - predicted_score;

// 	new_average_factor_error += predicted_score_error;

// 	double score_add_w_o_new_scale = scale_factor/new_scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
// 	new_scale_factor_error += score_add_w_o_new_scale*predicted_score_error;

// 	vector<double> score_errors{scale_factor*predicted_score_error}
// 	vector<double> score_state_output_errors;
// 	vector<double> score_s_input_output_errors;
// 	this->score_networks[this->best_index]->backprop_small_errors_with_no_weight_change(
// 		score_errors,
// 		score_state_output_errors,
// 		score_s_input_output_errors);
// 	for (int s_index = 0; s_index < (int)score_state_output_errors.size(); s_index++) {
// 		state_output_errors[s_index] += score_state_output_errors[s_index];
// 	}
// 	// use output sizes as might not have used all inputs
// 	for (int s_index = 0; s_index < (int)score_s_input_output_errors.size(); s_index++) {
// 		s_input_errors[s_index] += score_s_input_output_errors[s_index];
// 	}

// 	predicted_score -= scale_factor*this->score_networks[this->best_index]->output->acti_vals[0];
// }
