#include "fold.h"

#include <cmath>
#include <iostream>

#include "definitions.h"

using namespace std;

Fold::Fold(int sequence_length,
		   vector<bool> is_existing,
		   vector<Scope*> existing_actions,
		   vector<int> obs_sizes,
		   int output_size,
		   double* existing_misguess,
		   int starting_s_input_size,
		   int starting_state_size) {
	id_counter_mtx.lock();
	this->id = id_counter;
	id_counter++;
	id_counter_mtx.unlock();

	this->sequence_length = sequence_length;
	this->is_existing = is_existing;
	this->existing_actions = existing_actions;
	this->obs_sizes = obs_sizes;
	this->output_size = output_size;

	this->average_misguess = 0.0;

	this->standard_deviation = 0.0;

	this->existing_misguess = existing_misguess;
	this->misguess_improvement = 0.0;

	this->starting_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   vector<int>{starting_state_size},
												   20);

	this->replace_existing = 0.0;
	this->replace_combined = 0.0;

	this->combined_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   vector<int>{starting_state_size},
												   20);

	this->combined_improvement = 0.0;

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

	vector<int> flat_sizes;
	vector<vector<int>> input_flat_sizes(this->sequence_length);
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

	this->starting_compress_original_size = starting_state_size;
	this->curr_starting_compress_new_size = starting_state_size;
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

Fold::Fold(Fold* original) {
	id_counter_mtx.lock();
	this->id = id_counter;
	id_counter++;
	id_counter_mtx.unlock();

	this->sequence_length = original->sequence_length;
	this->is_existing = original->is_existing;
	this->existing_actions = original->existing_actions;
	this->obs_sizes = original->obs_sizes;
	this->output_size = original->output_size;

	for (int f_index = 0; f_index < (int)original->finished_steps.size(); f_index++) {
		this->finished_steps.push_back(new FinishedStep(original->finished_steps[f_index]));
	}

	// no need to set this->state, this->last_state, this->state_iter, this->sum_error

	// this->average_misguess has already been passed on

	// this->starting_score_network, this->combined_score_network has already been passed on

	for (int f_index = 0; f_index < (int)original->finished_steps.size(); f_index++) {
		this->scope_scale_mod_calcs.push_back(NULL);
	}
	for (int f_index = (int)original->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			this->scope_scale_mod_calcs.push_back(new Network(original->scope_scale_mod_calcs[f_index]));
		} else {
			this->scope_scale_mod_calcs.push_back(NULL);
		}
	}
	
	// this->end_scale_mod has already been passed on

	this->curr_s_input_sizes = original->curr_s_input_sizes;
	this->curr_scope_sizes = original->curr_scope_sizes;
	this->curr_fold = new FoldNetwork(original->curr_fold);
	for (int f_index = 0; f_index < (int)original->finished_steps.size(); f_index++) {
		this->curr_input_folds.push_back(NULL);
	}
	for (int f_index = (int)original->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			this->curr_input_folds.push_back(new FoldNetwork(original->curr_input_folds[f_index]));
		} else {
			this->curr_input_folds.push_back(NULL);
		}
	}
	this->curr_end_fold = new FoldNetwork(original->curr_end_fold);

	this->starting_average_misguess = original->starting_average_misguess;
	this->starting_average_local_impact = original->starting_average_local_impact;

	this->curr_starting_compress_new_size = original->curr_starting_compress_new_size;
	this->starting_compress_original_size = original->starting_compress_original_size;
	if (this->curr_starting_compress_new_size != this->starting_compress_original_size) {
		this->curr_starting_compress_network = new FoldNetwork(original->curr_starting_compress_network);
	}
	this->test_starting_compress_network = NULL;

	this->test_fold = NULL;
	this->test_end_fold = NULL;

	this->curr_input_network = NULL;
	this->test_input_network = NULL;

	this->curr_score_network = NULL;
	this->test_score_network = NULL;

	this->curr_compress_network = NULL;
	this->test_compress_network = NULL;

	restart_from_finished_step();
}

