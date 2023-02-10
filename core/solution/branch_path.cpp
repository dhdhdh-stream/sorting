#include "branch_path.h"

#include <iostream>

#include "globals.h"
#include "utilities.h"

using namespace std;

BranchPath::BranchPath(int num_inputs,
					   int num_outputs,
					   int outer_s_input_size,
					   int sequence_length,
					   vector<bool> is_inner_scope,
					   vector<Scope*> scopes,
					   vector<Action> actions,
					   vector<vector<FoldNetwork*>> inner_input_networks,
					   vector<vector<int>> inner_input_sizes,
					   vector<Network*> scope_scale_mod,
					   vector<int> step_types,
					   vector<Branch*> branches,
					   vector<Fold*> folds,
					   vector<FoldNetwork*> score_networks,
					   vector<FoldNetwork*> confidence_networks,
					   vector<double> average_inner_scope_impacts,
					   vector<double> average_local_impacts,
					   vector<double> average_inner_branch_impacts,
					   double average_score,
					   double score_variance,
					   double average_misguess,
					   double misguess_variance,
					   vector<bool> active_compress,
					   vector<int> compress_new_sizes,
					   vector<FoldNetwork*> compress_networks,
					   vector<int> compress_original_sizes,
					   bool full_last) {
	solution->id_counter_mtx.lock();
	this->id = solution->id_counter;
	solution->id_counter++;
	solution->id_counter_mtx.unlock();

	this->num_inputs = num_inputs;
	this->num_outputs = num_outputs;
	this->outer_s_input_size = outer_s_input_size;

	this->sequence_length = sequence_length;
	this->is_inner_scope = is_inner_scope;
	this->scopes = scopes;
	this->actions = actions;

	this->inner_input_networks = inner_input_networks;
	this->inner_input_sizes = inner_input_sizes;
	this->scope_scale_mod = scope_scale_mod;

	this->step_types = step_types;
	this->branches = branches;
	this->folds = folds;

	this->score_networks = score_networks;
	this->confidence_networks = confidence_networks;

	this->average_inner_scope_impacts = average_inner_scope_impacts;
	this->average_local_impacts = average_local_impacts;
	this->average_inner_branch_impacts = average_inner_branch_impacts;

	this->average_score = average_score;
	this->score_variance = score_variance;
	this->average_misguess = average_misguess;
	this->misguess_variance = misguess_variance;

	this->active_compress = active_compress;
	this->compress_new_sizes = compress_new_sizes;
	this->compress_networks = compress_networks;
	this->compress_original_sizes = compress_original_sizes;

	this->full_last = full_last;

	// differs from scope
	int state_size = this->num_inputs;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		this->starting_state_sizes.push_back(state_size);

		if (a_index != 0) {
			if (!this->is_inner_scope[a_index]) {
				// obs_size always 1 for sorting
				state_size++;
			} else {
				state_size += this->scopes[a_index]->num_outputs;
			}
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// do nothing
			} else {
				if (this->active_compress[a_index]) {
					state_size = this->compress_new_sizes[a_index];
				} else {
					int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
					state_size -= compress_size;
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			state_size = this->branches[a_index]->num_outputs;
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			state_size = this->folds[a_index]->num_outputs;
		}
	}

	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_is_try = true;
	this->explore_curr_try = 0;
	this->explore_target_tries = 1;
	// int rand_scale = rand()%5;
	int rand_scale = 1;
	for (int i = 0; i < rand_scale; i++) {
		this->explore_target_tries *= 10;
	}
	this->best_explore_surprise = 0.0;
	this->explore_fold = NULL;
}

BranchPath::BranchPath(ifstream& input_file) {
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

	string full_last_line;
	getline(input_file, full_last_line);
	this->full_last = stoi(full_last_line);

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

			this->scopes.push_back(solution->scope_dictionary[scope_id]);

			this->actions.push_back(Action());

			string inner_input_networks_size_line;
			getline(input_file, inner_input_networks_size_line);
			int inner_input_networks_size = stoi(inner_input_networks_size_line);

			this->inner_input_networks.push_back(vector<FoldNetwork*>());
			this->inner_input_sizes.push_back(vector<int>());
			for (int i_index = 0; i_index < inner_input_networks_size; i_index++) {
				ifstream inner_input_network_save_file;
				inner_input_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
				this->inner_input_networks[a_index].push_back(new FoldNetwork(inner_input_network_save_file));
				inner_input_network_save_file.close();

				string inner_input_size_line;
				getline(input_file, inner_input_size_line);
				this->inner_input_sizes[a_index].push_back(stoi(inner_input_size_line));
			}

			ifstream scope_scale_mod_save_file;
			scope_scale_mod_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_scope_scale_mod_" + to_string(a_index) + ".txt");
			this->scope_scale_mod.push_back(new Network(scope_scale_mod_save_file));
			scope_scale_mod_save_file.close();
		} else {
			this->scopes.push_back(NULL);

			this->actions.push_back(Action(input_file));

			this->inner_input_networks.push_back(vector<FoldNetwork*>());
			this->inner_input_sizes.push_back(vector<int>());
			this->scope_scale_mod.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		string step_type_line;
		getline(input_file, step_type_line);
		this->step_types.push_back(stoi(step_type_line));

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				this->score_networks.push_back(NULL);
				this->confidence_networks.push_back(NULL);

				this->active_compress.push_back(false);	// doesn't matter
				this->compress_new_sizes.push_back(-1);
				this->compress_networks.push_back(NULL);
				this->compress_original_sizes.push_back(-1);
			} else {
				if (a_index != 0) {
					ifstream score_network_save_file;
					score_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
					this->score_networks.push_back(new FoldNetwork(score_network_save_file));
					score_network_save_file.close();

					ifstream confidence_network_save_file;
					confidence_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_confidence_" + to_string(a_index) + ".txt");
					this->confidence_networks.push_back(new FoldNetwork(confidence_network_save_file));
					confidence_network_save_file.close();
				} else {
					this->score_networks.push_back(NULL);
					this->confidence_networks.push_back(NULL);
				}

				string active_compress_line;
				getline(input_file, active_compress_line);
				this->active_compress.push_back(stoi(active_compress_line));

				string compress_new_size_line;
				getline(input_file, compress_new_size_line);
				this->compress_new_sizes.push_back(stoi(compress_new_size_line));

				if (this->active_compress[a_index]) {
					ifstream compress_network_save_file;
					compress_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_compress_" + to_string(a_index) + ".txt");
					this->compress_networks.push_back(new FoldNetwork(compress_network_save_file));
					compress_network_save_file.close();
				} else {
					this->compress_networks.push_back(NULL);
				}

				string compress_original_size_line;
				getline(input_file, compress_original_size_line);
				this->compress_original_sizes.push_back(stoi(compress_original_size_line));
			}

			this->branches.push_back(NULL);
			this->folds.push_back(NULL);
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->score_networks.push_back(NULL);
			this->confidence_networks.push_back(NULL);

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
			if (a_index != 0) {
				ifstream score_network_save_file;
				score_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
				this->score_networks.push_back(new FoldNetwork(score_network_save_file));
				score_network_save_file.close();

				ifstream confidence_network_save_file;
				confidence_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_confidence_" + to_string(a_index) + ".txt");
				this->confidence_networks.push_back(new FoldNetwork(confidence_network_save_file));
				confidence_network_save_file.close();
			} else {
				this->score_networks.push_back(NULL);
				this->confidence_networks.push_back(NULL);
			}

			string fold_id_line;
			getline(input_file, fold_id_line);
			int fold_id = stoi(fold_id_line);

			ifstream fold_save_file;
			fold_save_file.open("saves/fold_" + to_string(fold_id) + ".txt");
			this->folds.push_back(new Fold(fold_save_file));
			fold_save_file.close();

			this->active_compress.push_back(false);	// doesn't matter
			this->compress_new_sizes.push_back(-1);
			this->compress_networks.push_back(NULL);
			this->compress_original_sizes.push_back(-1);

			this->branches.push_back(NULL);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
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

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stof(average_score_line);

	string score_variance_line;
	getline(input_file, score_variance_line);
	this->score_variance = stof(score_variance_line);

	string average_misguess_line;
	getline(input_file, average_misguess_line);
	this->average_misguess = stof(average_misguess_line);

	string misguess_variance_line;
	getline(input_file, misguess_variance_line);
	this->misguess_variance = stof(misguess_variance_line);

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		string starting_state_size_line;
		getline(input_file, starting_state_size_line);
		this->starting_state_sizes.push_back(stoi(starting_state_size_line));
	}

	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_is_try = true;
	this->explore_curr_try = 0;
	this->explore_target_tries = 1;
	// int rand_scale = rand()%5;
	int rand_scale = 1;
	for (int i = 0; i < rand_scale; i++) {
		this->explore_target_tries *= 10;
	}
	this->best_explore_surprise = 0.0;
	this->explore_fold = NULL;
}

