#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

Fold::Fold(int sequence_length,
		   vector<Scope*> existing_actions,
		   vector<int> obs_sizes,
		   int output_size,
		   double* existing_misguess,
		   int starting_s_input_size,
		   int starting_state_size) {
	this->sequence_length = sequence_length;
	this->obs_sizes = obs_sizes;	// for now, -1 also signals is existing action
	this->existing_actions = existing_actions;
	this->output_size = output_size;

	this->existing_misguess = existing_misguess;

	this->starting_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   starting_state_size,
												   20);

	this->replace_improvement = 0.0;

	// don't worry about starting_compress_network initially

	this->combined_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   starting_state_size,
												   20);

	this->combined_improvement = 0.0;

	this->scope_scale_mod_calcs = vector<Network*>(this->sequence_length, NULL);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->scope_scale_mod_calcs[f_index] = new Network(0, 0, 1);
			this->scope_scale_mod_calcs[f_index]->output->constants[0] = 1.0;
		}
	}

	this->end_scale_mod_calc = new Network(0, 0, 1);
	this->end_scale_mod_calc->output->constants[0] = 0.0;

	this->curr_s_input_sizes.push_back(starting_s_input_size);
	this->curr_scope_sizes.push_back(starting_state_size);

	vector<double> flat_sizes;
	vector<vector<double>> input_flat_sizes(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		int flat_size;
		if (this->existing_actions[f_index] == NULL) {
			flat_size = this->obs_sizes[f_index];
		} else {
			flat_size = this->existing_actions[f_index]->num_outputs;
		}
		flat_sizes.push_back(flat_size);
		for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
			if (this->existing_actions[ff_index] == NULL) {
				input_flat_sizes[ff_index].push_back(flat_size);
			}
		}
	}

	this->curr_fold = new FoldNetwork(flat_sizes,
									  1,
									  starting_s_input_size,
									  starting_state_size,
									  100);
	this->curr_scope_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->curr_scope_input_folds[f_index] = new FoldNetwork(input_flat_sizes[f_index],
																	this->existing_actions[f_index]->num_inputs,
																	starting_s_input_size,
																	starting_state_size,
																	50);
		}
	}
	this->curr_end_fold = new FoldNetwork(flat_sizes,
										  this->output_size,
										  starting_s_input_size,
										  starting_state_size,
										  50);

	this->test_fold = NULL;
	this->test_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
	this->test_end_fold = NULL;

	this->state = STATE_FLAT;
	this->state_iter = 0;
	this->sum_error = 0.0;
}

Fold::~Fold() {
	if (this->starting_score_network != NULL) {
		delete this->starting_score_network;
	}
	if (this->starting_compress_network != NULL) {
		delete this->starting_compress_network;
	}
	if (this->combined_score_network != NULL) {
		delete this->combined_score_network;
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			if (this->scope_scale_mod_calcs[f_index] != NULL) {
				delete this->scope_scale_mod_calcs[f_index];
			}
		}
	}

	if (this->end_scale_mod_calc != NULL) {
		delete this->end_scale_mod_calc;
	}

	if (this->curr_fold != NULL) {
		delete this->curr_fold;
	}
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			if (this->curr_input_folds[f_index] != NULL) {
				delete this->curr_input_folds[f_index];
			}
		}
	}
	if (this->curr_end_fold != NULL) {
		delete this->curr_end_fold;
	}

	if (this->test_fold != NULL) {
		delete this->test_fold;
	}
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			if (this->test_input_folds[f_index] != NULL) {
				delete this->test_input_folds[f_index];
			}
		}
	}
	if (this->test_end_fold != NULL) {
		delete this->test_end_fold;
	}
}

void Fold::explore_on_path_activate(double existing_score,
									vector<vector<double>>& flat_vals,
									vector<double>& local_s_input_vals,
									vector<double>& local_state_vals,
									double& predicted_score,
									double& scale_factor,
									FoldHistory* history) {
	flat_step_explore_on_path_activate(existing_score,
									   flat_vals,
									   local_s_input_vals,
									   local_state_vals,
									   predicted_score,
									   scale_factor,
									   history);
}

