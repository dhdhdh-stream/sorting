#include "branch.h"

#include <iostream>
#include <limits>

#include "globals.h"
#include "utilities.h"

using namespace std;

Branch::Branch(int num_inputs,
			   int num_outputs,
			   int outer_s_input_size,
			   FoldNetwork* combined_score_network,
			   bool passed_combined,
			   vector<FoldNetwork*> score_networks,
			   vector<bool> is_branch,
			   vector<BranchPath*> branches,
			   vector<Fold*> folds,
			   vector<int> num_travelled) {
	solution->id_counter_mtx.lock();
	this->id = solution->id_counter;
	solution->id_counter++;
	solution->id_counter_mtx.unlock();

	this->num_inputs = num_inputs;
	this->num_outputs = num_outputs;
	this->outer_s_input_size = outer_s_input_size;

	this->combined_score_network = combined_score_network;
	this->passed_combined = passed_combined;

	this->score_networks = score_networks;
	this->is_branch = is_branch;
	this->branches = branches;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		if (this->is_branch[b_index]) {
			this->branches[b_index]->parent = this;
			this->branches[b_index]->parent_index = b_index;
		}
	}
	this->folds = folds;
	this->num_travelled = num_travelled;

	this->explore_ref_count = 0;
}

Branch::Branch(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	this->num_inputs = stoi(num_inputs_line);

	string num_outputs_line;
	getline(input_file, num_outputs_line);
	this->num_outputs = stoi(num_outputs_line);

	string outer_s_input_size_line;
	getline(input_file, outer_s_input_size_line);
	this->outer_s_input_size = stoi(outer_s_input_size_line);

	string passed_combined_line;
	getline(input_file, passed_combined_line);
	this->passed_combined = stoi(passed_combined_line);

	if (!this->passed_combined) {
		ifstream combined_score_network_save_file;
		combined_score_network_save_file.open("saves/branch_" + to_string(this->id) + "_combined_score.txt");
		this->combined_score_network = new FoldNetwork(combined_score_network_save_file);
		combined_score_network_save_file.close();
	} else {
		this->combined_score_network = NULL;
	}

	string branches_size_line;
	getline(input_file, branches_size_line);
	int branches_size = stoi(branches_size_line);

	for (int b_index = 0; b_index < branches_size; b_index++) {
		ifstream score_network_save_file;
		score_network_save_file.open("saves/branch_" + to_string(this->id) + "_score_" + to_string(b_index) + ".txt");
		this->score_networks.push_back(new FoldNetwork(score_network_save_file));
		score_network_save_file.close();

		string is_branch_line;
		getline(input_file, is_branch_line);
		this->is_branch.push_back(stoi(is_branch_line));

		if (this->is_branch[b_index]) {
			string branch_id_line;
			getline(input_file, branch_id_line);
			int branch_id = stoi(branch_id_line);

			ifstream branch_save_file;
			branch_save_file.open("saves/branch_path_" + to_string(branch_id) + ".txt");
			this->branches.push_back(new BranchPath(branch_save_file));
			branch_save_file.close();
			this->branches[b_index]->parent = this;
			this->branches[b_index]->parent_index = b_index;

			this->folds.push_back(NULL);
		} else {
			string fold_id_line;
			getline(input_file, fold_id_line);
			int fold_id = stoi(fold_id_line);

			ifstream fold_save_file;
			fold_save_file.open("saves/fold_" + to_string(fold_id) + ".txt");
			this->folds.push_back(new Fold(fold_save_file));
			fold_save_file.close();

			this->branches.push_back(NULL);
		}

		string num_travelled_line;
		getline(input_file, num_travelled_line);
		this->num_travelled.push_back(stoi(num_travelled_line));
	}

	// explores cleared on reload
	this->explore_ref_count = 0;
}

Branch::~Branch() {
	if (this->combined_score_network != NULL) {
		delete this->combined_score_network;
	}

	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		delete this->score_networks[b_index];

		if (this->branches[b_index] != NULL) {
			delete this->branches[b_index];
		}

		if (this->folds[b_index] != NULL) {
			delete this->folds[b_index];
		}
	}
}