BranchPath::~BranchPath() {
	// scopes owned and deleted by solution

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
			delete this->inner_input_networks[a_index][i_index];
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->scope_scale_mod[a_index] != NULL) {
			delete this->scope_scale_mod[a_index];
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
		if (this->confidence_networks[a_index] != NULL) {
			delete this->confidence_networks[a_index];
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

void BranchPath::explore_on_path_activate(Problem& problem,
										  double starting_score,
										  double starting_predicted_misguess,
										  vector<double>& local_s_input_vals,
										  vector<double>& local_state_vals,	// i.e., combined initially
										  double& predicted_score,
										  double& scale_factor,
										  RunStatus& run_status,
										  BranchPathHistory* history) {
	if (this->explore_type == EXPLORE_TYPE_NONE) {
		double sum_impact = 0.0;
		for (int a_index = 0; a_index < this->sequence_length; a_index++) {
			if (this->is_inner_scope[a_index]) {
				// sum_impact += this->average_inner_scope_impacts[a_index];
				sum_impact += 1.0;
			}

			if (this->step_types[a_index] == STEP_TYPE_STEP) {
				if (a_index == this->sequence_length-1 && !this->full_last) {
					// do nothing
				} else {
					// sum_impact += this->average_local_impacts[a_index];
					sum_impact += 1.0;
				}
			} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
				// sum_impact += this->average_inner_branch_impacts[a_index];
				sum_impact += 1.0;
			} else {
				// this->step_types[a_index] == STEP_TYPE_FOLD
				// do nothing
			}
		}

		double rand_val = randuni()*sum_impact;

		for (int a_index = 0; a_index < this->sequence_length; a_index++) {
			if (this->is_inner_scope[a_index]) {
				// rand_val -= this->average_inner_scope_impacts[a_index];
				rand_val -= 1.0;
				if (rand_val <= 0.0) {
					if (rand()%2 == 0) {
						this->explore_type = EXPLORE_TYPE_INNER_SCOPE;
						this->explore_is_try = true;
						this->explore_index_inclusive = a_index;
						this->explore_end_non_inclusive = -1;
					} else {
						this->explore_type = EXPLORE_TYPE_NEW;
						this->explore_is_try = true;
						this->explore_index_inclusive = a_index;
						this->explore_end_non_inclusive = a_index + 1 + rand()%(this->sequence_length - a_index);

						bool can_be_empty;
						if (this->explore_end_non_inclusive > this->explore_index_inclusive+1) {
							can_be_empty = true;
						} else {
							can_be_empty = false;
						}

						solution->new_sequence(this->curr_explore_sequence_length,
											   this->curr_explore_new_sequence_types,
											   this->curr_explore_existing_scope_ids,
											   this->curr_explore_existing_action_ids,
											   this->curr_explore_new_actions,
											   can_be_empty);
					}
					break;
				}
			}

			if (this->step_types[a_index] == STEP_TYPE_STEP) {
				if (a_index == this->sequence_length-1 && !this->full_last) {
					// do nothing
				} else {
					// rand_val -= this->average_local_impacts[a_index];
					rand_val -= 1.0;
					if (rand_val <= 0.0) {
						this->explore_type = EXPLORE_TYPE_NEW;
						this->explore_is_try = true;
						this->explore_index_inclusive = a_index;
						this->explore_end_non_inclusive = a_index + 1 + rand()%(this->sequence_length - a_index);

						bool can_be_empty;
						if (this->explore_end_non_inclusive > this->explore_index_inclusive+1) {
							can_be_empty = true;
						} else {
							can_be_empty = false;
						}

						solution->new_sequence(this->curr_explore_sequence_length,
											   this->curr_explore_new_sequence_types,
											   this->curr_explore_existing_scope_ids,
											   this->curr_explore_existing_action_ids,
											   this->curr_explore_new_actions,
											   can_be_empty);

						break;
					}
				}
			} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
				// rand_val -= this->average_inner_branch_impacts[a_index];
				rand_val -= 1.0;
				if (rand_val <= 0.0) {
					// Note: don't worry about explore after branch as there's either inner scope in-between, or an action performed first
					if (rand()%2 == 0) {
						this->explore_type = EXPLORE_TYPE_INNER_BRANCH;
						this->explore_is_try = true;
						this->explore_index_inclusive = a_index;
						this->explore_end_non_inclusive = -1;
					} else {
						this->explore_type = EXPLORE_TYPE_NEW;
						this->explore_is_try = true;
						this->explore_index_inclusive = a_index;
						this->explore_end_non_inclusive = a_index + 1 + rand()%(this->sequence_length - a_index);

						bool can_be_empty;
						if (this->explore_end_non_inclusive > this->explore_index_inclusive+1) {
							can_be_empty = true;
						} else {
							can_be_empty = false;
						}

						solution->new_sequence(this->curr_explore_sequence_length,
											   this->curr_explore_new_sequence_types,
											   this->curr_explore_existing_scope_ids,
											   this->curr_explore_existing_action_ids,
											   this->curr_explore_new_actions,
											   can_be_empty);
					}
					break;
				}
			} else {
				// this->step_types[a_index] == STEP_TYPE_FOLD
				// do nothing
			}
		}
	}

	int a_index = 1;

	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// starting_score already scaled

		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			if (this->explore_is_try) {
				run_status.existing_score = starting_score;
				run_status.predicted_misguess = starting_predicted_misguess;

				run_status.explore_phase = EXPLORE_PHASE_EXPLORE;

				for (int n_index = 0; n_index < this->curr_explore_sequence_length; n_index++) {
					if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
						Scope* explore_scope = solution->scope_dictionary[this->curr_explore_existing_scope_ids[n_index]];
						vector<double> scope_input(explore_scope->num_inputs, 0.0);
						vector<double> scope_output;	// unused
						ScopeHistory* scope_history = new ScopeHistory(explore_scope);
						explore_scope->existing_update_activate(
							problem,
							scope_input,
							scope_output,
							predicted_score,	// won't be relevant after
							scale_factor,	// won't be relevant after
							run_status,
							scope_history);
						delete scope_history;

						if (run_status.exceeded_depth) {
							history->exit_index = a_index;
							history->exit_location = EXIT_LOCATION_SPOT;
							return;
						}
					} else if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
						problem.perform_action(solution->action_dictionary[this->curr_explore_existing_action_ids[n_index]]);
					} else {
						// this->explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
						problem.perform_action(this->curr_explore_new_actions[n_index]);
					}
				}

				if (this->explore_end_non_inclusive == this->sequence_length) {
					local_state_vals = vector<double>(this->num_outputs, 0.0);
				} else {
					local_state_vals = vector<double>(this->starting_state_sizes[this->explore_end_non_inclusive], 0.0);
				}
				a_index = this->explore_end_non_inclusive;
			} else {
				run_status.explore_location = this;

				FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->explore_on_path_activate(starting_score,
															 problem,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 run_status,
															 explore_fold_history);
				history->explore_fold_history = explore_fold_history;

				run_status.explore_phase = EXPLORE_PHASE_FLAT;

				if (run_status.exceeded_depth) {
					history->exit_index = 0;
					history->exit_location = EXIT_LOCATION_BACK;
					return;
				}

				a_index = this->explore_end_non_inclusive;
			}
		} else {
			// wait until on branch_path to update predicted_score
			history->score_updates[0] = starting_score;
			predicted_score += starting_score;

			if (this->active_compress[0]) {
				// explore_phase == EXPLORE_PHASE_NONE
				this->compress_networks[0]->activate_small(local_s_input_vals,
														   local_state_vals);
				local_state_vals.clear();
				local_state_vals.reserve(this->compress_new_sizes[0]);
				for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
					local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
				}
			} else {
				int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
				local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
			}
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
			this->branches[0]->explore_on_path_activate_score(local_s_input_vals,
															  local_state_vals,
															  scale_factor,
															  run_status,
															  branch_history);
		} else {
			this->branches[0]->explore_off_path_activate_score(local_s_input_vals,
															   local_state_vals,
															   scale_factor,
															   run_status,
															   branch_history);
		}

		if (this->explore_index_inclusive == 0
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			if (this->explore_is_try) {
				run_status.existing_score = branch_history->best_score;
				run_status.predicted_misguess = branch_history->best_predicted_misguess;
				delete branch_history;

				run_status.explore_phase = EXPLORE_PHASE_EXPLORE;

				for (int n_index = 0; n_index < this->curr_explore_sequence_length; n_index++) {
					if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
						Scope* explore_scope = solution->scope_dictionary[this->curr_explore_existing_scope_ids[n_index]];
						vector<double> scope_input(explore_scope->num_inputs, 0.0);
						vector<double> scope_output;	// unused
						ScopeHistory* scope_history = new ScopeHistory(explore_scope);
						explore_scope->existing_update_activate(
							problem,
							scope_input,
							scope_output,
							predicted_score,	// won't be relevant after
							scale_factor,	// won't be relevant after
							run_status,
							scope_history);
						delete scope_history;

						if (run_status.exceeded_depth) {
							history->exit_index = a_index;
							history->exit_location = EXIT_LOCATION_SPOT;
							return;
						}
					} else if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
						problem.perform_action(solution->action_dictionary[this->curr_explore_existing_action_ids[n_index]]);
					} else {
						// this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
						problem.perform_action(this->curr_explore_new_actions[n_index]);
					}
				}

				if (this->explore_end_non_inclusive == this->sequence_length) {
					local_state_vals = vector<double>(this->num_outputs, 0.0);
				} else {
					local_state_vals = vector<double>(this->starting_state_sizes[this->explore_end_non_inclusive], 0.0);
				}
				a_index = this->explore_end_non_inclusive;
			} else {
				double existing_score = branch_history->best_score;
				delete branch_history;

				run_status.explore_location = this;

				FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
				this->explore_fold->explore_on_path_activate(existing_score,
															 problem,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 run_status,
															 explore_fold_history);
				history->explore_fold_history = explore_fold_history;

				run_status.explore_phase = EXPLORE_PHASE_FLAT;

				if (run_status.exceeded_depth) {
					history->exit_index = 0;
					history->exit_location = EXIT_LOCATION_BACK;
					return;
				}

				a_index = this->explore_end_non_inclusive;
			}
		} else {
			if (this->explore_index_inclusive == 0
					&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
				this->branches[0]->explore_on_path_activate(problem,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															run_status,
															branch_history);
			} else {
				this->branches[0]->explore_off_path_activate(problem,
															 local_s_input_vals,
															 local_state_vals,
															 predicted_score,
															 scale_factor,
															 run_status,
															 branch_history);
			}
			history->branch_histories[0] = branch_history;

			if (run_status.exceeded_depth) {
				history->exit_index = 0;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		}
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		// starting_score already scaled

		history->score_updates[0] = starting_score;
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->explore_off_path_activate(problem,
												  starting_score,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  run_status,
												  fold_history);
		history->fold_histories[0] = fold_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	}

	// mid
	while (a_index < this->sequence_length) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
					scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				this->scopes[a_index]->explore_on_path_activate(problem,
																scope_input,
																scope_output,
																predicted_score,
																scale_factor,
																run_status,
																scope_history);
			} else {
				this->scopes[a_index]->explore_off_path_activate(problem,
																 scope_input,
																 scope_output,
																 predicted_score,
																 scale_factor,
																 run_status,
																 scope_history);
			}
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_FRONT;
				return;
			}

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
			} else {
				FoldNetworkHistory* score_network_history = NULL;
				if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
					if (this->explore_is_try) {
						// explore_phase != EXPLORE_PHASE_FLAT, so don't need to delete score_network_history

						// don't worry about backpropping confidence
						this->confidence_networks[a_index]->activate_small(local_s_input_vals,
																		   local_state_vals);
						double predicted_misguess = abs(scale_factor)*this->confidence_networks[a_index]->output->acti_vals[0];

						run_status.existing_score = existing_score;
						run_status.predicted_misguess = predicted_misguess;

						run_status.explore_phase = EXPLORE_PHASE_EXPLORE;

						for (int n_index = 0; n_index < this->curr_explore_sequence_length; n_index++) {
							if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
								Scope* explore_scope = solution->scope_dictionary[this->curr_explore_existing_scope_ids[n_index]];
								vector<double> scope_input(explore_scope->num_inputs, 0.0);
								vector<double> scope_output;	// unused
								ScopeHistory* scope_history = new ScopeHistory(explore_scope);
								explore_scope->existing_update_activate(
									problem,
									scope_input,
									scope_output,
									predicted_score,	// won't be relevant after
									scale_factor,	// won't be relevant after
									run_status,
									scope_history);
								delete scope_history;

								if (run_status.exceeded_depth) {
									history->exit_index = a_index;
									history->exit_location = EXIT_LOCATION_SPOT;
									return;
								}
							} else if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
								problem.perform_action(solution->action_dictionary[this->curr_explore_existing_action_ids[n_index]]);
							} else {
								// this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
								problem.perform_action(this->curr_explore_new_actions[n_index]);
							}
						}

						if (this->explore_end_non_inclusive == this->sequence_length) {
							local_state_vals = vector<double>(this->num_outputs, 0.0);
						} else {
							local_state_vals = vector<double>(this->starting_state_sizes[this->explore_end_non_inclusive], 0.0);
						}
						a_index = this->explore_end_non_inclusive-1;		// account for increment at end
					} else {
						// explore_phase != EXPLORE_PHASE_FLAT, so don't need to delete score_network_history

						run_status.explore_location = this;

						FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
						this->explore_fold->explore_on_path_activate(existing_score,
																	 problem,
																	 local_s_input_vals,
																	 local_state_vals,
																	 predicted_score,
																	 scale_factor,
																	 run_status,
																	 explore_fold_history);
						history->explore_fold_history = explore_fold_history;

						run_status.explore_phase = EXPLORE_PHASE_FLAT;

						if (run_status.exceeded_depth) {
							history->exit_index = a_index;
							history->exit_location = EXIT_LOCATION_BACK;
							return;
						}

						a_index = this->explore_end_non_inclusive-1;	// account for increment at end
					}
				} else {
					history->score_network_histories[a_index] = score_network_history;
					history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
					predicted_score += existing_score;

					if (this->active_compress[a_index]) {
						// compress 2 layers, add 1 layer
						if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
						int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
						local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
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
																		run_status,
																		branch_history);
			} else {
				this->branches[a_index]->explore_off_path_activate_score(local_s_input_vals,
																		 local_state_vals,
																		 scale_factor,
																		 run_status,
																		 branch_history);
			}

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_NEW) {
				if (this->explore_is_try) {
					run_status.existing_score = branch_history->best_score;
					run_status.predicted_misguess = branch_history->best_predicted_misguess;
					delete branch_history;

					run_status.explore_phase = EXPLORE_PHASE_EXPLORE;

					for (int n_index = 0; n_index < this->curr_explore_sequence_length; n_index++) {
						if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
							Scope* explore_scope = solution->scope_dictionary[this->curr_explore_existing_scope_ids[n_index]];
							vector<double> scope_input(explore_scope->num_inputs, 0.0);
							vector<double> scope_output;	// unused
							ScopeHistory* scope_history = new ScopeHistory(explore_scope);
							explore_scope->existing_update_activate(
								problem,
								scope_input,
								scope_output,
								predicted_score,	// won't be relevant after
								scale_factor,	// won't be relevant after
								run_status,
								scope_history);
							delete scope_history;

							if (run_status.exceeded_depth) {
								history->exit_index = a_index;
								history->exit_location = EXIT_LOCATION_SPOT;
								return;
							}
						} else if (this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
							problem.perform_action(solution->action_dictionary[this->curr_explore_existing_action_ids[n_index]]);
						} else {
							// this->curr_explore_new_sequence_types[n_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
							problem.perform_action(this->curr_explore_new_actions[n_index]);
						}
					}

					if (this->explore_end_non_inclusive == this->sequence_length) {
						local_state_vals = vector<double>(this->num_outputs, 0.0);
					} else {
						local_state_vals = vector<double>(this->starting_state_sizes[this->explore_end_non_inclusive], 0.0);
					}
					a_index = this->explore_end_non_inclusive-1;		// account for increment at end
				} else {
					double existing_score = branch_history->best_score;
					delete branch_history;

					run_status.explore_location = this;

					FoldHistory* explore_fold_history = new FoldHistory(this->explore_fold);
					this->explore_fold->explore_on_path_activate(existing_score,
																 problem,
																 local_s_input_vals,
																 local_state_vals,
																 predicted_score,
																 scale_factor,
																 run_status,
																 explore_fold_history);
					history->explore_fold_history = explore_fold_history;

					run_status.explore_phase = EXPLORE_PHASE_FLAT;

					if (run_status.exceeded_depth) {
						history->exit_index = a_index;
						history->exit_location = EXIT_LOCATION_BACK;
						return;
					}

					a_index = this->explore_end_non_inclusive-1;	// account for increment at end
				}
			} else {
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_activate(problem,
																	  local_s_input_vals,
																	  local_state_vals,
																	  predicted_score,
																	  scale_factor,
																	  run_status,
																	  branch_history);
				} else {
					this->branches[a_index]->explore_off_path_activate(problem,
																	   local_s_input_vals,
																	   local_state_vals,
																	   predicted_score,
																	   scale_factor,
																	   run_status,
																	   branch_history);
				}
				history->branch_histories[a_index] = branch_history;

				if (run_status.exceeded_depth) {
					history->exit_index = a_index;
					history->exit_location = EXIT_LOCATION_BACK;
					return;
				}
			}
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			FoldNetworkHistory* score_network_history = NULL;
			if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
				score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals,
															  score_network_history);
				history->score_network_histories[a_index] = score_network_history;
			} else {
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals);
			}
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];

			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->explore_off_path_activate(problem,
															existing_score,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															run_status,
															fold_history);
			history->fold_histories[a_index] = fold_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		}

		a_index++;
	}
}