Fold::Fold(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string sequence_length_line;
	getline(input_file, sequence_length_line);
	this->sequence_length = stoi(sequence_length_line);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		string is_existing_line;
		getline(input_file, is_existing_line);
		this->is_existing.push_back(stoi(is_existing_line));

		if (this->is_existing[f_index]) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			int scope_id = stoi(scope_id_line);

			ifstream scope_save_file;
			scope_save_file.open("saves/scope_" + to_string(scope_id) + ".txt");
			this->existing_actions.push_back(new Scope(scope_save_file));
			scope_save_file.close();
		} else {
			this->existing_actions.push_back(NULL);
		}

		string obs_size_line;
		getline(input_file, obs_size_line);
		this->obs_sizes.push_back(stoi(obs_size_line));
	}

	string output_size_line;
	getline(input_file, output_size_line);
	this->output_size = stoi(output_size_line);

	string finished_steps_size_line;
	getline(input_file, finished_steps_size_line);
	int finished_step_size = stoi(finished_steps_size_line);

	for (int f_index = 0; f_index < finished_step_size; f_index++) {
		string finished_step_id_line;
		getline(input_file, finished_step_id_line);
		int finished_step_id = stoi(finished_step_id_line);

		ifstream finished_step_save_file;
		finished_step_save_file.open("saves/finished_step_" + to_string(finished_step_id) + ".txt");
		this->finished_steps.push_back(new FinishedStep(finished_step_save_file));
		finished_step_save_file.close();
	}

	// no need to set this->state, this->last_state, this->state_iter, this->sum_error

	// this->average_misguess has already been passed on

	// this->starting_score_network, this->combined_score_network has already been passed on

	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		this->scope_scale_mod_calcs.push_back(NULL);
	}
	for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			ifstream scope_scale_mod_calc_save_file;
			scope_scale_mod_calc_save_file.open("saves/nns/fold_" + to_string(this->id) + "_scope_scale_mod_" + to_string(f_index) + ".txt");
			this->scope_scale_mod_calcs.push_back(new Network(scope_scale_mod_calc_save_file));
			scope_scale_mod_calc_save_file.close();
		} else {
			this->scope_scale_mod_calcs.push_back(NULL);
		}
	}

	// this->end_scale_mod has already been passed on

	string curr_s_input_sizes_size_line;
	getline(input_file, curr_s_input_sizes_size_line);
	int curr_s_input_sizes_size = stoi(curr_s_input_sizes_size_line);

	for (int l_index = 0; l_index < curr_s_input_sizes_size; l_index++) {
		string curr_s_input_sizes_line;
		getline(input_file, curr_s_input_sizes_line);
		this->curr_s_input_sizes.push_back(stoi(curr_s_input_sizes_line));

		string curr_scope_sizes_line;
		getline(input_file, curr_scope_sizes_line);
		this->curr_scope_sizes.push_back(stoi(curr_scope_sizes_line));
	}

	ifstream curr_fold_save_file;
	curr_fold_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_fold.txt");
	this->curr_fold = new FoldNetwork(curr_fold_save_file);
	curr_fold_save_file.close();

	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		this->curr_input_folds.push_back(NULL);
	}
	for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			ifstream curr_input_fold_save_file;
			curr_input_fold_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_input_fold_" + to_string(f_index) + ".txt");
			this->curr_input_folds.push_back(new FoldNetwork(curr_input_fold_save_file));
			curr_input_fold_save_file.close();
		} else {
			this->curr_input_folds.push_back(NULL);
		}
	}

	ifstream curr_end_fold_save_file;
	curr_end_fold_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_end_fold.txt");
	this->curr_end_fold = new FoldNetwork(curr_end_fold_save_file);
	curr_end_fold_save_file.close();

	string starting_average_misguess_line;
	getline(input_file, starting_average_misguess_line);
	this->starting_average_misguess = stof(starting_average_misguess_line);

	string starting_average_local_impact_line;
	getline(input_file, starting_average_local_impact_line);
	this->starting_average_local_impact = stof(starting_average_local_impact_line);

	string curr_starting_compress_new_size_line;
	getline(input_file, curr_starting_compress_new_size_line);
	this->curr_starting_compress_new_size = stoi(curr_starting_compress_new_size_line);

	string starting_compress_original_size_line;
	getline(input_file, starting_compress_original_size_line);
	this->starting_compress_original_size = stoi(starting_compress_original_size_line);

	if (this->curr_starting_compress_new_size != this->starting_compress_original_size) {
		ifstream curr_starting_compress_network_save_file;
		curr_starting_compress_network_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_starting_compress.txt");
		this->curr_starting_compress_network = new FoldNetwork(curr_starting_compress_network_save_file);
		curr_starting_compress_network_save_file.close();
	}

	this->test_starting_compress_network = NULL;

	this->test_fold = NULL;
	this->test_end_fold = NULL;

	this->curr_input_network = NULL;
	this->test_input_network = NULL;

	this->curr_score_network = NULL;
	this->test_score_network = NULL;

	this->curr_compress_network = NULL;
	this->test_compress_network = NULL;

	restart_from_finished_step();
}