void Branch::explore_on_path_activate_score(vector<double>& local_s_input_vals,
											vector<double>& local_state_vals,
											double& scale_factor,
											RunStatus& run_status,
											BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	int best_index = -1;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		if (this->is_branch[b_index]) {
			this->score_networks[b_index]->activate_small(local_s_input_vals,
														  local_state_vals);
			double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];

			bool branch_avail = true;
			if (this->num_travelled[b_index] < 100000) {
				if (randuni() < (double)this->num_travelled[b_index]/100000) {
					branch_avail = false;
				}
			}

			if (curr_score > best_score && branch_avail) {
				best_score = curr_score;
				best_index = b_index;
			}
		}
	}
	history->best_score = best_score;
	history->best_index = best_index;
}

void Branch::explore_off_path_activate_score(vector<double>& local_s_input_vals,
											 vector<double>& local_state_vals,
											 double& scale_factor,
											 RunStatus& run_status,
											 BranchHistory* history) {
	double best_score = numeric_limits<double>::lowest();
	int best_index = -1;
	if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
		if (!this->passed_combined) {
			FoldNetworkHistory* combined_score_network_history = new FoldNetworkHistory(this->combined_score_network);
			this->combined_score_network->activate_small(local_s_input_vals,
														 local_state_vals,
														 combined_score_network_history);
			history->combined_score_network_history = combined_score_network_history;
			history->combined_score_network_output = this->combined_score_network->output->acti_vals[0];
		}

		FoldNetworkHistory* best_history = NULL;
		for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
			FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
			this->score_networks[b_index]->activate_small(local_s_input_vals,
														  local_state_vals,
														  curr_history);
			double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];

			bool branch_avail = true;
			if (this->num_travelled[b_index] < 100000) {
				if (randuni() < (double)this->num_travelled[b_index]/100000) {
					branch_avail = false;
				}
			}

			if (curr_score > best_score && branch_avail) {
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

			bool branch_avail = true;
			if (this->num_travelled[b_index] < 100000) {
				if (randuni() < (double)this->num_travelled[b_index]/100000) {
					branch_avail = false;
				}
			}

			if (curr_score > best_score && branch_avail) {
				best_score = curr_score;
				best_index = b_index;
			}
		}
	}
	history->best_score = best_score;
	history->best_index = best_index;
}

void Branch::explore_on_path_activate(Problem& problem,
									  vector<double>& local_s_input_vals,
									  vector<double>& local_state_vals,
									  double& predicted_score,
									  double& scale_factor,
									  RunStatus& run_status,
									  BranchHistory* history) {
	BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[history->best_index]);
	this->branches[history->best_index]->explore_on_path_activate(problem,
																  history->best_score,
																  history->best_predicted_misguess,
																  local_s_input_vals,
																  local_state_vals,
																  predicted_score,
																  scale_factor,
																  run_status,
																  branch_path_history);
	history->branch_path_history = branch_path_history;
}