void BranchPath::explore_off_path_activate(Problem& problem,
										   double starting_score,		// matters when start is not branch
										   vector<double>& local_s_input_vals,
										   vector<double>& local_state_vals,	// i.e., combined initially
										   double& predicted_score,
										   double& scale_factor,
										   RunStatus& run_status,
										   BranchPathHistory* history) {
	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// starting_score already scaled

		// wait until on branch_path to update predicted_score
		history->score_updates[0] = starting_score;
		predicted_score += starting_score;

		if (this->active_compress[0]) {
			if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
			int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
			local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->explore_off_path_activate_score(local_s_input_vals,
														   local_state_vals,
														   scale_factor,
														   run_status,
														   branch_history);

		this->branches[0]->explore_off_path_activate(problem,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 run_status,
													 branch_history);
		history->branch_histories[0] = branch_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		// starting_score already scaled

		history->score_updates[0] = starting_score;
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->explore_off_path_activate(problem,
												  starting_score,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  run_status,
												  fold_history);
		history->fold_histories[0] = fold_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	}

	// mid
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
					scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->explore_off_path_activate(problem,
															 scope_input,
															 scope_output,
															 predicted_score,
															 scale_factor,
															 run_status,
															 scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_FRONT;
				return;
			}

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
			} else {
				if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
					if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
					int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
					local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->explore_off_path_activate_score(local_s_input_vals,
																	 local_state_vals,
																	 scale_factor,
																	 run_status,
																	 branch_history);

			this->branches[a_index]->explore_off_path_activate(problem,
															   local_s_input_vals,
															   local_state_vals,
															   predicted_score,
															   scale_factor,
															   run_status,
															   branch_history);
			history->branch_histories[a_index] = branch_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
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
			this->folds[a_index]->explore_off_path_activate(problem,
															existing_score,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															run_status,
															fold_history);
			history->fold_histories[a_index] = fold_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		}
	}
}