Fold::~Fold() {
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			delete this->existing_actions[f_index];
		}
	}

	for (int n_index = 0; n_index < (int)this->finished_steps.size(); n_index++) {
		delete this->finished_steps[n_index];
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
	if (this->state_iter == 500000) {
		this->standard_deviation = sqrt(this->standard_deviation/9999);

		this->misguess_improvement /= 10000;
		cout << "this->misguess_improvement: " << this->misguess_improvement << endl;

		this->replace_existing /= 10000;
		cout << "this->replace_existing: " << this->replace_existing << endl;
		this->replace_combined /= 10000;
		cout << "this->replace_combined: " << this->replace_combined << endl;
		this->combined_improvement /= 10000;
		cout << "this->combined_improvement: " << this->combined_improvement << endl;

		double combined_t_value = this->combined_improvement
			/ (standard_deviation / sqrt(10000));
		cout << "combined_t_value: " << combined_t_value << endl;
		if (this->combined_improvement > 0.0 && combined_t_value > 2.576) {	// > 99%
			double replace_combined_t_value = this->replace_combined
				/ (standard_deviation / sqrt(10000));
			cout << "replace_combined_t_value: " << replace_combined_t_value << endl;
			if (this->replace_combined > 0.0
					|| abs(replace_combined_t_value) < 1.960) {	// 95%<
				flat_to_fold();

				cout << "EXPLORE_SIGNAL_REPLACE" << endl;
				return EXPLORE_SIGNAL_REPLACE;
			} else {
				flat_to_fold();

				cout << "EXPLORE_SIGNAL_BRANCH" << endl;
				return EXPLORE_SIGNAL_BRANCH;
			}
		} else {
			double replace_existing_t_value = this->replace_existing
				/ (standard_deviation / sqrt(10000));
			cout << "replace_existing_t_value: " << replace_existing_t_value << endl;

			double misguess_improvement_t_value = this->misguess_improvement
				/ (standard_deviation / sqrt(10000));
			cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

			if ((this->replace_existing > 0.0 || abs(replace_existing_t_value) < 1.960)	// 95%<
					&& (this->misguess_improvement > 0.0 && misguess_improvement_t_value > 2.576)) {
				flat_to_fold();

				cout << "EXPLORE_SIGNAL_REPLACE" << endl;
				return EXPLORE_SIGNAL_REPLACE;
			} else {
				cout << "EXPLORE_SIGNAL_CLEAN" << endl;
				return EXPLORE_SIGNAL_CLEAN;
			}
		}
	} else {
		if (this->state_iter%10000 == 0) {
			cout << "this->sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

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
	switch (this->state) {
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
		// if (this->state_iter == 300000) {
		if (this->state_iter == 30) {
			starting_compress_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_INNER_SCOPE_INPUT) {
		// if (this->state_iter == 50000) {
		if (this->state_iter == 5) {
			this->new_state_factor = 5;
		// } else if (this->state_iter == 100000) {
		} else if (this->state_iter == 10) {
			this->new_state_factor = 1;
		}

		// if (this->state_iter == 300000) {
		if (this->state_iter == 30) {
			inner_scope_input_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE) {
		// if (this->state_iter == 300000) {
		if (this->state_iter == 30) {
			score_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_STATE) {
		// if (this->state_iter == 50000) {
		if (this->state_iter == 5) {
			this->new_state_factor = 5;
		// } else if (this->state_iter == 100000) {
		} else if (this->state_iter == 10) {
			this->new_state_factor = 1;
		}

		// if (this->state_iter == 300000) {
		if (this->state_iter == 30) {
			compress_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_SCOPE) {
		// if (this->state_iter == 50000) {
		if (this->state_iter == 5) {
			this->new_state_factor = 5;
		// } else if (this->state_iter == 100000) {
		} else if (this->state_iter == 10) {
			this->new_state_factor = 1;
		}

		// if (this->state_iter == 300000) {
		if (this->state_iter == 30) {
			compress_scope_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == STATE_INPUT
		// if (this->state_iter == 50000) {
		if (this->state_iter == 5) {
			this->new_state_factor = 5;
		// } else if (this->state_iter == 100000) {
		} else if (this->state_iter == 10) {
			this->new_state_factor = 1;
		}

		// if (this->state_iter == 300000) {
		if (this->state_iter == 30) {
			input_end();
		} else {
			if (this->state_iter%10000 == 0) {
				this->sum_error = 0.0;
			}
		}
	}
}

void Fold::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->sequence_length << endl;

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		output_file << this->is_existing[f_index] << endl;

		if (this->is_existing[f_index]) {
			output_file << this->existing_actions[f_index]->id << endl;

			ofstream scope_save_file;
			scope_save_file.open("saves/scope_" + to_string(this->existing_actions[f_index]->id) + ".txt");
			this->existing_actions[f_index]->save(scope_save_file);
			scope_save_file.close();
		}

		output_file << this->obs_sizes[f_index] << endl;
	}

	output_file << this->output_size << endl;

	output_file << this->finished_steps.size() << endl;
	for (int f_index = 0; f_index < (int)this->finished_steps.size(); f_index++) {
		output_file << this->finished_steps[f_index]->id << endl;

		ofstream finished_step_save_file;
		finished_step_save_file.open("saves/finished_step_" + to_string(this->finished_steps[f_index]->id) + ".txt");
		this->finished_steps[f_index]->save(finished_step_save_file);
		finished_step_save_file.close();
	}

	// no need to save this->state, this->last_state, this->state_iter, this->sum_error

	// this->average_misguess has already been passed on

	// this->starting_score_network, this->combined_score_network has already been passed on

	for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			ofstream scope_scale_mod_calc_save_file;
			scope_scale_mod_calc_save_file.open("saves/nns/fold_" + to_string(this->id) + "_scope_scale_mod_" + to_string(f_index) + ".txt");
			this->scope_scale_mod_calcs[f_index]->save(scope_scale_mod_calc_save_file);
			scope_scale_mod_calc_save_file.close();
		}
	}

	// this->end_scale_mod has already been passed on

	output_file << this->curr_s_input_sizes.size() << endl;
	for (int l_index = 0; l_index < (int)this->curr_s_input_sizes.size(); l_index++) {
		output_file << this->curr_s_input_sizes[l_index] << endl;
		output_file << this->curr_scope_sizes[l_index] << endl;
	}

	ofstream curr_fold_save_file;
	curr_fold_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_fold.txt");
	this->curr_fold->save(curr_fold_save_file);
	curr_fold_save_file.close();

	for (int f_index = (int)this->finished_steps.size(); f_index < this->sequence_length; f_index++) {
		if (this->is_existing[f_index]) {
			ofstream curr_input_fold_save_file;
			curr_input_fold_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_input_fold_" + to_string(f_index) + ".txt");
			this->curr_input_folds[f_index]->save(curr_input_fold_save_file);
			curr_input_fold_save_file.close();
		}
	}

	ofstream curr_end_fold_save_file;
	curr_end_fold_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_end_fold.txt");
	this->curr_end_fold->save(curr_end_fold_save_file);
	curr_end_fold_save_file.close();

	output_file << this->starting_average_misguess << endl;
	output_file << this->starting_average_local_impact << endl;

	output_file << this->curr_starting_compress_new_size << endl;
	output_file << this->starting_compress_original_size << endl;
	if (this->curr_starting_compress_new_size != this->starting_compress_original_size) {
		ofstream curr_starting_compress_network_save_file;
		curr_starting_compress_network_save_file.open("saves/nns/fold_" + to_string(this->id) + "_curr_starting_compress.txt");
		this->curr_starting_compress_network->save(curr_starting_compress_network_save_file);
		curr_starting_compress_network_save_file.close();
	}
}

FoldHistory::FoldHistory(Fold* fold) {
	this->fold = fold;

	this->curr_starting_compress_network_history = NULL;

	this->curr_input_network_history = NULL;

	this->curr_score_network_history = NULL;

	this->curr_compress_network_history = NULL;

	this->curr_input_fold_histories = vector<FoldNetworkHistory*>(fold->sequence_length, NULL);
	this->scope_histories = vector<ScopeHistory*>(fold->sequence_length, NULL);

	this->curr_fold_history = NULL;
	this->curr_end_fold_history = NULL;
}

FoldHistory::~FoldHistory() {
	if (this->curr_starting_compress_network_history != NULL) {
		delete this->curr_starting_compress_network_history;
	}

	for (int n_index = 0; n_index < (int)this->finished_step_histories.size(); n_index++) {
		delete this->finished_step_histories[n_index];
	}

	for (int i_index = 0; i_index < (int)this->inner_input_input_network_histories.size(); i_index++) {
		delete this->inner_input_input_network_histories[i_index];
	}

	if (this->curr_input_network_history != NULL) {
		delete this->curr_input_network_history;
	}

	if (this->curr_score_network_history != NULL) {
		delete this->curr_score_network_history;
	}

	if (this->curr_compress_network_history != NULL) {
		delete this->curr_compress_network_history;
	}

	for (int i_index = 0; i_index < (int)this->input_network_histories.size(); i_index++) {
		delete this->input_network_histories[i_index];
	}

	for (int f_index = 0; f_index < (int)this->curr_input_fold_histories.size(); f_index++) {
		if (this->curr_input_fold_histories[f_index] != NULL) {
			delete this->curr_input_fold_histories[f_index];
		}
	}

	for (int f_index = 0; f_index < (int)this->scope_histories.size(); f_index++) {
		if (this->scope_histories[f_index] != NULL) {
			delete this->scope_histories[f_index];
		}
	}

	if (this->curr_fold_history != NULL) {
		delete this->curr_fold_history;
	}

	if (this->curr_end_fold_history != NULL) {
		delete this->curr_end_fold_history;
	}
}
