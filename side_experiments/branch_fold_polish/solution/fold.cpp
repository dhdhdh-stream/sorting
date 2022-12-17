#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

Fold::Fold(int sequence_length,
		   vector<bool> is_existing,
		   vector<Scope*> existing_actions,
		   vector<int> obs_sizes,
		   int output_size,
		   double* existing_misguess,
		   int starting_s_input_size,
		   int starting_state_size) {
	this->sequence_length = sequence_length;
	this->is_existing = is_existing;
	this->existing_actions = existing_actions;
	this->obs_sizes = obs_sizes;
	this->output_size = output_size;

	this->average_misguess = 0.0;
	this->existing_misguess = existing_misguess;
	this->average_misguess_standard_deviation = 0.0;

	this->starting_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   starting_state_size,
												   20);

	this->replace_existing = 0.0;
	this->replace_existing_standard_deviation = 0.0;
	this->replace_combined = 0.0;
	this->replace_combined_standard_deviation = 0.0;

	this->combined_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   starting_state_size,
												   20);

	this->combined_improvement = 0.0;
	this->combined_standard_deviation = 0.0;

	this->scope_scale_mod_calcs = vector<Network*>(this->sequence_length, NULL);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			this->scope_scale_mod_calcs[f_index] = new Network(0, 0, 1);
			this->scope_scale_mod_calcs[f_index]->output->constants[0] = 1.0;
		}
	}

	this->end_scale_mod_calc = new Network(0, 0, 1);
	this->end_scale_mod_calc->output->constants[0] = 1.0;

	this->curr_s_input_sizes.push_back(starting_s_input_size);
	this->curr_scope_sizes.push_back(starting_state_size);

	vector<double> flat_sizes;
	vector<vector<double>> input_flat_sizes(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		int flat_size;
		if (!this->is_existing[f_index]) {
			flat_size = this->obs_sizes[f_index];
		} else {
			flat_size = this->existing_actions[f_index]->num_outputs;
		}
		flat_sizes.push_back(flat_size);
		for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
			if (this->is_existing[ff_index]) {
				input_flat_sizes[ff_index].push_back(flat_size);
			}
		}
	}

	this->curr_fold = new FoldNetwork(flat_sizes,
									  1,
									  starting_s_input_size,
									  starting_state_size,
									  100);
	this->curr_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			this->curr_input_folds[f_index] = new FoldNetwork(input_flat_sizes[f_index],
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

	int starting_compress_original_size = starting_state_size;
	int curr_starting_compress_new_size = starting_state_size;
	this->curr_starting_compress_network = NULL;
	this->test_starting_compress_network = NULL;

	this->test_fold = NULL;
	this->test_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
	this->test_end_fold = NULL;

	this->curr_input_network = NULL;
	this->test_input_network = NULL;

	this->curr_score_network = NULL;
	this->test_score_network = NULL;

	this->curr_compress_network = NULL;
	this->test_compress_network = NULL;

	this->state_iter = 0;
	this->sum_error = 0.0;
}

Fold::~Fold() {
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			delete this->existing_actions[f_index];
		}
	}

	if (this->starting_score_network != NULL) {
		delete this->starting_score_network;
	}
	if (this->combined_score_network != NULL) {
		delete this->combined_score_network;
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
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
		if (this->is_existing[f_index]) {
			if (this->curr_input_folds[f_index] != NULL) {
				delete this->curr_input_folds[f_index];
			}
		}
	}
	if (this->curr_end_fold != NULL) {
		delete this->curr_end_fold;
	}

	if (this->curr_starting_compress_network != NULL) {
		delete this->curr_starting_compress_network;
	}
	if (this->test_starting_compress_network != NULL) {
		delete this->test_starting_compress_network;
	}

	if (this->test_fold != NULL) {
		delete this->test_fold;
	}
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			if (this->test_input_folds[f_index] != NULL) {
				delete this->test_input_folds[f_index];
			}
		}
	}
	if (this->test_end_fold != NULL) {
		delete this->test_end_fold;
	}

	for (int i_index = 0; i_index < (int)this->inner_input_input_networks.size(); i_index++) {
		delete this->inner_input_input_networks[i_index];
	}

	if (this->curr_input_network != NULL) {
		delete this->curr_input_network;
	}
	if (this->test_input_network != NULL) {
		delete this->test_input_network;
	}

	if (this->curr_score_network != NULL) {
		delete this->curr_score_network;
	}
	if (this->test_score_network != NULL) {
		delete this->test_score_network;
	}

	if (this->curr_compress_network != NULL) {
		delete this->curr_compress_network;
	}
	if (this->test_compress_network != NULL) {
		delete this->test_compress_network;
	}

	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		delete this->input_networks[i_index];
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