void BranchPath::explore_on_path_backprop(vector<double>& local_s_input_errors,	// don't need to output local_s_input_errors on path but for explore_off_path_backprop
										  vector<double>& local_state_errors,
										  double& predicted_score,
										  double target_val,
										  double final_misguess,
										  double& scale_factor,
										  double& scale_factor_error,
										  BranchPathHistory* history) {
	this->explore_is_try = false;

	int a_index;
	if (history->exit_location != EXIT_LOCATION_NORMAL) {
		a_index = history->exit_index;
	} else {
		if (this->explore_end_non_inclusive == this->sequence_length
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			a_index = this->explore_index_inclusive;
		} else {
			a_index = this->sequence_length-1;
		}
	}

	// mid
	while (a_index >= 1) {
		if (this->explore_index_inclusive == a_index
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			int explore_signal = this->explore_fold->explore_on_path_backprop(
				local_state_errors,
				predicted_score,
				target_val,
				final_misguess,
				scale_factor,
				scale_factor_error,
				history->explore_fold_history);

			if (explore_signal == EXPLORE_SIGNAL_REPLACE) {
				explore_replace();
			} else if (explore_signal == EXPLORE_SIGNAL_BRANCH) {
				explore_branch();
			} else if (explore_signal == EXPLORE_SIGNAL_CLEAN) {
				this->explore_type = EXPLORE_TYPE_NONE;
				this->explore_is_try = true;
				this->explore_curr_try = 0;
				this->explore_target_tries = 1;
				// int rand_scale = rand()%5;
				int rand_scale = 1;
				for (int i = 0; i < rand_scale; i++) {
					this->explore_target_tries *= 10;
				}
				this->best_explore_surprise = 0.0;
				delete this->explore_fold;
				this->explore_fold = NULL;
			}

			return;
		} else {
			if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
				// do nothing
			} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
				if (a_index == this->sequence_length-1 && !this->full_last) {
					// scope end for outer scope -- do nothing
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

					scale_factor_error += history->score_updates[a_index]*predicted_score_error;

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

					predicted_score -= scale_factor*history->score_updates[a_index];
				}
			} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_backprop(local_s_input_errors,
																	  local_state_errors,
																	  predicted_score,
																	  target_val,
																	  final_misguess,
																	  scale_factor,
																	  scale_factor_error,
																	  history->branch_histories[a_index]);

					this->explore_count++;
					if (this->branches[a_index]->explore_ref_count == 0) {
						this->explore_type = EXPLORE_TYPE_NONE;
					}

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

				scale_factor_error += history->score_updates[a_index]*score_predicted_score_error;

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
			// obs_size always 1 for sorting
			local_state_errors.pop_back();
		} else {
			vector<double> scope_input_errors;
			if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
				local_state_errors = vector<double>(this->starting_state_sizes[a_index], 0.0);
			} else {
				scope_input_errors = vector<double>(local_state_errors.end()-this->scopes[a_index]->num_outputs,
					local_state_errors.end());
				local_state_errors.erase(local_state_errors.end()-this->scopes[a_index]->num_outputs,
					local_state_errors.end());
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			if (this->explore_index_inclusive == a_index
					&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
				scale_factor_error /= scope_scale_mod_val;

				this->scopes[a_index]->explore_on_path_backprop(scope_input_errors,
																predicted_score,
																target_val,
																final_misguess,
																scale_factor,
																scale_factor_error,
																history->scope_histories[a_index]);

				this->explore_count++;
				if (this->scopes[a_index]->explore_type == EXPLORE_TYPE_NONE) {
					this->explore_type = EXPLORE_TYPE_NONE;
				}

				return;
			} else {
				vector<double> scope_output_errors;
				double scope_scale_factor_error = 0.0;
				this->scopes[a_index]->explore_off_path_backprop(scope_input_errors,
																 scope_output_errors,
																 predicted_score,
																 target_val,
																 scale_factor,
																 scope_scale_factor_error,
																 history->scope_histories[a_index]);

				scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

				scale_factor /= scope_scale_mod_val;

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
			final_misguess,
			scale_factor,
			scale_factor_error,
			history->explore_fold_history);

		if (explore_signal == EXPLORE_SIGNAL_REPLACE) {
			explore_replace();
		} else if (explore_signal == EXPLORE_SIGNAL_BRANCH) {
			explore_branch();
		} else if (explore_signal == EXPLORE_SIGNAL_CLEAN) {
			this->explore_type = EXPLORE_TYPE_NONE;
			this->explore_is_try = true;
			this->explore_curr_try = 0;
			this->explore_target_tries = 1;
			// int rand_scale = rand()%5;
			int rand_scale = 1;
			for (int i = 0; i < rand_scale; i++) {
				this->explore_target_tries *= 10;
			}
			this->best_explore_surprise = 0.0;
			delete this->explore_fold;
			this->explore_fold = NULL;
		}

		return;
	} else {
		// this->step_types[0] == STEP_TYPE_BRANCH
		// this->explore_index_inclusive == 0 && this->explore_type == EXPLORE_TYPE_INNER_BRANCH
		this->branches[0]->explore_on_path_backprop(local_s_input_errors,
													local_state_errors,
													predicted_score,
													target_val,
													final_misguess,
													scale_factor,
													scale_factor_error,
													history->branch_histories[0]);

		this->explore_count++;
		if (this->branches[0]->explore_ref_count == 0) {
			this->explore_type = EXPLORE_TYPE_NONE;
		}

		return;
	}
}