void Branch::explore_off_path_activate(Problem& problem,
									   vector<double>& local_s_input_vals,
									   vector<double>& local_state_vals,
									   double& predicted_score,
									   double& scale_factor,
									   RunStatus& run_status,
									   BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[history->best_index]);
		this->branches[history->best_index]->explore_off_path_activate(problem,
																	   history->best_score,
																	   local_s_input_vals,
																	   local_state_vals,
																	   predicted_score,
																	   scale_factor,
																	   run_status,
																	   branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[history->best_index]);
		this->folds[history->best_index]->explore_off_path_activate(problem,
																	history->best_score,
																	local_s_input_vals,
																	local_state_vals,
																	predicted_score,
																	scale_factor,
																	run_status,
																	fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::explore_on_path_backprop(vector<double>& local_s_input_errors,
									  vector<double>& local_state_errors,
									  double& predicted_score,
									  double target_val,
									  double final_misguess,
									  double& scale_factor,
									  double& scale_factor_error,
									  BranchHistory* history) {
	this->branches[history->best_index]->explore_on_path_backprop(local_s_input_errors,
																  local_state_errors,
																  predicted_score,
																  target_val,
																  final_misguess,
																  scale_factor,
																  scale_factor_error,
																  history->branch_path_history);

	if (this->branches[history->best_index]->explore_type == EXPLORE_TYPE_NONE) {
		this->explore_ref_count--;
	}
}

void Branch::explore_off_path_backprop(vector<double>& local_s_input_errors,
									   vector<double>& local_state_errors,
									   double& predicted_score,
									   double target_val,
									   double& scale_factor,
									   double& scale_factor_error,
									   BranchHistory* history) {
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

	vector<double> score_errors{scale_factor*score_predicted_score_error};
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

	if (!this->passed_combined) {
		double combined_score_predicted_score = predicted_score + scale_factor*history->combined_score_network_output;
		double combined_score_predicted_score_error = target_val - combined_score_predicted_score;

		vector<double> combined_score_errors{scale_factor*combined_score_predicted_score_error};
		vector<double> combined_score_s_input_output_errors;
		vector<double> combined_score_state_output_errors;
		this->combined_score_network->backprop_small_errors_with_no_weight_change(
			combined_score_errors,
			combined_score_s_input_output_errors,
			combined_score_state_output_errors,
			history->combined_score_network_history);
		for (int s_index = 0; s_index < (int)combined_score_s_input_output_errors.size(); s_index++) {
			local_s_input_errors[s_index] += combined_score_s_input_output_errors[s_index];
		}
		for (int s_index = 0; s_index < (int)combined_score_state_output_errors.size(); s_index++) {
			local_state_errors[s_index] += combined_score_state_output_errors[s_index];
		}
	}
}

void Branch::existing_flat_activate(Problem& problem,
									vector<double>& local_s_input_vals,
									vector<double>& local_state_vals,
									double& predicted_score,
									double& scale_factor,
									RunStatus& run_status,
									BranchHistory* history) {
	// don't activate combined as will not backprop

	double best_score = numeric_limits<double>::lowest();
	int best_index = -1;
	FoldNetworkHistory* best_history = NULL;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		FoldNetworkHistory* curr_history = new FoldNetworkHistory(this->score_networks[b_index]);
		this->score_networks[b_index]->activate_small(local_s_input_vals,
													  local_state_vals,
													  curr_history);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];

		bool branch_avail = true;
		if (this->num_travelled[b_index] < 100000) {
			if (randuni() < (double)this->num_travelled[b_index]/100000) {
				branch_avail = false;
			}
		}

		if (curr_score > best_score && branch_avail) {
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
		this->branches[best_index]->existing_flat_activate(problem,
														   best_score,
														   local_s_input_vals,
														   local_state_vals,
														   predicted_score,
														   scale_factor,
														   run_status,
														   branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->existing_flat_activate(problem,
														best_score,
														local_s_input_vals,
														local_state_vals,
														predicted_score,
														scale_factor,
														run_status,
														fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::existing_flat_backprop(vector<double>& local_s_input_errors,
									vector<double>& local_state_errors,
									double& predicted_score,
									double predicted_score_error,
									double& scale_factor,
									double& scale_factor_error,
									BranchHistory* history) {
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

	vector<double> score_errors{scale_factor*predicted_score_error};
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

	// combined_score_network has no direct connection to predicted_score_error (and has no impact for existing_flat)
}

void Branch::update_activate(Problem& problem,
							 vector<double>& local_s_input_vals,
							 vector<double>& local_state_vals,
							 double& predicted_score,
							 double& scale_factor,
							 RunStatus& run_status,
							 BranchHistory* history) {
	if (!this->passed_combined) {
		FoldNetworkHistory* combined_score_network_history = new FoldNetworkHistory(this->combined_score_network);
		this->combined_score_network->activate_small(local_s_input_vals,
													 local_state_vals,
													 combined_score_network_history);
		history->combined_score_network_history = combined_score_network_history;
		history->combined_score_network_output = this->combined_score_network->output->acti_vals[0];
	}

	double best_score = numeric_limits<double>::lowest();
	int best_index = -1;
	FoldNetworkHistory* best_score_history = NULL;
	double best_full_score = numeric_limits<double>::lowest();
	int best_full_index = -1;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		FoldNetworkHistory* curr_score_history = new FoldNetworkHistory(this->score_networks[b_index]);
		this->score_networks[b_index]->activate_small(local_s_input_vals,
													  local_state_vals,
													  curr_score_history);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];

		bool branch_avail = true;
		if (this->num_travelled[b_index] < 100000) {
			if (randuni() < (double)this->num_travelled[b_index]/100000) {
				branch_avail = false;
			}
		}

		if (curr_score > best_score && branch_avail) {
			best_score = curr_score;
			best_index = b_index;
			if (best_score_history != NULL) {
				delete best_score_history;
			}
			best_score_history = curr_score_history;
		} else {
			delete curr_score_history;
		}

		if (curr_score > best_full_score) {
			best_full_score = curr_score;
			best_full_index = b_index;
		}
	}
	history->score_network_history = best_score_history;

	if (this->num_travelled[best_full_index] < 100000) {
		this->num_travelled[best_full_index]++;
	}

	history->best_score = best_score;
	history->best_index = best_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->update_activate(problem,
													best_score,
													local_s_input_vals,
													local_state_vals,
													predicted_score,
													scale_factor,
													run_status,
													branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->update_activate(problem,
												 best_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 run_status,
												 fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::update_backprop(double& predicted_score,
							 double target_val,
							 double final_misguess,
							 double& scale_factor,
							 double& scale_factor_error,
							 BranchHistory* history) {
	if (this->is_branch[history->best_index]) {
		this->branches[history->best_index]->update_backprop(predicted_score,
															 target_val,
															 final_misguess,
															 scale_factor,
															 scale_factor_error,
															 history->branch_path_history);
	} else {
		this->folds[history->best_index]->update_backprop(predicted_score,
														  target_val,
														  final_misguess,
														  scale_factor,
														  scale_factor_error,
														  history->fold_history);
	}

	// predicted_score already modified to before branch value in branch_path
	double score_predicted_score = predicted_score + history->best_score;
	double score_predicted_score_error = target_val - score_predicted_score;

	double score_update = history->best_score/scale_factor;
	scale_factor_error += score_update*score_predicted_score_error;

	vector<double> score_errors{scale_factor*score_predicted_score_error};
	this->score_networks[history->best_index]->backprop_small_weights_with_no_error_signal(
		score_errors,
		0.001,
		history->score_network_history);

	// score_networks don't update predicted_score

	if (!this->passed_combined) {
		double combined_score_predicted_score = predicted_score + scale_factor*history->combined_score_network_output;
		double combined_score_predicted_score_error = target_val - combined_score_predicted_score;

		scale_factor_error += history->combined_score_network_output*combined_score_predicted_score_error;

		vector<double> combined_score_errors{scale_factor*combined_score_predicted_score_error};
		this->combined_score_network->backprop_small_weights_with_no_error_signal(
			combined_score_errors,
			0.001,
			history->combined_score_network_history);
	}
}

void Branch::existing_update_activate(Problem& problem,
									  vector<double>& local_s_input_vals,	// i.e., input
									  vector<double>& local_state_vals,	// i.e., output
									  double& predicted_score,
									  double& scale_factor,
									  RunStatus& run_status,
									  BranchHistory* history) {
	// don't activate branch_score as will not backprop it

	double best_score = numeric_limits<double>::lowest();
	int best_index = -1;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		this->score_networks[b_index]->activate_small(local_s_input_vals,
													  local_state_vals);
		double curr_score = scale_factor*this->score_networks[b_index]->output->acti_vals[0];

		bool branch_avail = true;
		if (this->num_travelled[b_index] < 100000) {
			if (randuni() < (double)this->num_travelled[b_index]/100000) {
				branch_avail = false;
			}
		}

		if (curr_score > best_score && branch_avail) {
			best_score = curr_score;
			best_index = b_index;
		}
	}

	history->best_score = best_score;
	history->best_index = best_index;

	if (this->is_branch[best_index]) {
		BranchPathHistory* branch_path_history = new BranchPathHistory(this->branches[best_index]);
		this->branches[best_index]->existing_update_activate(problem,
															 best_score,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 run_status,
															 branch_path_history);
		history->branch_path_history = branch_path_history;
	} else {
		FoldHistory* fold_history = new FoldHistory(this->folds[best_index]);
		this->folds[best_index]->existing_update_activate(problem,
														  best_score,
														  local_s_input_vals,
														  local_state_vals,
														  predicted_score,
														  scale_factor,
														  run_status,
														  fold_history);
		history->fold_history = fold_history;
	}
}

void Branch::existing_update_backprop(double& predicted_score,
									  double predicted_score_error,
									  double& scale_factor,
									  double& scale_factor_error,
									  BranchHistory* history) {
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

void Branch::explore_set(double target_val,
						 double existing_score,
						 BranchHistory* history) {
	if (this->branches[history->best_index]->explore_type == EXPLORE_TYPE_NONE) {
		this->explore_ref_count++;
	}

	this->branches[history->best_index]->explore_set(target_val,
													 existing_score,
													 history->branch_path_history);
}

void Branch::update_increment(BranchHistory* history,
							  vector<Fold*>& folds_to_delete) {
	if (this->is_branch[history->best_index]) {
		if (history->branch_path_history != NULL) {	// might be new is_branch from resolved fold
			this->branches[history->best_index]->update_increment(history->branch_path_history,
																  folds_to_delete);
		}
	} else {
		if (history->fold_history->fold == this->folds[history->best_index]) {
			this->folds[history->best_index]->update_increment(history->fold_history,
															   folds_to_delete);
		}

		if (history->fold_history->fold == this->folds[history->best_index]) {
			if (this->folds[history->best_index]->state == STATE_DONE) {
				resolve_fold(history->best_index,
							 folds_to_delete);
			}
		}
	}
}

void Branch::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->num_inputs << endl;
	output_file << this->num_outputs << endl;
	output_file << this->outer_s_input_size << endl;

	output_file << this->passed_combined << endl;
	if (!this->passed_combined) {
		ofstream combined_score_network_save_file;
		combined_score_network_save_file.open("saves/branch_" + to_string(this->id) + "_combined_score.txt");
		this->combined_score_network->save(combined_score_network_save_file);
		combined_score_network_save_file.close();
	}

	output_file << this->branches.size() << endl;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		ofstream score_network_save_file;
		score_network_save_file.open("saves/branch_" + to_string(this->id) + "_score_" + to_string(b_index) + ".txt");
		this->score_networks[b_index]->save(score_network_save_file);
		score_network_save_file.close();

		output_file << this->is_branch[b_index] << endl;

		if (this->is_branch[b_index]) {
			output_file << this->branches[b_index]->id << endl;

			ofstream branch_save_file;
			branch_save_file.open("saves/branch_path_" + to_string(this->branches[b_index]->id) + ".txt");
			this->branches[b_index]->save(branch_save_file);
			branch_save_file.close();
		} else {
			output_file << this->folds[b_index]->id << endl;

			ofstream fold_save_file;
			fold_save_file.open("saves/fold_" + to_string(this->folds[b_index]->id) + ".txt");
			this->folds[b_index]->save(fold_save_file);
			fold_save_file.close();
		}

		output_file << this->num_travelled[b_index] << endl;
	}
}

void Branch::save_for_display(ofstream& output_file,
							  int curr_scope_id) {
	output_file << this->branches.size() << endl;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		output_file << this->is_branch[b_index] << endl;

		if (this->is_branch[b_index]) {
			this->branches[b_index]->save_for_display(output_file,
													  curr_scope_id);
		} else {
			this->folds[b_index]->save_for_display(output_file);
		}
	}
}

BranchHistory::BranchHistory(Branch* branch) {
	this->branch = branch;

	this->combined_score_network_history = NULL;

	this->score_network_history = NULL;
	this->branch_path_history = NULL;
	this->fold_history = NULL;
}

BranchHistory::~BranchHistory() {
	if (this->combined_score_network_history != NULL) {
		delete this->combined_score_network_history;
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}

	if (this->branch_path_history != NULL) {
		delete this->branch_path_history;
	}

	if (this->fold_history != NULL) {
		delete this->fold_history;
	}
}
