#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"

using namespace std;

Scope::Scope(int num_inputs,
			 int num_outputs,
			 int sequence_length,
			 vector<bool> is_inner_scope,
			 vector<Scope*> scopes,
			 vector<int> obs_sizes,
			 vector<vector<FoldNetwork*>> inner_input_networks,
			 vector<vector<int>> inner_input_sizes,
			 vector<double> scope_scale_mod,
			 vector<int> step_types,
			 vector<Branch*> branches,
			 vector<Fold*> folds,
			 vector<FoldNetwork*> score_networks,
			 vector<double> average_misguesses,
			 vector<double> average_inner_scope_impacts,
			 vector<double> average_local_impacts,
			 vector<double> average_inner_branch_impacts,
			 vector<bool> active_compress,
			 vector<int> compress_new_sizes,
			 vector<FoldNetwork*> compress_networks,
			 vector<int> compress_original_sizes) {
	id_counter_mtx.lock();
	this->id = id_counter;
	id_counter++;
	id_counter_mtx.unlock();

	this->num_inputs = num_inputs;
	this->num_outputs = num_outputs;

	this->sequence_length = sequence_length;
	this->is_inner_scope = is_inner_scope;
	this->scopes = scopes;
	this->obs_sizes = obs_sizes;

	this->inner_input_networks = inner_input_networks;
	this->inner_input_sizes = inner_input_sizes;
	this->scope_scale_mod = scope_scale_mod;

	this->step_types = step_types;
	this->branches = branches;
	this->folds = folds;

	this->score_networks = score_networks;

	this->average_misguesses = average_misguesses;
	this->average_inner_scope_impacts = average_inner_scope_impacts;
	this->average_local_impacts = average_local_impacts;
	this->average_inner_branch_impacts = average_inner_branch_impacts;

	this->active_compress = active_compress;
	this->compress_new_sizes = compress_new_sizes;
	this->compress_networks = compress_networks;
	this->compress_original_sizes = compress_original_sizes;

	this->explore_index_inclusive = -1;
	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

Scope::Scope(Scope* original) {
	id_counter_mtx.lock();
	this->id = id_counter;
	id_counter++;
	id_counter_mtx.unlock();

	this->num_inputs = original->num_inputs;
	this->num_outputs = original->num_outputs;

	this->sequence_length = original->sequence_length;
	this->is_inner_scope = original->is_inner_scope;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (original->is_inner_scope[a_index]) {
			this->scopes.push_back(new Scope(original->scopes[a_index]));
		} else {
			this->scopes.push_back(NULL);
		}
	}
	this->obs_sizes = original->obs_sizes;

	// TODO: no need to deep copy as won't change
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		this->inner_input_networks.push_back(vector<FoldNetwork*>());
		for (int i_index = 0; i_index < (int)original->inner_input_networks[a_index].size(); i_index++) {
			this->inner_input_networks.back().push_back(original->inner_input_networks[a_index][i_index]);
		}
	}
	this->inner_input_sizes = original->inner_input_sizes;
	this->scope_scale_mod = original->scope_scale_mod;

	this->step_types = original->step_types;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (original->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->branches.push_back(new Branch(original->branches[a_index]));
		} else {
			this->branches.push_back(NULL);
		}
	}
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (original->step_types[a_index] == STEP_TYPE_FOLD) {
			this->folds.push_back(new Fold(original->folds[a_index]));
		} else {
			this->folds.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (original->step_types[a_index] == STEP_TYPE_STEP) {
			this->score_networks.push_back(new FoldNetwork(original->score_networks[a_index]));
		} else {
			this->score_networks.push_back(NULL);
		}
	}

	this->average_misguesses = original->average_misguesses;
	this->average_inner_scope_impacts = original->average_inner_scope_impacts;
	this->average_local_impacts = original->average_local_impacts;
	this->average_inner_branch_impacts = original->average_inner_branch_impacts;

	this->active_compress = original->active_compress;
	this->compress_new_sizes = original->compress_new_sizes;
	// TODO: no need to deep copy as won't change
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (original->step_types[a_index] == STEP_TYPE_STEP && original->active_compress[a_index]) {
			this->compress_networks.push_back(new FoldNetwork(original->compress_networks[a_index]));
		} else {
			this->compress_networks.push_back(NULL);
		}
	}
	this->compress_original_sizes = compress_original_sizes;

	// initialize to empty
	this->explore_index_inclusive = -1;
	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