void Fold::explore_on_path_backprop(vector<double>& local_state_errors,
									double& predicted_score,
									double target_val,
									double& scale_factor,
									double& scale_factor_error,
									FoldHistory* history) {
	flat_step_explore_on_path_backprop(local_state_errors,
									   predicted_score,
									   target_val,
									   scale_factor,
									   scale_factor_error,
									   history);

	// explore_increment
	this->state_iter++;
	if (this->state_iter >= 500000) {
		if (this->combined_improvement > MIN_SCORE_GAIN) {
			if (this->replace_improvement > this->combined_improvement + MAX_SCORE_LOSS) {
				// replace

				this->last_state = state;
				this->state = STATE_INNER_SCOPE_INPUT;
				this->state_iter = 0;
				this->sum_error = 0.0;
			} else {
				// branch
			}
		} else if (this->replace_improvement > MAX_SCORE_LOSS) {
			if (this->average_misguess + MIN_INFO_GAIN < *this->existing_misguess) {
				// replace
			} else {
				// clean-up
			}
		} else {
			// clean-up
		}
	}
}

void Fold::explore_off_path_activate(vector<vector<double>>& flat_vals,
									 double starting_score,
									 vector<double>& local_s_input_vals,
									 vector<double>& local_state_vals,
									 double& predicted_score,
									 double& scale_factor,
									 int& explore_phase,
									 FoldHistory* history) {
	switch (this->last_state) {
		case STATE_FLAT:
			flat_step_explore_off_path_activate(flat_vals,
												starting_score,
												local_s_input_vals,
												local_state_vals,
												predicted_score,
												scale_factor,
												explore_phase,
												history);
			break;
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_explore_off_path_activate(flat_vals,
															 starting_score,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 explore_phase,
															 history);
			break;
		case STATE_SCORE:
			score_step_explore_off_path_activate(flat_vals,
												 starting_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 explore_phase,
												 history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_explore_off_path_activate(flat_vals,
													  starting_score,
													  local_s_input_vals,
													  local_state_vals,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_explore_off_path_activate(flat_vals,
														  starting_score,
														  local_s_input_vals,
														  local_state_vals,
														  predicted_score,
														  scale_factor,
														  explore_phase,
														  history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_explore_off_path_activate(flat_vals,
														  starting_score,
														  local_s_input_vals,
														  local_state_vals,
														  predicted_score,
														  scale_factor,
														  explore_phase,
														  history);
			break;
		case STATE_INPUT:
			input_step_explore_off_path_activate(flat_vals,
												 starting_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 explore_phase,
												 history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_explore_off_path_activate(flat_vals,
													  starting_score,
													  local_s_input_vals,
													  local_state_vals,
													  predicted_score,
													  scale_factor,
													  explore_phase,
													  history);
			break;
	}
}

void Fold::explore_off_path_backprop(vector<double>& local_s_input_errors,
									 vector<double>& local_state_errors,
									 double& predicted_score,
									 double target_val,
									 double& scale_factor,
									 double& scale_factor_error,
									 FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_explore_off_path_backprop(local_s_input_errors,
															 local_state_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 scale_factor_error,
															 history);
			break;
		case STATE_SCORE:
			score_step_explore_off_path_backprop(local_s_input_errors,
												 local_state_errors,
												 predicted_score,
												 target_val,
												 scale_factor,
												 scale_factor_error,
												 history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_explore_off_path_backprop(local_s_input_errors,
													  local_state_errors,
													  predicted_score,
													  target_val,
													  scale_factor,
													  scale_factor_error,
													  history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_explore_off_path_backprop(local_s_input_errors,
														  local_state_errors,
														  predicted_score,
														  target_val,
														  scale_factor,
														  scale_factor_error,
														  history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_explore_off_path_backprop(local_s_input_errors,
														  local_state_errors,
														  predicted_score,
														  target_val,
														  scale_factor,
														  scale_factor_error,
														  history);
			break;
		case STATE_INPUT:
			input_step_explore_off_path_backprop(local_s_input_errors,
												 local_state_errors,
												 predicted_score,
												 target_val,
												 scale_factor,
												 scale_factor_error,
												 history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_explore_off_path_backprop(local_s_input_errors,
													  local_state_errors,
													  predicted_score,
													  target_val,
													  scale_factor,
													  scale_factor_error,
													  history);
			break;
	}
}

void Fold::existing_flat_activate(vector<vector<double>>& flat_vals,
								  double starting_score,
								  vector<double>& local_s_input_vals,
								  vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_existing_flat_activate(flat_vals,
														  starting_score,
														  local_s_input_vals,
														  local_state_vals,
														  predicted_score,
														  scale_factor,
														  history);
			break;
		case STATE_SCORE:
			score_step_existing_flat_activate(flat_vals,
											  starting_score,
											  local_s_input_vals,
											  local_state_vals,
											  predicted_score,
											  scale_factor,
											  history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_existing_flat_activate(flat_vals,
												   starting_score,
												   local_s_input_vals,
												   local_state_vals,
												   predicted_score,
												   scale_factor,
												   history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_existing_flat_activate(flat_vals,
													   starting_score,
													   local_s_input_vals,
													   local_state_vals,
													   predicted_score,
													   scale_factor,
													   history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_existing_flat_activate(flat_vals,
													   starting_score,
													   local_s_input_vals,
													   local_state_vals,
													   predicted_score,
													   scale_factor,
													   history);
			break;
		case STATE_INPUT:
			input_step_existing_flat_activate(flat_vals,
											  starting_score,
											  local_s_input_vals,
											  local_state_vals,
											  predicted_score,
											  scale_factor,
											  history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_existing_flat_activate(flat_vals,
												   starting_score,
												   local_s_input_vals,
												   local_state_vals,
												   predicted_score,
												   scale_factor,
												   history);
			break;
	}
}

void Fold::existing_flat_backprop(vector<double>& local_s_input_errors,
								  vector<double>& local_state_errors,
								  double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_existing_flat_backprop(local_s_input_errors,
														  local_state_errors,
														  predicted_score,
														  predicted_score_error,
														  scale_factor,
														  scale_factor_error,
														  history);
			break;
		case STATE_SCORE:
			score_step_existing_flat_backprop(local_s_input_errors,
											  local_state_errors,
											  predicted_score,
											  predicted_score_error,
											  scale_factor,
											  scale_factor_error,
											  history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_existing_flat_backprop(local_s_input_errors,
												   local_state_errors,
												   predicted_score,
												   predicted_score_error,
												   scale_factor,
												   scale_factor_error,
												   history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_existing_flat_backprop(local_s_input_errors,
													   local_state_errors,
													   predicted_score,
													   predicted_score_error,
													   scale_factor,
													   scale_factor_error,
													   history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_existing_flat_backprop(local_s_input_errors,
													   local_state_errors,
													   predicted_score,
													   predicted_score_error,
													   scale_factor,
													   scale_factor_error,
													   history);
			break;
		case STATE_INPUT:
			input_step_existing_flat_backprop(local_s_input_errors,
											  local_state_errors,
											  predicted_score,
											  predicted_score_error,
											  scale_factor,
											  scale_factor_error,
											  history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_existing_flat_backprop(local_s_input_errors,
												   local_state_errors,
												   predicted_score,
												   predicted_score_error,
												   scale_factor,
												   scale_factor_error,
												   history);
			break;
	}
}

void Fold::update_activate(vector<vector<double>>& flat_vals,
						   double starting_score,
						   vector<double>& local_s_input_vals,
						   vector<double>& local_state_vals,
						   double& predicted_score,
						   double& scale_factor,
						   FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_update_activate(flat_vals,
												   starting_score,
												   local_s_input_vals,
												   local_state_vals,
												   predicted_score,
												   scale_factor,
												   history);
			break;
		case STATE_SCORE:
			score_step_update_activate(flat_vals,
									   starting_score,
									   local_s_input_vals,
									   local_state_vals,
									   predicted_score,
									   scale_factor,
									   history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_update_activate(flat_vals,
											starting_score,
											local_s_input_vals,
											local_state_vals,
											predicted_score,
											scale_factor,
											history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_update_activate(flat_vals,
												starting_score,
												local_s_input_vals,
												local_state_vals,
												predicted_score,
												scale_factor,
												history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_update_activate(flat_vals,
												starting_score,
												local_s_input_vals,
												local_state_vals,
												predicted_score,
												scale_factor,
												history);
			break;
		case STATE_INPUT:
			input_step_update_activate(flat_vals,
									   starting_score,
									   local_s_input_vals,
									   local_state_vals,
									   predicted_score,
									   scale_factor,
									   history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_update_activate(flat_vals,
											starting_score,
											local_s_input_vals,
											local_state_vals,
											predicted_score,
											scale_factor,
											history);
			break;
	}
}

void Fold::update_backprop(double& predicted_score,
						   double& next_predicted_score,
						   double target_val,
						   double& scale_factor,
						   FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_update_backprop(predicted_score,
												   next_predicted_score,
												   target_val,
												   scale_factor,
												   history);
			break;
		case STATE_SCORE:
			score_step_update_backprop(predicted_score,
									   next_predicted_score,
									   target_val,
									   scale_factor,
									   history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_update_backprop(predicted_score,
											next_predicted_score,
											target_val,
											scale_factor,
											history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_update_backprop(predicted_score,
												next_predicted_score,
												target_val,
												scale_factor,
												history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_update_backprop(predicted_score,
												next_predicted_score,
												target_val,
												scale_factor,
												history);
			break;
		case STATE_INPUT:
			input_step_update_backprop(predicted_score,
									   next_predicted_score,
									   target_val,
									   scale_factor,
									   history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_update_backprop(predicted_score,
											next_predicted_score,
											target_val,
											scale_factor,
											history);
			break;
	}

	fold_increment();
}

void Fold::existing_update_activate(vector<vector<double>>& flat_vals,
									double starting_score,
									vector<double>& local_s_input_vals,
									vector<double>& local_state_vals,
									double& predicted_score,
									double& scale_factor,
									FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_existing_update_activate(flat_vals,
															starting_score,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															history);
			break;
		case STATE_SCORE:
			score_step_existing_update_activate(flat_vals,
												starting_score,
												local_s_input_vals,
												local_state_vals,
												predicted_score,
												scale_factor,
												history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_existing_update_activate(flat_vals,
													 starting_score,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_existing_update_activate(flat_vals,
														 starting_score,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_existing_update_activate(flat_vals,
														 starting_score,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 history);
			break;
		case STATE_INPUT:
			input_step_existing_update_activate(flat_vals,
												starting_score,
												local_s_input_vals,
												local_state_vals,
												predicted_score,
												scale_factor,
												history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_existing_update_activate(flat_vals,
													 starting_score,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 history);
			break;
	}
}

void Fold::existing_update_backprop(double& predicted_score,
									double predicted_score_error,
									double& scale_factor,
									double& scale_factor_error,
									FoldHistory* history) {
	switch (this->last_state) {
		case STATE_INNER_SCOPE_INPUT:
			inner_scope_input_step_existing_update_backprop(predicted_score,
															predicted_score_error,
															scale_factor,
															scale_factor_error,
															history);
			break;
		case STATE_SCORE:
			score_step_existing_update_backprop(predicted_score,
												predicted_score_error,
												scale_factor,
												scale_factor_error,
												history);
			break;
		case STATE_SCORE_TUNE:
			score_tune_step_existing_update_backprop(predicted_score,
													 predicted_score_error,
													 scale_factor,
													 scale_factor_error,
													 history);
			break;
		case STATE_COMPRESS_STATE:
			compress_state_step_existing_update_backprop(predicted_score,
														 predicted_score_error,
														 scale_factor,
														 scale_factor_error,
														 history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_scope_step_existing_update_backprop(predicted_score,
														 predicted_score_error,
														 scale_factor,
														 scale_factor_error,
														 history);
			break;
		case STATE_INPUT:
			input_step_existing_update_backprop(predicted_score,
												predicted_score_error,
												scale_factor,
												scale_factor_error,
												history);
			break;
		case STATE_STEP_ADDED:
			step_added_step_existing_update_backprop(predicted_score,
													 predicted_score_error,
													 scale_factor,
													 scale_factor_error,
													 history);
			break;
	}
}

void Fold::fold_increment() {

}