void BranchPath::explore_off_path_backprop(vector<double>& local_s_input_errors,
										   vector<double>& local_state_errors,
										   double& predicted_score,
										   double target_val,
										   double& scale_factor,
										   double& scale_factor_error,
										   BranchPathHistory* history) {
	// mid
	for (int a_index = history->exit_index; a_index >= 1; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
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

				scale_factor_error += history->score_updates[a_index]*predicted_score_error;

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

				predicted_score -= scale_factor*history->score_updates[a_index];
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

			scale_factor_error += history->score_updates[a_index]*score_predicted_score_error;

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
			// obs_size always 1 for sorting
			local_state_errors.pop_back();
		} else {
			vector<double> scope_input_errors;
			if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
				local_state_errors = vector<double>(this->starting_state_sizes[a_index], 0.0);
			} else {
				scope_input_errors = vector<double>(local_state_errors.end()-this->scopes[a_index]->num_outputs,
					local_state_errors.end());
				local_state_errors.erase(local_state_errors.end()-this->scopes[a_index]->num_outputs,
					local_state_errors.end());
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->explore_off_path_backprop(scope_input_errors,
															 scope_output_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 scope_scale_factor_error,
															 history->scope_histories[a_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

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

		// start step score errors handled in branch

		// starting score already scaled
		predicted_score -= history->score_updates[0];
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

		// start step score errors handled in branch

		// predicted_score already modified to before fold value in fold
	}
}

void BranchPath::existing_flat_activate(Problem& problem,
										double starting_score,		// matters when start is not branch
										vector<double>& local_s_input_vals,
										vector<double>& local_state_vals,	// i.e., combined initially
										double& predicted_score,
										double& scale_factor,
										RunStatus& run_status,
										BranchPathHistory* history) {
	if (this == run_status.explore_location) {
		run_status.is_recursive = true;
	}

	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// starting_score already scaled

		// wait until on branch_path to update predicted_score
		history->score_updates[0] = starting_score;
		predicted_score += starting_score;

		if (this->active_compress[0]) {
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
			int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
			local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->existing_flat_activate(problem,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  run_status,
												  branch_history);
		history->branch_histories[0] = branch_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		// starting_score already scaled

		history->score_updates[0] = starting_score;
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->existing_flat_activate(problem,
											   starting_score,
											   local_s_input_vals,
											   local_state_vals,
											   predicted_score,
											   scale_factor,
											   run_status,
											   fold_history);
		history->fold_histories[0] = fold_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	}

	// mid
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				FoldNetworkHistory* inner_input_network_history = new FoldNetworkHistory(this->inner_input_networks[a_index][i_index]);
				this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																			 local_state_vals,
																			 inner_input_network_history);
				history->inner_input_network_histories[a_index].push_back(inner_input_network_history);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->existing_flat_activate(problem,
														  scope_input,
														  scope_output,
														  predicted_score,
														  scale_factor,
														  run_status,
														  scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_FRONT;
				return;
			}

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
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
					int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
					local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->existing_flat_activate(problem,
															local_s_input_vals,
															local_state_vals,
															predicted_score,
															scale_factor,
															run_status,
															branch_history);
			history->branch_histories[a_index] = branch_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
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
			this->folds[a_index]->existing_flat_activate(problem,
														 existing_score,
														 local_s_input_vals,
														 local_state_vals,
														 predicted_score,
														 scale_factor,
														 run_status,
														 fold_history);
			history->fold_histories[a_index] = fold_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		}
	}
}

void BranchPath::existing_flat_backprop(vector<double>& local_s_input_errors,
										vector<double>& local_state_errors,
										double& predicted_score,
										double predicted_score_error,
										double& scale_factor,
										double& scale_factor_error,
										BranchPathHistory* history) {
	// mid
	for (int a_index = history->exit_index; a_index >= 1; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
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

				scale_factor_error += history->score_updates[a_index]*predicted_score_error;

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

				predicted_score -= scale_factor*history->score_updates[a_index];
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

			scale_factor_error += history->score_updates[a_index]*predicted_score_error;

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
			// obs_size always 1 for sorting
			local_state_errors.pop_back();
		} else {
			vector<double> scope_input_errors;
			if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
				local_state_errors = vector<double>(this->starting_state_sizes[a_index], 0.0);
			} else {
				scope_input_errors = vector<double>(local_state_errors.end()-this->scopes[a_index]->num_outputs,
					local_state_errors.end());
				local_state_errors.erase(local_state_errors.end()-this->scopes[a_index]->num_outputs,
					local_state_errors.end());
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->existing_flat_backprop(scope_input_errors,
														  scope_output_errors,
														  predicted_score,
														  predicted_score_error,
														  scale_factor,
														  scope_scale_factor_error,
														  history->scope_histories[a_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

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

		// start step score errors handled in branch

		// starting score already scaled
		predicted_score -= history->score_updates[0];
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

		// start step score errors handled in branch

		// predicted_score already modified to before fold value in fold
	}
}

void BranchPath::update_activate(Problem& problem,
								 double starting_score,		// matters when start is not branch
								 vector<double>& local_s_input_vals,
								 vector<double>& local_state_vals,	// i.e., combined initially
								 double& predicted_score,
								 double& scale_factor,
								 RunStatus& run_status,
								 BranchPathHistory* history) {
	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// starting_score already scaled

		// wait until on branch_path to update predicted_score
		history->score_updates[0] = starting_score;
		predicted_score += starting_score;

		if (this->active_compress[0]) {
			this->compress_networks[0]->activate_small(local_s_input_vals,
													   local_state_vals);

			local_state_vals.clear();
			local_state_vals.reserve(this->compress_new_sizes[0]);
			for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		} else {
			int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
			local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->update_activate(problem,
										   local_s_input_vals,
										   local_state_vals,
										   predicted_score,
										   scale_factor,
										   run_status,
										   branch_history);
		history->branch_histories[0] = branch_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		// starting_score already scaled

		history->score_updates[0] = starting_score;
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->update_activate(problem,
										starting_score,
										local_s_input_vals,
										local_state_vals,
										predicted_score,
										scale_factor,
										run_status,
										fold_history);
		history->fold_histories[0] = fold_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	}

	// mid
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																			 local_state_vals);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->update_activate(problem,
												   scope_input,
												   scope_output,
												   predicted_score,
												   scale_factor,
												   run_status,
												   scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_FRONT;
				return;
			}

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
			} else {
				FoldNetworkHistory* score_network_history = new FoldNetworkHistory(this->score_networks[a_index]);
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals,
															  score_network_history);
				history->score_network_histories[a_index] = score_network_history;
				history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
				predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

				FoldNetworkHistory* confidence_network_history = new FoldNetworkHistory(this->confidence_networks[a_index]);
				this->confidence_networks[a_index]->activate_small(local_s_input_vals,
																   local_state_vals,
																   confidence_network_history);
				history->confidence_network_histories[a_index] = confidence_network_history;
				history->confidence_network_outputs[a_index] = this->confidence_networks[a_index]->output->acti_vals[0];

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
					int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
					local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->update_activate(problem,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 run_status,
													 branch_history);
			history->branch_histories[a_index] = branch_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
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

			FoldNetworkHistory* confidence_network_history = new FoldNetworkHistory(this->confidence_networks[a_index]);
			this->confidence_networks[a_index]->activate_small(local_s_input_vals,
															   local_state_vals,
															   confidence_network_history);
			history->confidence_network_histories[a_index] = confidence_network_history;
			history->confidence_network_outputs[a_index] = this->confidence_networks[a_index]->output->acti_vals[0];

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->update_activate(problem,
												  existing_score,
												  local_s_input_vals,
												  local_state_vals,
												  predicted_score,
												  scale_factor,
												  run_status,
												  fold_history);
			history->fold_histories[a_index] = fold_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		}
	}
}