Scope::Scope(std::ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	this->num_inputs = stoi(num_inputs_line);

	string num_outputs_line;
	getline(input_file, num_outputs_line);
	this->num_outputs = stoi(num_outputs_line);

	string sequence_length_line;
	getline(input_file, sequence_length_line);
	this->sequence_length = stoi(sequence_length_line);

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		string is_inner_scope_line;
		getline(input_file, is_inner_scope_line);
		this->is_inner_scope.push_back(stoi(is_inner_scope_line));

		if (this->is_inner_scope[a_index]) {
			string scope_id_line;
			getline(input_file, scope_id_line);
			int scope_id = stoi(scope_id_line);

			ifstream scope_save_file;
			scope_save_file.open("saves/scope_" + to_string(scope_id) + ".txt");
			this->scopes.push_back(new Scope(scope_save_file));
			scope_save_file.close();

			this->obs_sizes.push_back(-1);

			string inner_input_networks_size_line;
			getline(input_file, inner_input_networks_size_line);
			int inner_input_networks_size = stoi(inner_input_networks_size_line);

			this->inner_input_networks.push_back(vector<FoldNetwork*>());
			this->inner_input_sizes.push_back(vector<int>());
			for (int i_index = 0; i_index < inner_input_networks_size; i_index++) {
				ifstream inner_input_network_save_file;
				inner_input_network_save_file.open("saves/nns/scope_" + this->id + "_inner_input_" + to_string(a_index) + " " + to_string(i_index) + ".txt");
				this->inner_input_networks[a_index].push_back(new FoldNetwork(inner_input_network_save_file));
				inner_input_network_save_file.close();

				string inner_input_size_line;
				getline(input_file, inner_input_size_line);
				this->inner_input_sizes[a_index].push_back(stoi(inner_input_size_line));

				string scope_scale_mod_line;
				getline(input_file, scope_scale_mod_line);
				this->scope_scale_mod.push_back(stof(scope_scale_mod_line));
			}
		} else {
			this->scopes.push_back(NULL);

			string obs_size_line;
			getline(input_file, obs_size_line);
			this->obs_sizes.push_back(stoi(obs_size_line));

			this->inner_input_networks.push_back(vector<FoldNetwork*>());
			this->inner_input_sizes.push_back(vector<int>());
			this->scope_scale_mod.push_back(0.0);	// doesn't matter
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		string step_type_line;
		getline(input_file, step_type_line);
		this->step_types.push_back(stoi(step_type_line));

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index != this->sequence_length-1) {
				ifstream score_network_save_file;
				score_network_save_file.open("saves/nns/" + this->id + "_score_" + to_string(a_index) + ".txt");
				this->score_networks.push_back(new FoldNetwork(score_network_save_file));
				score_network_save_file.close();

				string active_compress_line;
				getline(input_file, active_compress_line);
				this->active_compress.push_back(stoi(active_compress_line));

				if (this->active_compress[a_index]) {
					string compress_new_size_line;
					getline(input_file, compress_new_size_line);
					this->compress_new_sizes.push_back(stoi(compress_new_size_line))

					ifstream compress_network_save_file;
					compress_network_save_file.open("saves/nns/" + this->id + "_compress_" + to_string(a_index) + ".txt");
					this->compress_networks.push_back(new FoldNetwork(compress_network_save_file));
					compress_network_save_file.close();

					string compress_original_size_line;
					getline(input_file, compress_original_size_line);
					this->compress_original_sizes.push_back(stoi(compress_original_size_line));
				} else {
					this->compress_new_sizes.push_back(-1);
					this->compress_networks.push_back(NULL);
					this->compress_original_sizes.push_back(-1);
				}
			} else {
				this->score_networks.push_back(NULL);

				this->active_compress.push_back(false);	// doesn't matter
				this->compress_new_sizes.push_back(-1);
				this->compress_networks.push_back(NULL);
				this->compress_original_sizes.push_back(-1);
			}

			this->branches.push_back(NULL);
			this->folds.push_back(NULL);
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->score_networks.push_back(NULL);
			
			string branch_id_line;
			getline(input_file, branch_id_line);
			int branch_id = stoi(branch_id_line);

			ifstream branch_save_file;
			branch_save_file.open("saves/branch_" + to_string(branch_id) + ".txt");
			this->branches.push_back(new Branch(branch_save_file));
			branch_save_file.close();

			this->active_compress.push_back(false);	// doesn't matter
			this->compress_new_sizes.push_back(-1);
			this->compress_networks.push_back(NULL);
			this->compress_original_sizes.push_back(-1);

			this->folds.push_back(NULL);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			ifstream score_network_save_file;
			score_network_save_file.open("saves/nns/" + this->id + "_score_" + to_string(a_index) + ".txt");
			this->score_networks.push_back(new FoldNetwork(score_network_save_file));
			score_network_save_file.close();

			string fold_id_line;
			getline(input_file, fold_id_line);
			int fold_id = stoi(fold_id_line);

			ifstream fold_save_file;
			fold_save_file.open("saves/fold_" + to_string(fold_id) + ".txt");
			this->folds.push_back(fold_save_file);
			fold_save_file.close();

			this->active_compress.push_back(false);	// doesn't matter
			this->compress_new_sizes.push_back(-1);
			this->compress_networks.push_back(NULL);
			this->compress_original_sizes.push_back(-1);

			this->branches.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		string average_misguess_line;
		getline(input_file, average_misguess_line);
		this->average_misguesses.push_back(stof(average_misguess_line));

		string average_inner_scope_impact_line;
		getline(input_file, average_inner_scope_impact_line);
		this->average_inner_scope_impacts.push_back(stof(average_inner_scope_impact_line));

		string average_local_impact_line;
		getline(input_file, average_local_impact_line);
		this->average_local_impacts.push_back(stof(average_local_impact_line));

		string average_inner_branch_impact_line;
		getline(input_file, average_inner_branch_impact_line);
		this->average_inner_branch_impacts.push_back(stof(average_inner_branch_impact_line));
	}

	this->explore_index_inclusive = -1;
	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

