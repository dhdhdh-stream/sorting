#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "globals.h"
#include "utilities.h"

using namespace std;

Scope::Scope(int num_inputs,
			 int num_outputs,
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
			 vector<double> average_scores,
			 vector<double> average_misguesses,
			 vector<double> average_inner_scope_impacts,
			 vector<double> average_local_impacts,
			 vector<double> average_inner_branch_impacts,
			 vector<bool> active_compress,
			 vector<int> compress_new_sizes,
			 vector<FoldNetwork*> compress_networks,
			 vector<int> compress_original_sizes,
			 bool full_last) {
	this->num_inputs = num_inputs;
	this->num_outputs = num_outputs;

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

	this->average_scores = average_scores;
	this->average_misguesses = average_misguesses;
	this->average_inner_scope_impacts = average_inner_scope_impacts;
	this->average_local_impacts = average_local_impacts;
	this->average_inner_branch_impacts = average_inner_branch_impacts;

	this->active_compress = active_compress;
	this->compress_new_sizes = compress_new_sizes;
	this->compress_networks = compress_networks;
	this->compress_original_sizes = compress_original_sizes;

	this->full_last = full_last;

	int state_size = 0;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		this->starting_state_sizes.push_back(state_size);

		if (!this->is_inner_scope[a_index]) {
			// obs_size always 1 for sorting
			state_size++;
		} else {
			state_size += this->scopes[a_index]->num_outputs;
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
	this->explore_index_inclusive = -1;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

Scope::Scope() {
	// do nothing -- will be initialized by load()
}

Scope::~Scope() {
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
		if (this->compress_networks[a_index] != NULL) {
			delete this->compress_networks[a_index];
		}
	}

	if (this->explore_fold != NULL) {
		delete this->explore_fold;
	}
}

void Scope::load(std::ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string num_inputs_line;
	getline(input_file, num_inputs_line);
	this->num_inputs = stoi(num_inputs_line);

	string num_outputs_line;
	getline(input_file, num_outputs_line);
	this->num_outputs = stoi(num_outputs_line);

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
				inner_input_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
				this->inner_input_networks[a_index].push_back(new FoldNetwork(inner_input_network_save_file));
				inner_input_network_save_file.close();

				string inner_input_size_line;
				getline(input_file, inner_input_size_line);
				this->inner_input_sizes[a_index].push_back(stoi(inner_input_size_line));
			}

			ifstream scope_scale_mod_save_file;
			scope_scale_mod_save_file.open("saves/nns/scope_" + to_string(this->id) + "_scope_scale_mod_" + to_string(a_index) + ".txt");
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

				this->active_compress.push_back(false);	// doesn't matter
				this->compress_new_sizes.push_back(-1);
				this->compress_networks.push_back(NULL);
				this->compress_original_sizes.push_back(-1);
			} else {
				ifstream score_network_save_file;
				score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
				this->score_networks.push_back(new FoldNetwork(score_network_save_file));
				score_network_save_file.close();

				string active_compress_line;
				getline(input_file, active_compress_line);
				this->active_compress.push_back(stoi(active_compress_line));

				string compress_new_size_line;
				getline(input_file, compress_new_size_line);
				this->compress_new_sizes.push_back(stoi(compress_new_size_line));

				if (this->active_compress[a_index]) {
					ifstream compress_network_save_file;
					compress_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_compress_" + to_string(a_index) + ".txt");
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
			score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
			this->score_networks.push_back(new FoldNetwork(score_network_save_file));
			score_network_save_file.close();

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
		string average_score_line;
		getline(input_file, average_score_line);
		this->average_scores.push_back(stof(average_score_line));

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

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		string starting_state_size_line;
		getline(input_file, starting_state_size_line);
		this->starting_state_sizes.push_back(stoi(starting_state_size_line));
	}

	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_index_inclusive = -1;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

void Scope::explore_on_path_activate(Problem& problem,
									 vector<double>& local_s_input_vals,	// i.e., input
									 vector<double>& local_state_vals,		// i.e., output
									 double& predicted_score,
									 double& scale_factor,
									 RunStatus& run_status,
									 ScopeHistory* history) {
	run_status.curr_depth++;
	if (run_status.curr_depth > solution->depth_limit) {
		run_status.exceeded_depth = true;
		history->exit_location = EXIT_LOCATION_SPOT;	// though doesn't matter, as won't backprop
		return;
	} else if (run_status.curr_depth > run_status.max_depth) {
		run_status.max_depth = run_status.curr_depth;
	}

	if (this->explore_type == EXPLORE_TYPE_NONE) {
		double sum_impact = 0.0;
		for (int a_index = 0; a_index < this->sequence_length; a_index++) {
			if (this->is_inner_scope[a_index]) {
				// sum_impact += this->average_inner_scope_impacts[a_index];
				sum_impact += 1.0;
			}

			if (this->step_types[a_index] == STEP_TYPE_STEP) {
				// sum_impact += this->average_local_impacts[a_index];
				sum_impact += 1.0;
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
						history->explore_type = EXPLORE_TYPE_INNER_SCOPE;
						history->explore_index_inclusive = a_index;
						history->explore_end_non_inclusive = -1;
					} else {
						history->explore_type = EXPLORE_TYPE_NEW;
						history->explore_index_inclusive = a_index;
						history->explore_end_non_inclusive = a_index + 1 + rand()%(this->sequence_length - a_index);

						bool new_can_be_empty;
						if (history->explore_end_non_inclusive > history->explore_index_inclusive+1) {
							new_can_be_empty = true;
						} else {
							new_can_be_empty = false;
						}

						int new_sequence_length;
						vector<bool> new_is_existing;
						vector<Scope*> new_existing_actions;
						vector<Action> new_actions;
						solution->new_sequence(new_sequence_length,
											   new_is_existing,
											   new_existing_actions,
											   new_actions,
											   new_can_be_empty);

						history->sequence_length = new_sequence_length;
						history->is_existing = new_is_existing;
						history->existing_actions = new_existing_actions;
						history->actions = new_actions;
					}
					break;
				}
			}

			if (this->step_types[a_index] == STEP_TYPE_STEP) {
				// rand_val -= this->average_local_impacts[a_index];
				rand_val -= 1.0;
				if (rand_val <= 0.0) {
					history->explore_type = EXPLORE_TYPE_NEW;
					history->explore_index_inclusive = a_index;
					history->explore_end_non_inclusive = a_index + 1 + rand()%(this->sequence_length - a_index);

					bool new_can_be_empty;
					if (history->explore_end_non_inclusive > history->explore_index_inclusive+1) {
						new_can_be_empty = true;
					} else {
						new_can_be_empty = false;
					}

					int new_sequence_length;
					vector<bool> new_is_existing;
					vector<Scope*> new_existing_actions;
					vector<Action> new_actions;
					solution->new_sequence(new_sequence_length,
										   new_is_existing,
										   new_existing_actions,
										   new_actions,
										   new_can_be_empty);

					history->sequence_length = new_sequence_length;
					history->is_existing = new_is_existing;
					history->existing_actions = new_existing_actions;
					history->actions = new_actions;

					break;
				}
			} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
				// rand_val -= this->average_inner_branch_impacts[a_index];
				rand_val -= 1.0;
				if (rand_val <= 0.0) {
					// Note: don't worry about explore after branch as there's either inner scope in-between, or an action performed first
					if (rand()%2 == 0) {
						history->explore_type = EXPLORE_TYPE_INNER_BRANCH;
						history->explore_index_inclusive = a_index;
						history->explore_end_non_inclusive = -1;
					} else {
						history->explore_type = EXPLORE_TYPE_NEW;
						history->explore_index_inclusive = a_index;
						history->explore_end_non_inclusive = a_index + 1 + rand()%(this->sequence_length - a_index);

						bool new_can_be_empty;
						if (history->explore_end_non_inclusive > history->explore_index_inclusive+1) {
							new_can_be_empty = true;
						} else {
							new_can_be_empty = false;
						}

						int new_sequence_length;
						vector<bool> new_is_existing;
						vector<Scope*> new_existing_actions;
						vector<Action> new_actions;
						solution->new_sequence(new_sequence_length,
											   new_is_existing,
											   new_existing_actions,
											   new_actions,
											   new_can_be_empty);

						history->sequence_length = new_sequence_length;
						history->is_existing = new_is_existing;
						history->existing_actions = new_existing_actions;
						history->actions = new_actions;
					}
					break;
				}
			} else {
				// this->step_types[a_index] == STEP_TYPE_FOLD
				// do nothing
			}
		}
	}

	int a_index = 0;
	while (a_index < this->sequence_length) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			vector<double> scope_input;
			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				scope_input = vector<double>(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);
			} else {
				// scopes formed through folding may have multiple inner_input_networks
				// reused scopes will have 1 inner_input_network
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
			}

			double scope_scale_mod_val = this->scope_scale_mod[a_index]->output->constants[0];
			scale_factor *= scope_scale_mod_val;

			vector<double> scope_output;
			ScopeHistory* scope_history = new ScopeHistory(this->scopes[a_index]);
			if ((this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_SCOPE)
					|| (this->explore_type == EXPLORE_TYPE_NONE
						&& history->explore_type == EXPLORE_TYPE_INNER_SCOPE
						&& history->explore_index_inclusive == a_index)) {
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

			// return after scale_factor restored
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
				// scope end -- do nothing
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
					// explore_phase != EXPLORE_PHASE_FLAT, so don't need to delete score_network_history

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
				} else if (this->explore_type == EXPLORE_TYPE_NONE
						&& history->explore_type == EXPLORE_TYPE_NEW
						&& history->explore_index_inclusive == a_index) {
					// explore_phase != EXPLORE_PHASE_FLAT, so don't need to delete score_network_history

					run_status.existing_score = existing_score;

					for (int n_index = 0; n_index < history->sequence_length; n_index++) {
						if (!history->is_existing[n_index]) {
							problem.perform_action(history->actions[n_index]);
						} else {
							vector<double> scope_input(history->existing_actions[n_index]->num_inputs, 0.0);
							vector<double> scope_output;	// unused
							ScopeHistory* scope_history = new ScopeHistory(history->existing_actions[n_index]);
							history->existing_actions[n_index]->existing_update_activate(
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
								history->exit_location = EXIT_LOCATION_BACK;
								// though setting doesn't matter as won't backprop
								return;
							}
						}
					}

					run_status.explore_phase = EXPLORE_PHASE_EXPLORE;

					if (history->explore_end_non_inclusive == this->sequence_length) {
						local_state_vals = vector<double>(this->num_outputs, 0.0);
					} else {
						local_state_vals = vector<double>(this->starting_state_sizes[history->explore_end_non_inclusive], 0.0);
					}
					a_index = history->explore_end_non_inclusive-1;		// account for increment at end
				} else {
					history->score_network_histories[a_index] = score_network_history;
					history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
					predicted_score += existing_score;

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
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			BranchHistory* branch_history = new BranchHistory(this->branches[a_index]);
			if ((this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH)
					|| (this->explore_type == EXPLORE_TYPE_NONE
						&& history->explore_type == EXPLORE_TYPE_INNER_BRANCH
						&& history->explore_index_inclusive == a_index)) {
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
				double existing_score = branch_history->best_score;
				delete branch_history;

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
			} else if (this->explore_type == EXPLORE_TYPE_NONE
					&& history->explore_type == EXPLORE_TYPE_NEW
					&& history->explore_index_inclusive == a_index) {
				run_status.existing_score = branch_history->best_score;
				delete branch_history;

				for (int n_index = 0; n_index < history->sequence_length; n_index++) {
					if (!history->is_existing[n_index]) {
						problem.perform_action(history->actions[n_index]);
					} else {
						vector<double> scope_input(history->existing_actions[n_index]->num_inputs, 0.0);
						vector<double> scope_output;	// unused
						ScopeHistory* scope_history = new ScopeHistory(history->existing_actions[n_index]);
						history->existing_actions[n_index]->existing_update_activate(
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
							history->exit_location = EXIT_LOCATION_BACK;
							// though setting doesn't matter as won't backprop
							return;
						}
					}
				}

				run_status.explore_phase = EXPLORE_PHASE_EXPLORE;

				if (history->explore_end_non_inclusive == this->sequence_length) {
					local_state_vals = vector<double>(this->num_outputs, 0.0);
				} else {
					local_state_vals = vector<double>(this->starting_state_sizes[history->explore_end_non_inclusive], 0.0);
				}
				a_index = history->explore_end_non_inclusive-1;		// account for increment at end
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

void Scope::explore_off_path_activate(Problem& problem,
									  vector<double>& local_s_input_vals,	// i.e., input
									  vector<double>& local_state_vals,		// i.e., output
									  double& predicted_score,
									  double& scale_factor,
									  RunStatus& run_status,
									  ScopeHistory* history) {
	run_status.curr_depth++;
	if (run_status.curr_depth > solution->depth_limit) {
		run_status.exceeded_depth = true;
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	} else if (run_status.curr_depth > run_status.max_depth) {
		run_status.max_depth = run_status.curr_depth;
	}

	// temp
	history->starting_explore_phase = run_status.explore_phase;

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			vector<double> scope_input;
			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				scope_input = vector<double>(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);
			} else {
				// scopes formed through folding may have multiple inner_input_networks
				// reused scopes will have 1 inner_input_network
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

			// return after scale_factor restored
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
				// scope end -- do nothing
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

void Scope::explore_on_path_backprop(vector<double>& local_state_errors,	// i.e., input_errors
									 double& predicted_score,
									 double target_val,
									 double& scale_factor,
									 ScopeHistory* history) {
	// history->exit_location != EXIT_LOCATION_SPOT

	// don't need to output local_s_input_errors on path but for explore_off_path_backprop
	vector<double> local_s_input_errors(this->num_inputs, 0.0);

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

	while (a_index >= 0) {
		if (this->explore_index_inclusive == a_index
				&& this->explore_type == EXPLORE_TYPE_NEW) {
			int explore_signal = this->explore_fold->explore_on_path_backprop(
				local_state_errors,
				predicted_score,
				target_val,
				scale_factor,
				history->explore_fold_history);

			if (explore_signal == EXPLORE_SIGNAL_REPLACE) {
				explore_replace();
			} else if (explore_signal == EXPLORE_SIGNAL_BRANCH) {
				explore_branch();
			} else if (explore_signal == EXPLORE_SIGNAL_CLEAN) {
				this->explore_type = EXPLORE_TYPE_NONE;
				this->explore_index_inclusive = -1;
				this->explore_end_non_inclusive = -1;
				delete this->explore_fold;
				this->explore_fold = NULL;
			}

			return;
		} else {
			if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
				// do nothing
			} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
				if (a_index == this->sequence_length-1 && !this->full_last) {
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

					predicted_score -= scale_factor*history->score_updates[a_index];
				}
			} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
				// if early exit, then local_state_errors will be initialized within
				if (this->explore_index_inclusive == a_index
						&& this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
					this->branches[a_index]->explore_on_path_backprop(local_s_input_errors,
																	  local_state_errors,
																	  predicted_score,
																	  target_val,
																	  scale_factor,
																	  history->branch_histories[a_index]);

					this->explore_count++;
					if (this->branches[a_index]->explore_ref_count == 0) {
						this->explore_type = EXPLORE_TYPE_NONE;
						this->explore_index_inclusive = -1;
						this->explore_end_non_inclusive = -1;
					}

					return;
				} else {
					this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
																	   local_state_errors,
																	   predicted_score,
																	   target_val,
																	   scale_factor,
																	   history->branch_histories[a_index]);
				}
			} else {
				// this->step_types[a_index] == STEP_TYPE_FOLD
				// if early exit, then local_state_errors will be initialized within
				this->folds[a_index]->explore_off_path_backprop(local_s_input_errors,
																local_state_errors,
																predicted_score,
																target_val,
																scale_factor,
																history->fold_histories[a_index]);

				// predicted_score already modified to before fold value in fold
				double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
				double score_predicted_score_error = target_val - score_predicted_score;

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
				// on_path doesn't need scope_output_errors

				this->scopes[a_index]->explore_on_path_backprop(scope_input_errors,
																predicted_score,
																target_val,
																scale_factor,
																history->scope_histories[a_index]);

				this->explore_count++;
				if (this->scopes[a_index]->explore_type == EXPLORE_TYPE_NONE) {
					this->explore_type = EXPLORE_TYPE_NONE;
					this->explore_index_inclusive = -1;
					this->explore_end_non_inclusive = -1;
				}

				return;
			} else {
				vector<double> scope_output_errors;	// i.e., temp_new_s_input_errors
				this->scopes[a_index]->explore_off_path_backprop(scope_input_errors,
																 scope_output_errors,
																 predicted_score,
																 target_val,
																 scale_factor,
																 history->scope_histories[a_index]);

				scale_factor /= scope_scale_mod_val;

				// a_index != 0
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
}

void Scope::explore_off_path_backprop(vector<double>& local_state_errors,	// i.e., input_errors
									  vector<double>& local_s_input_errors,	// i.e., output_errors
									  double& predicted_score,
									  double target_val,
									  double& scale_factor,
									  ScopeHistory* history) {
	local_s_input_errors = vector<double>(this->num_inputs, 0.0);

	if (history->exit_location == EXIT_LOCATION_SPOT) {
		return;
	}

	for (int a_index = history->exit_index; a_index >= 0; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
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

				if (history->score_network_histories[a_index] == NULL) {
					cout << "HERE" << endl;
					cout << "history->exit_index: " << history->exit_index << endl;
					cout << "history->exit_location: " << history->exit_location << endl;
					cout << "this->sequence_length: " << this->sequence_length << endl;
					cout << "this->score_networks[a_index]: " << this->score_networks[a_index] << endl;
					cout << "a_index: " << a_index << endl;
					if (this->full_last) {
						cout << "is full last" << endl;
					} else {
						cout << "not full last" << endl;
					}
					cout << "history->starting_explore_phase: " << history->starting_explore_phase << endl;
					cout << "this->explore_type: " << this->explore_type << endl;
					cout << "this->explore_index_inclusive: " << this->explore_index_inclusive << endl;
				}

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
			// if early exit, then local_state_errors will be initialized within
			this->branches[a_index]->explore_off_path_backprop(local_s_input_errors,
															   local_state_errors,
															   predicted_score,
															   target_val,
															   scale_factor,
															   history->branch_histories[a_index]);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			// if early exit, then local_state_errors will be initialized within
			this->folds[a_index]->explore_off_path_backprop(local_s_input_errors,
															local_state_errors,
															predicted_score,
															target_val,
															scale_factor,
															history->fold_histories[a_index]);

			// predicted_score already modified to before fold value in fold
			double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
			double score_predicted_score_error = target_val - score_predicted_score;

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

			if (history->scope_histories[a_index]->score_network_histories[0] == NULL) {
				cout << "HERE HERE" << endl;
				cout << "history->exit_index: " << history->exit_index << endl;
				cout << "history->exit_location: " << history->exit_location << endl;
				cout << "a_index: " << a_index << endl;
				cout << "this->step_types[a_index]: " << this->step_types[a_index] << endl;
				cout << "history->score_network_histories[a_index]: " << history->score_network_histories[a_index] << endl;
				cout << "history->starting_explore_phase: " << history->starting_explore_phase << endl;
				cout << "this->explore_type: " << this->explore_type << endl;
				cout << "this->explore_index_inclusive: " << this->explore_index_inclusive << endl;
			}

			// TODO: when reusing scopes, they can have previous explore indexes set, need to rethink explore rules

			vector<double> scope_output_errors;
			this->scopes[a_index]->explore_off_path_backprop(scope_input_errors,
															 scope_output_errors,
															 predicted_score,
															 target_val,
															 scale_factor,
															 history->scope_histories[a_index]);

			scale_factor /= scope_scale_mod_val;

			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				for (int i_index = 0; i_index < this->scopes[0]->num_inputs; i_index++) {
					local_s_input_errors[i_index] += scope_output_errors[i_index];
				}
			} else {
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
	}
}

void Scope::existing_flat_activate(Problem& problem,
								   vector<double>& local_s_input_vals,	// i.e., input
								   vector<double>& local_state_vals,	// i.e., output
								   double& predicted_score,
								   double& scale_factor,
								   RunStatus& run_status,
								   ScopeHistory* history) {
	run_status.curr_depth++;
	if (run_status.curr_depth > solution->depth_limit) {
		run_status.exceeded_depth = true;
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	} else if (run_status.curr_depth > run_status.max_depth) {
		run_status.max_depth = run_status.curr_depth;
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			vector<double> scope_input;
			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				scope_input = vector<double>(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);
			} else {
				// scopes formed through folding may have multiple inner_input_networks
				// reused scopes will have 1 inner_input_network
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

			// return after scale_factor restored
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

void Scope::existing_flat_backprop(vector<double>& local_state_errors,		// input_errors
								   vector<double>& local_s_input_errors,	// output_errors
								   double& predicted_score,
								   double predicted_score_error,
								   double& scale_factor,
								   double& scale_factor_error,
								   ScopeHistory* history) {
	local_s_input_errors = vector<double>(this->num_inputs, 0.0);

	if (history->exit_location == EXIT_LOCATION_SPOT) {
		return;
	}

	for (int a_index = history->exit_index; a_index >= 0; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
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

			vector<double> scope_output_errors;
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

			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				for (int i_index = 0; i_index < this->scopes[0]->num_inputs; i_index++) {
					local_s_input_errors[i_index] += scope_output_errors[i_index];
				}
			} else {
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
	}
}

void Scope::update_activate(Problem& problem,
							vector<double>& local_s_input_vals,	// i.e., input
							vector<double>& local_state_vals,	// i.e., output
							double& predicted_score,
							double& scale_factor,
							RunStatus& run_status,
							ScopeHistory* history) {
	run_status.curr_depth++;
	if (run_status.curr_depth > solution->depth_limit) {
		run_status.exceeded_depth = true;
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	} else if (run_status.curr_depth > run_status.max_depth) {
		run_status.max_depth = run_status.curr_depth;
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			vector<double> scope_input;
			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				scope_input = vector<double>(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);
			} else {
				// scopes formed through folding may have multiple inner_input_networks
				// reused scopes will have 1 inner_input_network
				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
					this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																				 local_state_vals);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
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

			// return after scale_factor restored
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

void Scope::update_backprop(double& predicted_score,
							double& next_predicted_score,
							double target_val,
							double& scale_factor,
							double& scale_factor_error,
							ScopeHistory* history) {
	if (history->exit_location == EXIT_LOCATION_SPOT) {
		return;
	}

	for (int a_index = history->exit_index; a_index >= 0; a_index--) {
		// update misguess even after early exit as goal is predict it correctly too
		if (a_index == this->sequence_length-1) {
			double misguess = (target_val - next_predicted_score)*(target_val - next_predicted_score);
			this->average_misguesses[a_index] = 0.999*this->average_misguesses[a_index] + 0.001*misguess;
		} else {
			double misguess = (target_val - predicted_score)*(target_val - predicted_score);
			this->average_misguesses[a_index] = 0.999*this->average_misguesses[a_index] + 0.001*misguess;
		}

		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end -- do nothing
			} else {
				double predicted_score_error = target_val - predicted_score;

				scale_factor_error += history->score_updates[a_index]*predicted_score_error;

				vector<double> score_errors{scale_factor*predicted_score_error};
				this->score_networks[a_index]->backprop_small_weights_with_no_error_signal(
					score_errors,
					0.001,
					history->score_network_histories[a_index]);

				this->average_scores[a_index] = 0.999*this->average_scores[a_index] + 0.001*predicted_score;

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
													 scale_factor_error,
													 history->branch_histories[a_index]);

			this->average_scores[a_index] = 0.999*this->average_scores[a_index] + 0.001*next_predicted_score;

			double starting_predicted_score = predicted_score;
			this->average_inner_branch_impacts[a_index] = 0.999*this->average_inner_branch_impacts[a_index]
				+ 0.001*abs(ending_predicted_score - starting_predicted_score);
		} else {
			// this->step_types[a_index] == STEP_TYPE_FOLD
			this->folds[a_index]->update_backprop(predicted_score,
												  next_predicted_score,
												  target_val,
												  scale_factor,
												  scale_factor_error,
												  history->fold_histories[a_index]);

			// predicted_score already modified to before fold value in fold
			double score_predicted_score = predicted_score + scale_factor*history->score_updates[a_index];
			double score_predicted_score_error = target_val - score_predicted_score;

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
												   next_predicted_score,
												   target_val,
												   scale_factor,
												   scope_scale_factor_error,
												   history->scope_histories[a_index]);

			// update mod even with early exit as might improve predictions/results
			vector<double> mod_errors{scope_scale_factor_error};
			this->scope_scale_mod[a_index]->backprop(mod_errors, 0.0002);

			scale_factor_error += scope_scale_mod_val*scope_scale_factor_error;

			scale_factor /= scope_scale_mod_val;

			double starting_predicted_score = predicted_score;
			this->average_inner_scope_impacts[a_index] = 0.999*this->average_inner_scope_impacts[a_index]
				+ 0.001*abs(ending_predicted_score - starting_predicted_score);
		}
	}
}

void Scope::existing_update_activate(Problem& problem,
									 vector<double>& local_s_input_vals,	// i.e., input
									 vector<double>& local_state_vals,	// i.e., output
									 double& predicted_score,
									 double& scale_factor,
									 RunStatus& run_status,
									 ScopeHistory* history) {
	run_status.curr_depth++;
	if (run_status.curr_depth > solution->depth_limit) {
		run_status.exceeded_depth = true;
		history->exit_location = EXIT_LOCATION_SPOT;
		return;
	} else if (run_status.curr_depth > run_status.max_depth) {
		run_status.max_depth = run_status.curr_depth;
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (!this->is_inner_scope[a_index]) {
			problem.perform_action(this->actions[a_index]);
			local_state_vals.push_back(problem.get_observation());
		} else {
			vector<double> scope_input;
			if (a_index == 0) {
				// for start, if inner scope, scope_input is first local_s_input_vals
				scope_input = vector<double>(local_s_input_vals.begin(), local_s_input_vals.begin()+this->scopes[0]->num_inputs);
			} else {
				// scopes formed through folding may have multiple inner_input_networks
				// reused scopes will have 1 inner_input_network
				for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
					this->inner_input_networks[a_index][i_index]->activate_small(local_s_input_vals,
																				 local_state_vals);
					for (int s_index = 0; s_index < this->inner_input_sizes[a_index][i_index]; s_index++) {
						scope_input.push_back(this->inner_input_networks[a_index][i_index]->output->acti_vals[s_index]);
					}
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

			// return after scale_factor restored
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
				// scope end -- do nothing
			} else {
				this->score_networks[a_index]->activate_small(local_s_input_vals,
															  local_state_vals);
				history->score_updates[a_index] = this->score_networks[a_index]->output->acti_vals[0];
				predicted_score += scale_factor*this->score_networks[a_index]->output->acti_vals[0];

				if (this->active_compress[a_index]) {
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

void Scope::existing_update_backprop(double& predicted_score,
									 double predicted_score_error,
									 double& scale_factor,
									 double& scale_factor_error,
									 ScopeHistory* history) {
	if (history->exit_location == EXIT_LOCATION_SPOT) {
		return;
	}

	for (int a_index = history->exit_index; a_index >= 0; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->sequence_length-1 && !this->full_last) {
				// scope end -- do nothing
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
}

void Scope::explore_set(ScopeHistory* history) {
	if (this->explore_type == EXPLORE_TYPE_NONE) {
		if (history->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
			this->scopes[history->explore_index_inclusive]->explore_set(
				history->scope_histories[history->explore_index_inclusive]);
			
			this->explore_type = EXPLORE_TYPE_INNER_SCOPE;
			this->explore_index_inclusive = history->explore_index_inclusive;
			this->explore_end_non_inclusive = -1;
			this->explore_count = 0;
		} else if (history->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
			this->branches[history->explore_index_inclusive]->explore_set(
				history->branch_histories[history->explore_index_inclusive]);

			this->explore_type = EXPLORE_TYPE_INNER_BRANCH;
			this->explore_index_inclusive = history->explore_index_inclusive;
			this->explore_end_non_inclusive = -1;
			this->explore_count = 0;
		} else {
			// history->explore_type == EXPLORE_TYPE_NEW
			this->explore_type = EXPLORE_TYPE_NEW;
			this->explore_index_inclusive = history->explore_index_inclusive;
			this->explore_end_non_inclusive = history->explore_end_non_inclusive;

			int new_num_inputs = this->starting_state_sizes[history->explore_index_inclusive];
			if (!this->is_inner_scope[history->explore_index_inclusive]) {
				// obs_size always 1 for sorting
				new_num_inputs++;
			} else {
				new_num_inputs += this->scopes[history->explore_index_inclusive]->num_outputs;
			}
			int new_num_outputs;
			if (history->explore_end_non_inclusive == this->sequence_length) {
				new_num_outputs = this->num_outputs;
			} else {
				new_num_outputs = this->starting_state_sizes[history->explore_end_non_inclusive];
			}

			this->explore_fold = new Fold(new_num_inputs,
										  new_num_outputs,
										  this->num_inputs,
										  history->sequence_length,
										  history->is_existing,
										  history->existing_actions,
										  history->actions,
										  &this->average_scores[history->explore_index_inclusive],
										  &this->average_misguesses[history->explore_end_non_inclusive-1]);

			for (int s_index = 0; s_index < history->sequence_length; s_index++) {
				if (!history->is_existing[s_index]) {
					solution->action_dictionary.push_back(history->actions[s_index]);
				} else {
					solution->scope_use_counts[history->existing_actions[s_index]->id]++;
					solution->scope_use_sum_count++;
				}
			}
		}
	} else if (this->explore_type == EXPLORE_TYPE_INNER_SCOPE) {
		this->scopes[this->explore_index_inclusive]->explore_set(
			history->scope_histories[this->explore_index_inclusive]);
	} else if (this->explore_type == EXPLORE_TYPE_INNER_BRANCH) {
		this->branches[this->explore_index_inclusive]->explore_set(
			history->branch_histories[this->explore_index_inclusive]);
	}
	// this->explore_type != EXPLORE_TYPE_NEW
}

// may miss updates due to structural updates, but should not be significant
void Scope::update_increment(ScopeHistory* history,
							 vector<Fold*>& folds_to_delete) {
	if (history->exit_location == EXIT_LOCATION_SPOT) {
		return;
	}

	for (int a_index = history->exit_index; a_index >= 0; a_index--) {
		if (a_index == history->exit_index && history->exit_location == EXIT_LOCATION_FRONT) {
			// do nothing
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			// check for equality in case previous update_increment updated structure
			if (history->branch_histories[a_index]->branch == this->branches[a_index]) {
				this->branches[a_index]->update_increment(history->branch_histories[a_index],
														  folds_to_delete);
			}
		} else if (this->step_types[a_index] == STEP_TYPE_FOLD) {
			if (history->fold_histories[a_index]->fold == this->folds[a_index]) {
				this->folds[a_index]->update_increment(history->fold_histories[a_index],
													   folds_to_delete);
			}

			// re-check because might have updated
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
}

void Scope::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->num_inputs << endl;
	output_file << this->num_outputs << endl;

	output_file << this->full_last << endl;

	output_file << this->sequence_length << endl;
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->is_inner_scope[a_index] << endl;
		if (this->is_inner_scope[a_index]) {
			output_file << this->scopes[a_index]->id << endl;

			output_file << this->inner_input_networks[a_index].size() << endl;
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				ofstream inner_input_network_save_file;
				inner_input_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_inner_input_" + to_string(a_index) + "_" + to_string(i_index) + ".txt");
				this->inner_input_networks[a_index][i_index]->save(inner_input_network_save_file);
				inner_input_network_save_file.close();

				output_file << this->inner_input_sizes[a_index][i_index] << endl;
			}

			ofstream scope_scale_mod_save_file;
			scope_scale_mod_save_file.open("saves/nns/scope_" + to_string(this->id) + "_scope_scale_mod_" + to_string(a_index) + ".txt");
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
				ofstream score_network_save_file;
				score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
				this->score_networks[a_index]->save(score_network_save_file);
				score_network_save_file.close();

				output_file << this->active_compress[a_index] << endl;
				output_file << this->compress_new_sizes[a_index] << endl;
				if (this->active_compress[a_index]) {
					ofstream compress_network_save_file;
					compress_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_compress_" + to_string(a_index) + ".txt");
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
			ofstream score_network_save_file;
			score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_score_" + to_string(a_index) + ".txt");
			this->score_networks[a_index]->save(score_network_save_file);
			score_network_save_file.close();

			output_file << this->folds[a_index]->id << endl;

			ofstream fold_save_file;
			fold_save_file.open("saves/fold_" + to_string(this->folds[a_index]->id) + ".txt");
			this->folds[a_index]->save(fold_save_file);
			fold_save_file.close();
		}
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->average_scores[a_index] << endl;
		output_file << this->average_misguesses[a_index] << endl;
		output_file << this->average_inner_scope_impacts[a_index] << endl;
		output_file << this->average_local_impacts[a_index] << endl;
		output_file << this->average_inner_branch_impacts[a_index] << endl;
	}

	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		output_file << this->starting_state_sizes[a_index] << endl;
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->inner_input_network_histories = vector<vector<FoldNetworkHistory*>>(scope->sequence_length);
	this->scope_histories = vector<ScopeHistory*>(scope->sequence_length, NULL);
	this->branch_histories = vector<BranchHistory*>(scope->sequence_length, NULL);
	this->fold_histories = vector<FoldHistory*>(scope->sequence_length, NULL);
	this->score_network_histories = vector<FoldNetworkHistory*>(scope->sequence_length, NULL);
	this->score_updates = vector<double>(scope->sequence_length, 0.0);
	this->compress_network_histories = vector<FoldNetworkHistory*>(scope->sequence_length, NULL);

	this->explore_fold_history = NULL;

	this->exit_index = scope->sequence_length-1;	// initialize to normal exit
	this->exit_location = EXIT_LOCATION_NORMAL;

	// temp
	this->starting_explore_phase = -10;
}

ScopeHistory::~ScopeHistory() {
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

	for (int a_index = 0; a_index < (int)this->compress_network_histories.size(); a_index++) {
		if (this->compress_network_histories[a_index] != NULL) {
			delete this->compress_network_histories[a_index];
		}
	}

	if (this->explore_fold_history != NULL) {
		delete this->explore_fold_history;
	}
}