void BranchPath::update_backprop(double& predicted_score,
								 double target_val,
								 double final_misguess,
								 double& scale_factor,
								 double& scale_factor_error,
								 BranchPathHistory* history) {
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*final_misguess;
	double curr_misguess_variance = (this->average_misguess - final_misguess)*(this->average_misguess - final_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_score_variance = (this->average_score - target_val)*(this->average_score - target_val);
	this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;

	// mid
	for (int a_index = history->exit_index; a_index >= 1; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
			} else {
				double predicted_score_error = target_val - predicted_score;

				double confidence_error = abs(predicted_score_error) - abs(scale_factor)*history->confidence_network_outputs[a_index];
				vector<double> confidence_errors{abs(scale_factor)*confidence_error};
				this->confidence_networks[a_index]->backprop_small_weights_with_no_error_signal(
					confidence_errors,
					0.001,
					history->confidence_network_histories[a_index]);

				scale_factor_error += history->score_updates[a_index]*predicted_score_error;

				vector<double> score_errors{scale_factor*predicted_score_error};
				this->score_networks[a_index]->backprop_small_weights_with_no_error_signal(
					score_errors,
					0.001,
					history->score_network_histories[a_index]);

				predicted_score -= scale_factor*history->score_updates[a_index];

				this->average_local_impacts[a_index] = 0.9999*this->average_local_impacts[a_index]
					+ 0.0001*abs(scale_factor*history->score_updates[a_index]);
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			double ending_predicted_score = predicted_score;

			this->branches[a_index]->update_backprop(predicted_score,
													 target_val,
													 final_misguess,
													 scale_factor,
													 scale_factor_error,
													 history->branch_histories[a_index]);

			double starting_predicted_score = predicted_score;
			this->average_inner_branch_impacts[a_index] = 0.9999*this->average_inner_branch_impacts[a_index]
				+ 0.0001*abs(ending_predicted_score - starting_predicted_score);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->folds[a_index]->update_backprop(predicted_score,
												  target_val,
												  final_misguess,
												  scale_factor,
												  scale_factor_error,
												  history->fold_histories[a_index]);

			// predicted_score already modified to before fold value in fold
			double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
			double score_predicted_score_error = target_val - score_predicted_score;

			double confidence_error = abs(score_predicted_score_error) - abs(scale_factor)*history->confidence_network_outputs[a_index];
			vector<double> confidence_errors{abs(scale_factor)*confidence_error};
			this->confidence_networks[a_index]->backprop_small_weights_with_no_error_signal(
				confidence_errors,
				0.001,
				history->confidence_network_histories[a_index]);

			scale_factor_error += history->score_updates[a_index]*score_predicted_score_error;

			vector<double> score_errors{scale_factor*score_predicted_score_error};
			this->score_networks[a_index]->backprop_small_weights_with_no_error_signal(
				score_errors,
				0.001,
				history->score_network_histories[a_index]);

			// local_impact kept track of as starting impact in fold
		}

		if (!this->is_inner_scope[a_index]) {
			// do nothing
		} else {
			double ending_predicted_score = predicted_score;

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->update_backprop(predicted_score,
												   target_val,
												   final_misguess,
												   scale_factor,
												   scope_scale_factor_error,
												   history->scope_histories[a_index]);

			vector<double> mod_errors{scope_scale_factor_error};
			this->scope_scale_mod[a_index]->backprop(mod_errors, 0.0002);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

			double starting_predicted_score = predicted_score;
			this->average_inner_scope_impacts[a_index] = 0.9999*this->average_inner_scope_impacts[a_index]
				+ 0.0001*abs(ending_predicted_score - starting_predicted_score);
		}
	}

	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// start step score errors handled in branch

		// starting score already scaled
		predicted_score -= history->score_updates[0];

		this->average_local_impacts[0] = 0.9999*this->average_local_impacts[0]
			+ 0.0001*abs(history->score_updates[0]);
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		double ending_predicted_score = predicted_score;

		this->branches[0]->update_backprop(predicted_score,
										   target_val,
										   final_misguess,
										   scale_factor,
										   scale_factor_error,
										   history->branch_histories[0]);

		double starting_predicted_score = predicted_score;
		this->average_inner_branch_impacts[0] = 0.9999*this->average_inner_branch_impacts[0]
			+ 0.0001*abs(ending_predicted_score - starting_predicted_score);
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		this->folds[0]->update_backprop(predicted_score,
										target_val,
										final_misguess,
										scale_factor,
										scale_factor_error,
										history->fold_histories[0]);

		// start step score errors handled in branch

		// predicted_score already modified to before fold value in fold

		// local_impact kept track of as starting impact in fold
	}
}

void BranchPath::existing_update_activate(Problem& problem,
										  double starting_score,
										  vector<double>& local_s_input_vals,
										  vector<double>& local_state_vals,
										  double& predicted_score,
										  double& scale_factor,
										  RunStatus& run_status,
										  BranchPathHistory* history) {
	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// starting_score already scaled

		// wait until on branch_path to update predicted_score
		history->score_updates[0] = starting_score;
		predicted_score += starting_score;

		if (this->active_compress[0]) {
			this->compress_networks[0]->activate_small(local_s_input_vals,
													   local_state_vals);

			local_state_vals.clear();
			local_state_vals.reserve(this->compress_new_sizes[0]);
			for (int s_index = 0; s_index < this->compress_new_sizes[0]; s_index++) {
				local_state_vals.push_back(this->compress_networks[0]->output->acti_vals[s_index]);
			}
		} else {
			int compress_size = this->compress_original_sizes[0] - this->compress_new_sizes[0];
			local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
		}
	} else if (this->step_types[0] == STEP_TYPE_BRANCH) {
		BranchHistory* branch_history = new BranchHistory(this->branches[0]);
		this->branches[0]->existing_update_activate(problem,
													local_s_input_vals,
													local_state_vals,
													predicted_score,
													scale_factor,
													run_status,
													branch_history);
		history->branch_histories[0] = branch_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	} else {
		// this->step_types[0] == STEP_TYPE_FOLD
		// starting_score already scaled

		history->score_updates[0] = starting_score;
		// predicted_score updated in fold

		FoldHistory* fold_history = new FoldHistory(this->folds[0]);
		this->folds[0]->existing_update_activate(problem,
												 starting_score,
												 local_s_input_vals,
												 local_state_vals,
												 predicted_score,
												 scale_factor,
												 run_status,
												 fold_history);
		history->fold_histories[0] = fold_history;

		if (run_status.exceeded_depth) {
			history->exit_index = 0;
			history->exit_location = EXIT_LOCATION_BACK;
			return;
		}
	}

	// mid
	for (int a_index = 1; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			// scopes formed through folding may have multiple inner_input_networks
			// reused scopes will have 1 inner_input_network
			vector<double> scope_input;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																			 local_state_vals);
				for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
					scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
				}
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			this->scopes[a_index]->existing_update_activate(problem,
															scope_input,
															scope_output,
															predicted_score,
															scale_factor,
															run_status,
															scope_history);
			history->scope_histories[a_index] = scope_history;

			scale_factor /= scope_scale_mod_val;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_FRONT;
				return;
			}

			local_state_vals.insert(local_state_vals.end(),
				scope_output.begin(), scope_output.end());
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
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
					int compress_size = this->compress_original_sizes[a_index] - this->compress_new_sizes[a_index];
					local_state_vals.erase(local_state_vals.end()-compress_size, local_state_vals.end());
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			this->branches[a_index]->existing_update_activate(problem,
															  local_s_input_vals,
															  local_state_vals,
															  predicted_score,
															  scale_factor,
															  run_status,
															  branch_history);
			history->branch_histories[a_index] = branch_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->score_networks[a_index]->activate_small(local_s_input_vals,
														  local_state_vals);
			history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
			double existing_score = scale_factor*this->score_networks[a_index]->output->acti_vals[0];
			// predicted_score updated in fold

			FoldHistory* fold_history = new FoldHistory(this->folds[a_index]);
			this->folds[a_index]->existing_update_activate(problem,
														   existing_score,
														   local_s_input_vals,
														   local_state_vals,
														   predicted_score,
														   scale_factor,
														   run_status,
														   fold_history);
			history->fold_histories[a_index] = fold_history;

			if (run_status.exceeded_depth) {
				history->exit_index = a_index;
				history->exit_location = EXIT_LOCATION_BACK;
				return;
			}
		}
	}
}