Scope::~Scope() {
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->scopes[a_index] != NULL) {
			delete this->scopes[a_index];
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
			delete this->inner_input_networks[a_index][i_index];
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->branches[a_index] != NULL) {
			delete this->branches[a_index];
		}
	}
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->folds[a_index] != NULL) {
			delete this->folds[a_index];
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->score_networks[a_index] != NULL) {
			delete this->score_networks[a_index];
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->compress_networks[a_index] != NULL) {
			delete this->compress_networks[a_index];
		}
	}

	if (this->explore_fold != NULL) {
		delete this->explore_fold;
	}
}

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
		// Note: no NO-OPs besides absolute start, and instead, if want to branch start, explore from outside
	} else {
		// for start, if inner scope, scope_input is first local_s_input_vals
		vector<double> scope_input(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);

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
		// can't be scope end

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
			} else {
				// can compress down to 0
				local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[0], local_state_vals.end());
				// if no compress, this->compress_new_sizes[0] == this->compress_original_sizes[0], and does nothing
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
	while (a_index < this->sequence_length) {
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
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
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
						// can compress down to 0
						local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[a_index], local_state_vals.end());
					}
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
		// can't be scope end

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
		} else {
			local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[0], local_state_vals.end());
		}
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
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
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
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
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
	if (this->explore_end_non_inclusive == this->sequence_length
			&& this->explore_type == EXPLORE_TYPE_NEW) {
		a_index = this->explore_index_inclusive;
	} else {
		a_index = this->sequence_length-1;
	}

	// mid
	while (a_index >= 1) {
		if (this->explore_index_inclusive == a_index
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			int explore_signal = this->explore_fold->explore_on_path_backprop(
				local_state_errors,
				predicted_score,
				target_val,
				scale_factor,
				scale_factor_error,
				history->explore_fold_history);

			if (explore_signal == EXPLORE_SIGNAL_REPLACE) {
				explore_replace();
			} else if (explore_signal == EXPLORE_SIGNAL_BRANCH) {
				explore_branch();
			} else if (explore_signal == EXPLORE_SIGNAL_CLEAN) {
				this->explore_index_inclusive = -1;
				this->explore_type = EXPLORE_TYPE_NONE;
				this->explore_end_non_inclusive = -1;
				delete this->explore_fold;
				this->explore_fold = NULL;
			}

			return;
		} else {
			if (this->step_types[a_index] == STEP_TYPE_STEP) {
				if (a_index == this->sequence_length-1) {
					// scope end -- do nothing
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
				}
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
				// on_path doesn't need scope_output_errors

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
		int explore_signal = this->explore_fold->explore_on_path_backprop(
			local_state_errors,
			predicted_score,
			target_val,
			scale_factor,
			scale_factor_error,
			history->explore_fold_history);

		if (explore_signal == EXPLORE_SIGNAL_REPLACE) {
			explore_replace();
		} else if (explore_signal == EXPLORE_SIGNAL_BRANCH) {
			explore_branch();
		} else if (explore_signal == EXPLORE_SIGNAL_CLEAN) {
			this->explore_index_inclusive = -1;
			this->explore_type = EXPLORE_TYPE_NONE;
			this->explore_end_non_inclusive = -1;
			delete this->explore_fold;
			this->explore_fold = NULL;
		}

		return;
	} else {
		if (this->step_types[0] == STEP_TYPE_STEP) {
			// can't be scope end

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

	// mid
	for (int a_index = this->sequence_length-1; a_index >= 1; a_index--) {
		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
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
			}
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
		// can't be scope end

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
		// can't be scope end

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
		} else {
			local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[0], local_state_vals.end());
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
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
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
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
			} else {
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
}

void Scope::existing_flat_backprop(vector<double>& local_state_errors,		// input_errors
								   vector<double>& local_s_input_errors,	// output_errors
								   double& predicted_score,
								   double predicted_score_error,
								   double& scale_factor,
								   double& scale_factor_error,
								   ScopeHistory* history) {
	local_s_input_errors = vector<double>(this->num_inputs, 0.0);

	// mid
	for (int a_index = this->sequence_length-1; a_index >= 1; a_index--) {
		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
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
			}
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
		// can't be scope end

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
		} else {
			// this->compress_original_sizes[0] > this->compress_new_sizes[0]
			int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
			for (int c_index = 0; c_index < compress_size; c_index++) {
				local_state_errors.push_back(0.0);
			}
		}

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
		// can't be scope end

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
		} else {
			local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[0], local_state_vals.end());
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
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
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
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
			} else {
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
}