int Fold::explore_on_path_backprop(vector<double>& local_state_errors,
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
	if (this->state_iter == 495000) {
		this->average_misguess /= 5000;
		this->replace_existing /= 5000;
		this->replace_combined /= 5000;
		this->combined_improvement /= 5000;
	} else if (this->state_iter == 500000) {
		this->combined_standard_deviation =
			sqrt(this->combined_standard_deviation/4999);
		double combined_t_value = this->combined_improvement
			/ (this->combined_standard_deviation / sqrt(5000));
		if (this->combined_improvement > 0.0 && combined_t_value > 2.576) {	// > 99%
			this->replace_combined_standard_deviation =
				sqrt(this->replace_combined_standard_deviation/4999);
			double replace_combined_t_value = this->replace_combined
				/ (this->replace_combined_standard_deviation / sqrt(5000));
			if (this->replace_combined > 0.0
					|| abs(replace_combined_t_value) < 1.960) {	// 95%<
				flat_to_fold();

				return EXPLORE_SIGNAL_REPLACE;
			} else {
				flat_to_fold();

				return EXPLORE_SIGNAL_BRANCH;
			}
		} else {
			this->replace_existing_standard_deviation =
				sqrt(this->replace_existing_standard_deviation/4999);
			double replace_existing_t_value = this->replace_existing
				/ (this->replace_existing_standard_deviation / sqrt(5000));

			this->average_misguess_standard_deviation =
				sqrt(this->average_misguess_standard_deviation/4999);
			double average_misguess_t_value = this->average_misguess
				/ (this->average_misguess_standard_deviation / sqrt(5000));

			if ((this->replace_existing > 0.0 || abs(replace_existing_t_value) < 1.960)	// 95%<
					&& (this->average_misguess > 0.0 && average_misguess_t_value > 2.576)) {
				flat_to_fold();

				return EXPLORE_SIGNAL_REPLACE;
			} else {
				return EXPLORE_SIGNAL_CLEAN;
			}
		}
	} else {
		return EXPLORE_SIGNAL_NONE;
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_explore_off_path_activate(flat_vals,
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
		case STATE_COMPRESS_STATE:
			compress_step_explore_off_path_activate(flat_vals,
													starting_score,
													local_s_input_vals,
													local_state_vals,
													predicted_score,
													scale_factor,
													explore_phase,
													history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_explore_off_path_activate(flat_vals,
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_explore_off_path_backprop(local_s_input_errors,
															 local_state_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 scale_factor_error,
															 history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_explore_off_path_backprop(local_s_input_errors,
													local_state_errors,
													predicted_score,
													target_val,
													scale_factor,
													scale_factor_error,
													history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_explore_off_path_backprop(local_s_input_errors,
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_existing_flat_activate(flat_vals,
														  starting_score,
														  local_s_input_vals,
														  local_state_vals,
														  predicted_score,
														  scale_factor,
														  history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_existing_flat_activate(flat_vals,
												 starting_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_existing_flat_activate(flat_vals,
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_existing_flat_backprop(local_s_input_errors,
														  local_state_errors,
														  predicted_score,
														  predicted_score_error,
														  scale_factor,
														  scale_factor_error,
														  history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_existing_flat_backprop(local_s_input_errors,
												 local_state_errors,
												 predicted_score,
												 predicted_score_error,
												 scale_factor,
												 scale_factor_error,
												 history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_existing_flat_backprop(local_s_input_errors,
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_update_activate(flat_vals,
												   starting_score,
												   local_s_input_vals,
												   local_state_vals,
												   predicted_score,
												   scale_factor,
												   history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_update_activate(flat_vals,
										  starting_score,
										  local_s_input_vals,
										  local_state_vals,
										  predicted_score,
										  scale_factor,
										  history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_update_activate(flat_vals,
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
		// can't be STATE_STEP_ADDED
	}
}

void Fold::update_backprop(double& predicted_score,
						   double& next_predicted_score,
						   double target_val,
						   double& scale_factor,
						   FoldHistory* history) {
	switch (this->last_state) {
		case STATE_STARTING_COMPRESS:
			starting_compress_step_update_backprop(predicted_score,
												   next_predicted_score,
												   target_val,
												   scale_factor,
												   history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_update_backprop(predicted_score,
										  next_predicted_score,
										  target_val,
										  scale_factor,
										  history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_update_backprop(predicted_score,
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
		// can't be STATE_STEP_ADDED
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_existing_update_activate(flat_vals,
															starting_score,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_existing_update_activate(flat_vals,
												   starting_score,
												   local_s_input_vals,
												   local_state_vals,
												   predicted_score,
												   scale_factor,
												   history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_existing_update_activate(flat_vals,
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
		case STATE_STARTING_COMPRESS:
			starting_compress_step_existing_update_backprop(predicted_score,
															predicted_score_error,
															scale_factor,
															scale_factor_error,
															history);
			break;
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
		case STATE_COMPRESS_STATE:
			compress_step_existing_update_backprop(predicted_score,
												   predicted_score_error,
												   scale_factor,
												   scale_factor_error,
												   history);
			break;
		case STATE_COMPRESS_SCOPE:
			compress_step_existing_update_backprop(predicted_score,
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
	this->state_iter++;

	if (this->state == STATE_STARTING_COMPRESS) {
		// TODO: experiment with lowering to 100000
		if (this->state_iter == 300000) {
			starting_compress_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_INNER_SCOPE_INPUT) {
		if (this->state_iter == 50000) {
			this->new_state_factor = 5;
		} else if (this->state_iter == 100000) {
			this->new_state_factor = 1;
		}

		if (this->state_iter == 300000) {
			inner_scope_input_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE) {
		if (this->state_iter == 300000) {
			score_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_STATE) {
		if (this->state_iter == 50000) {
			this->new_state_factor = 5;
		} else if (this->state_iter == 100000) {
			this->new_state_factor = 1;
		}

		if (this->state_iter == 300000) {
			compress_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_SCOPE) {
		if (this->state_iter == 50000) {
			this->new_state_factor = 5;
		} else if (this->state_iter == 100000) {
			this->new_state_factor = 1;
		}

		if (this->state_iter == 300000) {
			compress_scope_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == STATE_INPUT
		if (this->state_iter == 50000) {
			this->new_state_factor = 5;
		} else if (this->state_iter == 100000) {
			this->new_state_factor = 1;
		}

		if (this->state_iter == 300000) {
			input_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	}
}