void BranchPath::existing_update_backprop(double& predicted_score,
										  double predicted_score_error,
										  double& scale_factor,
										  double& scale_factor_error,
										  BranchPathHistory* history) {
	// mid
	for (int a_index = history->exit_index; a_index >= 1; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end for outer scope -- do nothing
			} else {
				scale_factor_error += history->score_updates[a_index]*predicted_score_error;

				predicted_score -= scale_factor*history->score_updates[a_index];
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

			scale_factor_error += history->score_updates[a_index]*predicted_score_error;

			// predicted_score updated in fold
		}

		if (!this->is_inner_scope[a_index]) {
			// do nothing
		} else {
			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			double scope_scale_factor_error = 0.0;
			this->scopes[a_index]->existing_update_backprop(predicted_score,
															predicted_score_error,
															scale_factor,
															scope_scale_factor_error,
															history->scope_histories[a_index]);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;
		}
	}

	// start
	if (this->step_types[0] == STEP_TYPE_STEP) {
		// can't be scope end

		// start step score errors handled in branch

		// starting score already scaled
		predicted_score -= history->score_updates[0];
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

		// start step score errors handled in branch

		// predicted_score already modified to before fold value in fold
	}
}

void BranchPath::explore_set(double target_val,
							 double existing_score,
							 double predicted_misguess,
							 BranchPathHistory* history) {
	if (this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
		this->scopes[this->explore_index_inclusive]->explore_set(
			target_val,
			existing_score,
			predicted_misguess,
			history->scope_histories[this->explore_index_inclusive]);

		if (this->explore_is_try) {
			this->explore_is_try = false;
			this->explore_count = 0;
		}
	} else if (this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
		this->branches[this->explore_index_inclusive]->explore_set(
			target_val,
			existing_score,
			predicted_misguess,
			history->branch_histories[this->explore_index_inclusive]);

		if (this->explore_is_try) {
			this->explore_is_try = false;
			this->explore_count = 0;
		}
	} else {
		// history->explore_type == EXPLORE_TYPE_NEW
		// if (predicted_misguess <= 0.0
		// 		|| (target_val-existing_score)/predicted_misguess > this->best_explore_surprise) {
		if (true) {
			this->best_explore_surprise = (target_val-existing_score)/predicted_misguess;
			this->best_explore_index_inclusive = this->explore_index_inclusive;
			this->best_explore_end_non_inclusive = this->explore_end_non_inclusive;
			this->best_explore_sequence_length = this->curr_explore_sequence_length;
			this->best_explore_new_sequence_types = this->curr_explore_new_sequence_types;
			this->best_explore_existing_scope_ids = this->curr_explore_existing_scope_ids;
			this->best_explore_existing_action_ids = this->curr_explore_existing_action_ids;
			this->best_explore_new_actions = this->curr_explore_new_actions;
		}

		this->curr_explore_new_sequence_types.clear();
		this->curr_explore_existing_scope_ids.clear();
		this->curr_explore_existing_action_ids.clear();
		this->curr_explore_new_actions.clear();

		this->explore_curr_try++;
		if (predicted_misguess <= 0.0
				|| this->explore_curr_try >= this->explore_target_tries) {
			vector<bool> new_is_inner_scope;
			vector<Scope*> new_scopes;
			vector<Action> new_actions;
			for (int s_index = 0; s_index < this->best_explore_sequence_length; s_index++) {
				if (this->best_explore_new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
					new_is_inner_scope.push_back(true);
					new_scopes.push_back(solution->scope_dictionary[this->best_explore_existing_scope_ids[s_index]]);
					new_actions.push_back(Action());
				} else if (this->best_explore_new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
					new_is_inner_scope.push_back(false);
					new_scopes.push_back(NULL);
					new_actions.push_back(solution->action_dictionary[this->best_explore_existing_action_ids[s_index]]);
				} else {
					// this->explore_new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
					new_is_inner_scope.push_back(false);
					new_scopes.push_back(NULL);
					new_actions.push_back(this->best_explore_new_actions[s_index]);
				}
			}

			int new_num_inputs = this->starting_state_sizes[this->best_explore_index_inclusive];
			if (this->explore_index_inclusive != 0) {
				if (!this->is_inner_scope[this->best_explore_index_inclusive]) {
					// obs_size always 1 for sorting
					new_num_inputs++;
				} else {
					new_num_inputs += this->scopes[this->best_explore_index_inclusive]->num_outputs;
				}
			}
			int new_num_outputs;
			if (this->best_explore_end_non_inclusive == this->sequence_length) {
				new_num_outputs = this->num_outputs;
			} else {
				new_num_outputs = this->starting_state_sizes[this->best_explore_end_non_inclusive];
			}

			this->explore_fold = new Fold(new_num_inputs,
										  new_num_outputs,
										  this->outer_s_input_size,	// differs from scope
										  this->best_explore_sequence_length,
										  new_is_inner_scope,
										  new_scopes,
										  new_actions,
										  this->best_explore_end_non_inclusive-this->best_explore_index_inclusive-1,
										  &this->average_score,
										  &this->average_misguess);

			solution->new_sequence_success(this->best_explore_sequence_length,
										   this->best_explore_new_sequence_types,
										   this->best_explore_existing_scope_ids,
										   this->best_explore_existing_action_ids,
										   this->best_explore_new_actions);

			cout << "EXPLORE_SET" << endl;
			cout << "this->best_explore_surprise: " << this->best_explore_surprise << endl;
			cout << "this->explore_curr_try: " << this->explore_curr_try << endl;
			cout << "new_sequence:";
			for (int s_index = 0; s_index < this->best_explore_sequence_length; s_index++) {
				if (this->best_explore_new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
					cout << " S_" << this->best_explore_existing_scope_ids[s_index];
				} else if (this->best_explore_new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
					cout << " " << solution->action_dictionary[this->best_explore_existing_action_ids[s_index]].to_string();
				} else {
					// this->best_explore_new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
					cout << " " << this->best_explore_new_actions[s_index].to_string();
				}
			}
			cout << endl;

			this->explore_index_inclusive = this->best_explore_index_inclusive;
			this->explore_end_non_inclusive = this->best_explore_end_non_inclusive;
			this->explore_is_try = false;
		} else {
			this->explore_type = EXPLORE_TYPE_NONE;
		}
	}
}

void BranchPath::explore_clear(BranchPathHistory* history) {
	if (history->exit_location == EXIT_LOCATION_SPOT) {
		return;
	}

	if (this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
		this->scopes[this->explore_index_inclusive]->explore_clear(
			history->scope_histories[this->explore_index_inclusive]);

		if (this->explore_is_try) {
			this->explore_type = EXPLORE_TYPE_NONE;
		}
	} else if (this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
		this->branches[this->explore_index_inclusive]->explore_clear(
			history->branch_histories[this->explore_index_inclusive]);

		if (this->explore_is_try) {
			this->explore_type = EXPLORE_TYPE_NONE;
		}
	} else {
		// history->explore_type == EXPLORE_TYPE_NEW
		if (this->explore_is_try) {
			this->explore_type = EXPLORE_TYPE_NONE;
			this->curr_explore_new_sequence_types.clear();
			this->curr_explore_existing_scope_ids.clear();
			this->curr_explore_existing_action_ids.clear();
			this->curr_explore_new_actions.clear();
		}
	}
}