void Scope::update_backprop(double& predicted_score,
							double& next_predicted_score,	// for ending misguess from outside
							double target_val,
							double& scale_factor,
							ScopeHistory* history) {
	// mid
	for (int a_index = this->sequence_length-1; a_index >= 1; a_index--) {
		if (a_index == this->sequence_length-1) {
			double misguess = abs(target_val - next_predicted_score);
			this->average_misguesses[a_index] = 0.999*this->average_misguesses[a_index] + 0.001*misguess;
		} else {
			double misguess = abs(target_val - predicted_score);
			this->average_misguesses[a_index] = 0.999*this->average_misguesses[a_index] + 0.001*misguess;
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
			} else {
				double predicted_score_error = target_val - predicted_score;

				vector<double> score_errors{scale_factor*predicted_score_error};
				this->score_networks[a_index]->backprop_small_weights_with_no_error_signal(
					score_errors,
					0.001,
					history->score_network_histories[a_index]);

				next_predicted_score = predicted_score;
				predicted_score -= scale_factor*history->score_updates[a_index];

				this->average_local_impacts[a_index] = 0.999*this->average_local_impacts[a_index]
					+ 0.001*abs(scale_factor*history->score_updates[a_index]);
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			double ending_predicted_score = predicted_score;

			this->branches[a_index]->update_backprop(predicted_score,
													 next_predicted_score,
													 target_val,
													 scale_factor,
													 history->branch_histories[a_index]);

			double starting_predicted_score = predicted_score;
			this->average_inner_branch_impacts[a_index] = 0.999*this->average_inner_branch_impacts[a_index]
				+ 0.001*abs(ending_predicted_score - starting_predicted_score);
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

			// local_impact kept track of as starting impact in fold

			if (this->folds[a_index]->state == STATE_DONE) {
				resolve_fold(a_index);
			}
		}

		if (!this->is_inner_scope[a_index]) {
			// do nothing
		} else {
			double ending_predicted_score = predicted_score;

			scale_factor *= this->scope_scale_mod[a_index];

			this->scopes[a_index]->update_backprop(predicted_score,
												   next_predicted_score,
												   target_val,
												   scale_factor,
												   history->scope_histories[a_index]);

			// mods are a helper for initial flat, so no need to modify after

			scale_factor /= this->scope_scale_mod[a_index];

			double starting_predicted_score = predicted_score;
			this->average_inner_scope_impacts[a_index] = 0.999*this->average_inner_scope_impacts[a_index]
				+ 0.001*abs(ending_predicted_score - starting_predicted_score);
		}
	}

	// start
	double misguess = abs(target_val - predicted_score);
	this->average_misguesses[0] = 0.999*this->average_misguesses[0] + 0.001*misguess;

	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		double predicted_score_error = target_val - predicted_score;

		vector<double> score_errors{scale_factor*predicted_score_error};
		this->score_networks[0]->backprop_small_weights_with_no_error_signal(
			score_errors,
			0.001,
			history->score_network_histories[0]);

		next_predicted_score = predicted_score;
		predicted_score -= scale_factor*history->score_updates[0];

		this->average_local_impacts[0] = 0.999*this->average_local_impacts[0]
			+ 0.001*abs(scale_factor*history->score_updates[0]);
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		double ending_predicted_score = predicted_score;

		this->branches[0]->update_backprop(predicted_score,
										   next_predicted_score,
										   target_val,
										   scale_factor,
										   history->branch_histories[0]);

		double starting_predicted_score = predicted_score;
		this->average_inner_branch_impacts[0] = 0.999*this->average_inner_branch_impacts[0]
			+ 0.001*abs(ending_predicted_score - starting_predicted_score);
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

		// local_impact kept track of as starting impact in fold

		if (this->folds[0]->state == STATE_DONE) {
			resolve_fold(0);
		}
	}

	if (!this->is_inner_scope[0]) {
		// do nothing
	} else {
		double ending_predicted_score = predicted_score;

		scale_factor *= this->scope_scale_mod[0];

		this->scopes[0]->update_backprop(predicted_score,
										 next_predicted_score,
										 target_val,
										 scale_factor,
										 history->scope_histories[0]);

		// mods are a helper for initial flat, so no need to modify after

		scale_factor /= this->scope_scale_mod[0];

		double starting_predicted_score = predicted_score;
		this->average_inner_scope_impacts[0] = 0.999*this->average_inner_scope_impacts[0]
			+ 0.001*abs(ending_predicted_score - starting_predicted_score);
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
		// can't be scope end

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
		} else {
			local_state_vals.erase(local_state_vals.begin()+this->compress_new_sizes[0], local_state_vals.end());
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
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
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
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
			} else {
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
}