void BranchPath::update_increment(BranchPathHistory* history,
								  vector<Fold*>& folds_to_delete) {
	// mid
	for (int a_index = history->exit_index; a_index >= 1; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			if (history->branch_histories[a_index]->branch == this->branches[a_index]) {
				this->branches[a_index]->update_increment(history->branch_histories[a_index],
														  folds_to_delete);
			}
		} else if (this->step_types[a_index] == STEP_TYPE_FOLD) {
			if (history->fold_histories[a_index]->fold == this->folds[a_index]) {
				this->folds[a_index]->update_increment(history->fold_histories[a_index],
													   folds_to_delete);
			}

			if (history->fold_histories[a_index]->fold == this->folds[a_index]) {
				if (this->folds[a_index]->state == STATE_DONE) {
					resolve_fold(a_index,
								 folds_to_delete);
				}
			}
		}

		if (this->is_inner_scope[a_index]) {
			if (history->scope_histories[a_index]->scope == this->scopes[a_index]) {
				this->scopes[a_index]->update_increment(history->scope_histories[a_index],
														folds_to_delete);
			}
		}
	}

	// start
	if (this->step_types[0] == STEP_TYPE_BRANCH) {
		if (history->branch_histories[0]->branch == this->branches[0]) {
			this->branches[0]->update_increment(history->branch_histories[0],
												folds_to_delete);
		}
	} else if (this->step_types[0] == STEP_TYPE_FOLD) {
		if (history->fold_histories[0]->fold == this->folds[0]) {
			this->folds[0]->update_increment(history->fold_histories[0],
											 folds_to_delete);
		}

		if (history->fold_histories[0]->fold == this->folds[0]) {
			if (this->folds[0]->state == STATE_DONE) {
				resolve_fold(0, folds_to_delete);
			}
		}
	}
}

void BranchPath::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->num_inputs << endl;
	output_file << this->num_outputs << endl;
	output_file << this->outer_s_input_size << endl;

	output_file << this->full_last << endl;

	output_file << this->sequence_length << endl;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->is_inner_scope[a_index] << endl;
		if (this->is_inner_scope[a_index]) {
			output_file << this->scopes[a_index]->id << endl;

			output_file << this->inner_input_networks[a_index].size() << endl;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				ofstream inner_input_network_save_file;
				inner_input_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
				this->inner_input_networks[a_index][i_index]->save(inner_input_network_save_file);
				inner_input_network_save_file.close();

				output_file << this->inner_input_sizes[a_index][i_index] << endl;
			}
			
			ofstream scope_scale_mod_save_file;
			scope_scale_mod_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_scope_scale_mod_" + to_string(a_index) + ".txt");
			this->scope_scale_mod[a_index]->save(scope_scale_mod_save_file);
			scope_scale_mod_save_file.close();
		} else {
			this->actions[a_index].save(output_file);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->step_types[a_index] << endl;

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// do nothing
			} else {
				if (a_index != 0) {
					ofstream score_network_save_file;
					score_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
					this->score_networks[a_index]->save(score_network_save_file);
					score_network_save_file.close();

					ofstream confidence_network_save_file;
					confidence_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_confidence_" + to_string(a_index) + ".txt");
					this->confidence_networks[a_index]->save(confidence_network_save_file);
					confidence_network_save_file.close();
				}

				output_file << this->active_compress[a_index] << endl;
				output_file << this->compress_new_sizes[a_index] << endl;
				if (this->active_compress[a_index]) {
					ofstream compress_network_save_file;
					compress_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_compress_" + to_string(a_index) + ".txt");
					this->compress_networks[a_index]->save(compress_network_save_file);
					compress_network_save_file.close();
				}
				output_file << this->compress_original_sizes[a_index] << endl;
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			output_file << this->branches[a_index]->id << endl;

			ofstream branch_save_file;
			branch_save_file.open("saves/branch_" + to_string(this->branches[a_index]->id) + ".txt");
			this->branches[a_index]->save(branch_save_file);
			branch_save_file.close();
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			if (a_index != 0) {
				ofstream score_network_save_file;
				score_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
				this->score_networks[a_index]->save(score_network_save_file);
				score_network_save_file.close();

				ofstream confidence_network_save_file;
				confidence_network_save_file.open("saves/nns/branch_path_" + to_string(this->id) + "_confidence_" + to_string(a_index) + ".txt");
				this->confidence_networks[a_index]->save(confidence_network_save_file);
				confidence_network_save_file.close();
			}

			output_file << this->folds[a_index]->id << endl;

			ofstream fold_save_file;
			fold_save_file.open("saves/fold_" + to_string(this->folds[a_index]->id) + ".txt");
			this->folds[a_index]->save(fold_save_file);
			fold_save_file.close();
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->average_inner_scope_impacts[a_index] << endl;
		output_file << this->average_local_impacts[a_index] << endl;
		output_file << this->average_inner_branch_impacts[a_index] << endl;
	}

	output_file << this->average_score << endl;
	output_file << this->score_variance << endl;
	output_file << this->average_misguess << endl;
	output_file << this->misguess_variance << endl;
	
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->starting_state_sizes[a_index] << endl;
	}
}

void BranchPath::save_for_display(ofstream& output_file,
								  int curr_scope_id) {
	output_file << this->sequence_length << endl;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->is_inner_scope[a_index] << endl;
		if (this->is_inner_scope[a_index]) {
			output_file << this->scopes[a_index]->id << endl;

			if (this->scopes[a_index]->id > curr_scope_id) {
				this->scopes[a_index]->save_for_display(output_file);
			}
		} else {
			this->actions[a_index].save(output_file);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->step_types[a_index] << endl;

		if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			this->branches[a_index]->save_for_display(output_file,
													  this->id);
		} else if (this->step_types[a_index] == STEP_TYPE_FOLD) {
			this->folds[a_index]->save_for_display(output_file);
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->average_inner_scope_impacts[a_index] << endl;
		output_file << this->average_local_impacts[a_index] << endl;
		output_file << this->average_inner_branch_impacts[a_index] << endl;
	}
}

BranchPathHistory::BranchPathHistory(BranchPath* branch_path) {
	this->branch_path = branch_path;

	this->inner_input_network_histories = vector<vector<FoldNetworkHistory*>>(branch_path->sequence_length);
	this->scope_histories = vector<ScopeHistory*>(branch_path->sequence_length, NULL);
	this->branch_histories = vector<BranchHistory*>(branch_path->sequence_length, NULL);
	this->fold_histories = vector<FoldHistory*>(branch_path->sequence_length, NULL);
	this->score_network_histories = vector<FoldNetworkHistory*>(branch_path->sequence_length, NULL);
	this->confidence_network_histories = vector<FoldNetworkHistory*>(branch_path->sequence_length, NULL);
	this->score_updates = vector<double>(branch_path->sequence_length, 0.0);
	this->confidence_network_outputs = vector<double>(branch_path->sequence_length, 0.0);
	this->compress_network_histories = vector<FoldNetworkHistory*>(branch_path->sequence_length, NULL);

	this->explore_fold_history = NULL;

	this->exit_index = branch_path->sequence_length-1;	// initialize to normal exit
	this->exit_location = EXIT_LOCATION_NORMAL;
}

BranchPathHistory::~BranchPathHistory() {
	for (int a_index = 0; a_index < (int)this->inner_input_network_histories.size(); a_index++) {
		for (int i_index = 0; i_index < (int)this->inner_input_network_histories[a_index].size(); i_index++) {
			delete this->inner_input_network_histories[a_index][i_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->scope_histories.size(); a_index++) {
		if (this->scope_histories[a_index] != NULL) {
			delete this->scope_histories[a_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->branch_histories.size(); a_index++) {
		if (this->branch_histories[a_index] != NULL) {
			delete this->branch_histories[a_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->fold_histories.size(); a_index++) {
		if (this->fold_histories[a_index] != NULL) {
			delete this->fold_histories[a_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->score_network_histories.size(); a_index++) {
		if (this->score_network_histories[a_index] != NULL) {
			delete this->score_network_histories[a_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->confidence_network_histories.size(); a_index++) {
		if (this->confidence_network_histories[a_index] != NULL) {
			delete this->confidence_network_histories[a_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->compress_network_histories.size(); a_index++) {
		if (this->compress_network_histories[a_index] != NULL) {
			delete this->compress_network_histories[a_index];
		}
	}

	if (this->explore_fold_history != NULL) {
		delete this->explore_fold_history;
	}
}