void Scope::existing_update_backprop(double& predicted_score,
									 double predicted_score_error,
									 double& scale_factor,
									 double& scale_factor_error,
									 ScopeHistory* history) {
	// mid
	for (int a_index = this->sequence_length-1; a_index >= 1; a_index--) {
		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1) {
				// scope end -- do nothing
			} else {
				scale_factor_error += this->score_updates[a_index]*predicted_score_error;

				predicted_score -= scale_factor*this->score_updates[a_index];
			}
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
		// can't be scope end

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

void Scope::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->num_inputs << endl;
	output_file << this->num_outputs << endl;

	output_file << this->sequence_length << endl;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->is_inner_scope[a_index] << endl;
		if (this->is_inner_scope[a_index]) {
			output_file << this->scopes[a_index]->id << endl;

			ofstream scope_save_file;
			scope_save_file.open("saves/scope_" + this->scopes[a_index]->id + ".txt");
			this->scopes[a_index]->save(scope_save_file);
			scope_save_file.close();

			output_file << this->inner_input_networks[a_index].size() << endl;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				ofstream inner_input_network_save_file;
				inner_input_network_save_file.open("saves/nns/scope_" + this->id + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
				this->inner_input_networks[a_index][i_index]->save(inner_input_network_save_file);
				inner_input_network_save_file.close();

				output_file << this->inner_input_sizes[a_index][i_index] << endl;
			}
			output_file << scope_scale_mod[a_index] << endl;
		} else {
			output_file << this->obs_sizes[a_index] << endl;
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->step_types[a_index] << endl;

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index != this->sequence_length-1) {
				ofstream score_network_save_file;
				score_network_save_file.open("saves/nns/" + this->id + "_score_" + to_string(a_index) + ".txt");
				this->score_networks[a_index]->save(score_network_save_file);
				score_network_save_file.close();

				output_file << this->active_compress[a_index] << endl;
				if (this->active_compress[a_index]) {
					output_file << this->compress_new_sizes[a_index] << endl;

					ofstream compress_network_save_file;
					compress_network_save_file.open("saves/nns/" + this->id + "_compress_" + to_string(a_index) + ".txt");
					this->compress_networks[a_index]->save(compress_network_save_file);
					compress_network_save_file.close();

					output_file << this->compress_original_sizes[a_index] << endl;
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			output_file << this->branches[a_index]->id << endl;

			ofstream branch_save_file;
			branch_save_file.open("saves/branch_" + this->branches[a_index]->id + ".txt");
			this->branches[a_index]->save(branch_save_file);
			branch_save_file.close();
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			ofstream score_network_save_file;
			score_network_save_file.open("saves/nns/" + this->id + "_score_" + to_string(a_index) + ".txt");
			this->score_networks[a_index]->save(score_network_save_file);
			score_network_save_file.close();

			output_file << this->folds[a_index]->id << endl;

			ofstream fold_save_file;
			fold_save_file.open("saves/fold_" + this->folds[a_index]->id + ".txt");
			this->folds[a_index]->save(fold_save_file);
			fold_save_file.close();
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->average_misguesses[a_index] << endl;
		output_file << this->average_inner_scope_impacts[a_index] << endl;
		output_file << this->average_local_impacts[a_index] << endl;
		output_file << this->average_inner_branch_impacts[a_index] << endl;
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
