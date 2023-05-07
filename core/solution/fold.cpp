#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

Fold::Fold(vector<int> scope_context,
		   vector<int> node_context,
		   int exit_depth,
		   vector<bool> is_inner_scope,
		   vector<int> existing_scope_ids,
		   vector<Action> actions,
		   double* existing_average_score,
		   double* existing_score_variance,
		   double* existing_average_misguess,
		   double* existing_misguess_variance,
		   double seed_start_predicted_score,
		   double seed_start_scale_factor,
		   vector<double> seed_state_vals_snapshot,
		   ScopeHistory* seed_outer_context_history,
		   double seed_target_val) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->exit_depth = exit_depth;

	this->sequence_length = (int)is_inner_scope.size();
	this->is_inner_scope = is_inner_scope;
	this->existing_scope_ids = existing_scope_ids;
	this->actions = actions;

	this->existing_average_score = existing_average_score;
	this->existing_score_variance = existing_score_variance;
	this->existing_average_misguess = existing_average_misguess;
	this->existing_misguess_variance = existing_misguess_variance;

	this->seed_start_predicted_score = seed_start_predicted_score;
	this->seed_start_scale_factor = seed_start_scale_factor;
	this->seed_state_vals_snapshot = seed_state_vals_snapshot;
	this->seed_outer_context_history = seed_outer_context_history;
	this->seed_target_val = seed_target_val;

	Scope* score_scope = solution->scopes[this->scope_context[0]];
	this->num_score_states = score_scope->num_states;
	Scope* sequence_scope = solution->scopes[this->scope_context[this->exit_depth]];
	this->num_sequence_states = sequence_scope->num_states;

	this->sum_inner_inputs = 0;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			this->inner_input_start_indexes.push_back(this->sum_inner_inputs);
			this->num_inner_inputs.push_back(inner_scope->num_states);
			this->sum_inner_inputs += inner_scope->num_states;
			this->inner_scope_scale_mods.push_back(new Scale(1.0));
		} else {
			this->inner_input_start_indexes.push_back(-1);
			this->num_inner_inputs.push_back(-1);
			this->inner_scope_scale_mods.push_back(NULL);
		}
	}
	this->end_mod = 0.0;

	this->curr_num_new_outer_states = 1;
	this->test_num_new_outer_states = this->curr_num_new_outer_states;
	// test_outer_state_networks starts empty
	this->curr_starting_score_network = NULL;
	this->test_starting_score_network = new StateNetwork(0,
														 this->num_score_states,
														 0,
														 this->test_num_new_outer_states,
														 20);

	this->curr_num_new_inner_states = 1;
	this->test_num_new_inner_states = this->curr_num_new_inner_states;
	int num_inner_networks = this->sum_inner_inputs
		+ this->test_num_new_inner_states
		+ this->num_sequence_states;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->test_state_networks.push_back(vector<StateNetwork*>());
		if (this->is_inner_scope[f_index]) {
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(0,
																			  this->num_sequence_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->test_num_new_outer_states,
																			  20));
			}
		} else {
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(1,
																			  this->num_sequence_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->test_num_new_outer_states,
																			  20));
			}
		}

		this->test_score_networks.push_back(new StateNetwork(0,
															 this->num_sequence_states,
															 this->sum_inner_inputs+this->test_num_new_inner_states,
															 this->test_num_new_outer_states,
															 20));
	}

	this->test_branch_average_score = 0.0;
	this->test_existing_average_improvement = 0.0;
	this->test_replace_average_score = 0.0;
	this->test_replace_average_misguess = 0.0;
	this->test_replace_misguess_variance = 0.0;

	this->is_recursive = 0;

	// initialize for saving/loading
	this->clean_outer_scope_index = -1;
	this->clean_outer_node_index = -1;
	this->clean_outer_state_index = -1;
	this->clean_inner_step_index = -1;
	this->clean_inner_state_index = -1;

	this->state = FOLD_STATE_EXPERIMENT;
	this->state_iter = -1;
	this->sum_error = 0.0;
}

Fold::Fold(ifstream& input_file,
		   int scope_id,
		   int scope_index) {
	string context_size_line;
	getline(input_file, context_size_line);
	int context_size = stoi(context_size_line);
	for (int c_index = 0; c_index < context_size; c_index++) {
		string scope_context_line;
		getline(input_file, scope_context_line);
		this->scope_context.push_back(stoi(scope_context_line));

		string node_context_line;
		getline(input_file, node_context_line);
		this->node_context.push_back(stoi(node_context_line));
	}

	string exit_depth_line;
	getline(input_file, exit_depth_line);
	this->exit_depth = stoi(exit_depth_line);

	string sequence_length_line;
	getline(input_file, sequence_length_line);
	this->sequence_length = stoi(sequence_length_line);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		string is_inner_scope_line;
		getline(input_file, is_inner_scope_line);
		this->is_inner_scope.push_back(stoi(is_inner_scope_line));

		if (this->is_inner_scope[f_index]) {
			string existing_scope_id_line;
			getline(input_file, existing_scope_id_line);
			this->existing_scope_ids.push_back(stoi(existing_scope_id_line));

			string scope_scale_mod_weight_line;
			getline(input_file, scope_scale_mod_weight_line);
			this->inner_scope_scale_mods.push_back(new Scale(stof(scope_scale_mod_weight_line)));

			this->actions.push_back(Action());
		} else {
			this->existing_scope_ids.push_back(-1);
			this->inner_scope_scale_mods.push_back(NULL);

			this->actions.push_back(Action(input_file));
		}
	}

	string num_score_states_line;
	getline(input_file, num_score_states_line);
	this->num_score_states = stoi(num_score_states_line);

	string num_sequence_states_line;
	getline(input_file, num_sequence_states_line);
	this->num_sequence_states = stoi(num_sequence_states_line);

	this->sum_inner_inputs = 0;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			string num_inner_input_line;
			getline(input_file, num_inner_input_line);
			this->num_inner_inputs.push_back(stoi(num_inner_input_line));

			this->inner_input_start_indexes.push_back(this->sum_inner_inputs);
			this->sum_inner_inputs += this->num_inner_inputs.back();
		} else {
			this->inner_input_start_indexes.push_back(-1);
			this->num_inner_inputs.push_back(-1);
		}
	}

	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	string num_new_outer_states_line;
	getline(input_file, num_new_outer_states_line);
	this->curr_num_new_outer_states = stoi(num_new_outer_states_line);

	string outer_state_networks_not_needed_size_line;
	getline(input_file, outer_state_networks_not_needed_size_line);
	int outer_state_networks_not_needed_size = stoi(outer_state_networks_not_needed_size_line);
	for (int s_index = 0; s_index < outer_state_networks_not_needed_size; s_index++) {
		string outer_scope_id_line;
		getline(input_file, outer_scope_id_line);
		int outer_scope_id = stoi(outer_scope_id_line);

		this->curr_outer_state_networks_not_needed.insert({outer_scope_id, vector<vector<bool>>()});

		string num_nodes_line;
		getline(input_file, num_nodes_line);
		int num_nodes = stoi(num_nodes_line);
		for (int n_index = 0; n_index < num_nodes; n_index++) {
			this->curr_outer_state_networks_not_needed[outer_scope_id].push_back(vector<bool>());

			string num_states_line;
			getline(input_file, num_states_line);
			int num_states = stoi(num_states_line);
			for (int s_index = 0; s_index < num_states; s_index++) {
				string is_not_needed_line;
				getline(input_file, is_not_needed_line);
				this->curr_outer_state_networks_not_needed[outer_scope_id].back().push_back(stoi(is_not_needed_line));
			}
		}
	}

	string outer_state_networks_size_line;
	getline(input_file, outer_state_networks_size_line);
	int outer_state_networks_size = stoi(outer_state_networks_size_line);
	for (int s_index = 0; s_index < outer_state_networks_size; s_index++) {
		string outer_scope_id_line;
		getline(input_file, outer_scope_id_line);
		int outer_scope_id = stoi(outer_scope_id_line);

		this->curr_outer_state_networks.insert({outer_scope_id, vector<vector<StateNetwork*>>()});

		string num_nodes_line;
		getline(input_file, num_nodes_line);
		int num_nodes = stoi(num_nodes_line);
		for (int n_index = 0; n_index < num_nodes; n_index++) {
			this->curr_outer_state_networks[outer_scope_id].push_back(vector<StateNetwork*>());

			string num_states_line;
			getline(input_file, num_states_line);
			int num_states = stoi(num_states_line);
			for (int s_index = 0; s_index < num_states; s_index++) {
				if (this->curr_outer_state_networks_not_needed.size() > 0	// may not be initialized
						&& this->curr_outer_state_networks_not_needed[outer_scope_id][n_index][s_index]) {
					this->curr_outer_state_networks[outer_scope_id].back().push_back(NULL);
				} else {
					ifstream outer_state_network_save_file;
					outer_state_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_outer_state_" + to_string(outer_scope_id) + "_" + to_string(n_index) + "_" + to_string(s_index) + ".txt");
					this->curr_outer_state_networks[outer_scope_id].back().push_back(new StateNetwork(outer_state_network_save_file));
					outer_state_network_save_file.close();
				}
			}
		}
	}

	ifstream starting_score_network_save_file;
	starting_score_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_starting_score.txt");
	this->curr_starting_score_network = new StateNetwork(starting_score_network_save_file);
	starting_score_network_save_file.close();

	for (int i_index = 0; i_index < this->sum_inner_inputs; i_index++) {
		string needed_line;
		getline(input_file, needed_line);
		this->curr_inner_inputs_needed.push_back(stoi(needed_line));
	}

	string clean_outer_scope_index_line;
	getline(input_file, clean_outer_scope_index_line);
	this->clean_outer_scope_index = stoi(clean_outer_scope_index_line);

	string outer_scopes_needed_size_line;
	getline(input_file, outer_scopes_needed_size_line);
	int outer_scopes_needed_size = stoi(outer_scopes_needed_size_line);
	for (int s_index = 0; s_index < outer_scopes_needed_size; s_index++) {
		string outer_scope_id_line;
		getline(input_file, outer_scope_id_line);
		this->curr_outer_scopes_needed.insert(stoi(outer_scope_id_line));
	}

	string outer_contexts_needed_size_line;
	getline(input_file, outer_contexts_needed_size_line);
	int outer_contexts_needed_size = stoi(outer_contexts_needed_size_line);
	for (int c_index = 0; c_index < outer_contexts_needed_size; c_index++) {
		string outer_scope_id_line;
		getline(input_file, outer_scope_id_line);
		int outer_scope_id = stoi(outer_scope_id_line);

		string outer_node_id_line;
		getline(input_file, outer_node_id_line);
		int outer_node_id = stoi(outer_node_id_line);

		this->curr_outer_contexts_needed.insert({outer_scope_id, outer_node_id});
	}

	string clean_outer_node_index_line;
	getline(input_file, clean_outer_node_index_line);
	this->clean_outer_node_index = stoi(clean_outer_node_index_line);

	string clean_outer_state_index_line;
	getline(input_file, clean_outer_state_index_line);
	this->clean_outer_state_index = stoi(clean_outer_state_index_line);

	string num_new_inner_states_line;
	getline(input_file, num_new_inner_states_line);
	this->curr_num_new_inner_states = stoi(num_new_inner_states_line);

	string inner_state_networks_not_needed_size_line;
	getline(input_file, inner_state_networks_not_needed_size_line);
	int inner_state_networks_not_needed_size = stoi(inner_state_networks_not_needed_size_line);
	for (int s_index = 0; s_index < inner_state_networks_not_needed_size; s_index++) {
		string inner_scope_id_line;
		getline(input_file, inner_scope_id_line);
		int inner_scope_id = stoi(inner_scope_id_line);

		this->curr_inner_state_networks_not_needed.insert({inner_scope_id, vector<vector<bool>>()});

		string num_nodes_line;
		getline(input_file, num_nodes_line);
		int num_nodes = stoi(num_nodes_line);
		for (int n_index = 0; n_index < num_nodes; n_index++) {
			this->curr_inner_state_networks_not_needed[inner_scope_id].push_back(vector<bool>());

			string num_states_line;
			getline(input_file, num_states_line);
			int num_states = stoi(num_states_line);
			for (int s_index = 0; s_index < num_states; s_index++) {
				string is_not_needed_line;
				getline(input_file, is_not_needed_line);
				this->curr_inner_state_networks_not_needed[inner_scope_id].back().push_back(stoi(is_not_needed_line));
			}
		}
	}

	string inner_state_networks_size_line;
	getline(input_file, inner_state_networks_size_line);
	int inner_state_networks_size = stoi(inner_state_networks_size_line);
	for (int s_index = 0; s_index < inner_state_networks_size; s_index++) {
		string inner_scope_id_line;
		getline(input_file, inner_scope_id_line);
		int inner_scope_id = stoi(inner_scope_id_line);

		this->curr_inner_state_networks.insert({inner_scope_id, vector<vector<StateNetwork*>>()});

		string num_nodes_line;
		getline(input_file, num_nodes_line);
		int num_nodes = stoi(num_nodes_line);
		for (int n_index = 0; n_index < num_nodes; n_index++) {
			this->curr_inner_state_networks[inner_scope_id].push_back(vector<StateNetwork*>());

			string num_states_line;
			getline(input_file, num_states_line);
			int num_states = stoi(num_states_line);
			for (int s_index = 0; s_index < num_states; s_index++) {
				if (this->curr_inner_state_networks_not_needed.size() > 0	// may not be initialized
						&& this->curr_inner_state_networks_not_needed[inner_scope_id][n_index][s_index]) {
					this->curr_inner_state_networks[inner_scope_id].back().push_back(NULL);
				} else {
					ifstream inner_state_network_save_file;
					inner_state_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_inner_state_" + to_string(inner_scope_id) + "_" + to_string(n_index) + "_" + to_string(s_index) + ".txt");
					this->curr_inner_state_networks[inner_scope_id].back().push_back(new StateNetwork(inner_state_network_save_file));
					inner_state_network_save_file.close();
				}
			}
		}
	}

	string clean_inner_scope_index_line;
	getline(input_file, clean_inner_scope_index_line);
	this->clean_inner_scope_index = stoi(clean_inner_scope_index_line);

	string inner_scopes_needed_size_line;
	getline(input_file, inner_scopes_needed_size_line);
	int inner_scopes_needed_size = stoi(inner_scopes_needed_size_line);
	for (int s_index = 0; s_index < inner_scopes_needed_size; s_index++) {
		string inner_scope_id_line;
		getline(input_file, inner_scope_id_line);
		this->curr_inner_scopes_needed.insert(stoi(inner_scope_id_line));
	}

	string inner_contexts_needed_size_line;
	getline(input_file, inner_contexts_needed_size_line);
	int inner_contexts_needed_size = stoi(inner_contexts_needed_size_line);
	for (int c_index = 0; c_index < inner_contexts_needed_size; c_index++) {
		string inner_scope_id_line;
		getline(input_file, inner_scope_id_line);
		int inner_scope_id = stoi(inner_scope_id_line);

		string inner_node_id_line;
		getline(input_file, inner_node_id_line);
		int inner_node_id = stoi(inner_node_id_line);

		this->curr_inner_contexts_needed.insert({inner_scope_id, inner_node_id});
	}

	string clean_inner_node_index_line;
	getline(input_file, clean_inner_node_index_line);
	this->clean_inner_node_index = stoi(clean_inner_node_index_line);

	string clean_inner_state_index_line;
	getline(input_file, clean_inner_state_index_line);
	this->clean_inner_state_index = stoi(clean_inner_state_index_line);

	string clean_inner_step_index_line;
	getline(input_file, clean_inner_step_index_line);
	this->clean_inner_step_index = stoi(clean_inner_step_index_line);

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states;
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states
		+ this->curr_num_new_outer_states;

	this->curr_state_networks_not_needed = vector<vector<bool>>(this->sequence_length, vector<bool>(num_inner_networks));
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < num_inner_networks; s_index++) {
			string is_not_needed_line;
			getline(input_file, is_not_needed_line);
			this->curr_state_networks_not_needed[f_index][s_index] = stoi(is_not_needed_line);
		}
	}
	this->test_state_networks_not_needed = this->curr_state_networks_not_needed;

	this->curr_state_networks = vector<vector<StateNetwork*>>(this->sequence_length, vector<StateNetwork*>(num_inner_networks, NULL));
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < num_inner_networks; s_index++) {
			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
				ifstream state_network_save_file;
				state_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_state_" + to_string(f_index) + "_" + to_string(s_index) + ".txt");
				this->curr_state_networks[f_index][s_index] = new StateNetwork(state_network_save_file);
				state_network_save_file.close();
			}
		}
	}

	this->curr_score_networks = vector<StateNetwork*>(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		ifstream score_network_save_file;
		score_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_score_" + to_string(f_index) + ".txt");
		this->curr_score_networks[f_index] = new StateNetwork(score_network_save_file);
		score_network_save_file.close();
	}

	this->curr_state_not_needed_locally = vector<vector<bool>>(this->sequence_length, vector<bool>(total_num_states));
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < total_num_states; s_index++) {
			string is_not_needed_line;
			getline(input_file, is_not_needed_line);
			this->curr_state_not_needed_locally[f_index][s_index] = stoi(is_not_needed_line);
		}
	}
	this->test_state_not_needed_locally = this->curr_state_not_needed_locally;

	this->curr_num_states_cleared = vector<int>(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		string num_states_cleared_line;
		getline(input_file, num_states_cleared_line);
		this->curr_num_states_cleared[f_index] = stoi(num_states_cleared_line);
	}
	this->test_num_states_cleared = this->curr_num_states_cleared;

	this->seed_outer_context_history = NULL;

	if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		remove_outer_scope_from_load();
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK) {
		remove_outer_scope_network_from_load();
	} else if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE) {
		remove_inner_scope_from_load();
	} else if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK) {
		remove_inner_scope_network_from_load();
	} else if (this->state == FOLD_STATE_REMOVE_INNER_NETWORK) {
		remove_inner_network_from_load();
	} else if (this->state == FOLD_STATE_REMOVE_INNER_STATE) {
		remove_inner_state_from_load();
	} else {
		// this->state == FOLD_STATE_CLEAR_INNER_STATE
		clear_inner_state_from_load();
	}
}

Fold::~Fold() {
	// clean in case of early exit
	if (this->state != FOLD_STATE_EXPERIMENT_FAIL
			&& this->state != FOLD_STATE_DONE) {
		for (int f_index = 0; f_index < (int)this->inner_scope_scale_mods.size(); f_index++) {
			if (this->inner_scope_scale_mods[f_index] != NULL) {
				delete this->inner_scope_scale_mods[f_index];
			}
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
				it != this->curr_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					if (it->second[n_index][s_index] != NULL) {
						delete it->second[n_index][s_index];
					}
				}
			}
		}

		if (this->curr_starting_score_network != NULL) {
			delete this->curr_starting_score_network;
		}

		for (int f_index = 0; f_index < (int)this->curr_state_networks.size(); f_index++) {
			for (int s_index = 0; s_index < (int)this->curr_state_networks[f_index].size(); s_index++) {
				if (this->curr_state_networks[f_index][s_index] != NULL) {
					delete this->curr_state_networks[f_index][s_index];
				}
			}
		}

		for (int f_index = 0; f_index < (int)this->curr_score_networks.size(); f_index++) {
			delete this->curr_score_networks[f_index];
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
				it != this->curr_inner_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					if (it->second[n_index][s_index] != NULL) {
						delete it->second[n_index][s_index];
					}
				}
			}
		}

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_outer_state_networks.begin();
				it != this->test_outer_state_networks.end(); it++) {
			for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
				for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
					if (it->second[n_index][s_index] != NULL) {
						delete it->second[n_index][s_index];
					}
				}
			}
		}

		if (this->state != FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK) {	// TODO: find more elegant way of catching this
			if (this->test_starting_score_network != NULL) {
				delete this->test_starting_score_network;
			}

			for (int f_index = 0; f_index < (int)this->test_state_networks.size(); f_index++) {
				for (int s_index = 0; s_index < (int)this->test_state_networks[f_index].size(); s_index++) {
					if (this->test_state_networks[f_index][s_index] != NULL) {
						delete this->test_state_networks[f_index][s_index];
					}
				}
			}

			for (int f_index = 0; f_index < (int)this->test_score_networks.size(); f_index++) {
				delete this->test_score_networks[f_index];
			}

			for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->test_inner_state_networks.begin();
					it != this->test_inner_state_networks.end(); it++) {
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
						if (it->second[n_index][s_index] != NULL) {
							delete it->second[n_index][s_index];
						}
					}
				}
			}
		}
	}

	if (this->seed_outer_context_history != NULL) {
		delete this->seed_outer_context_history;
	}
}

void Fold::score_activate(vector<double>& state_vals,
						  double& predicted_score,
						  double& scale_factor,
						  vector<ScopeHistory*>& context_histories,
						  RunHelper& run_helper,
						  FoldHistory* history) {
	if (this->state_iter == -1) {
		// used to catch if state change occurred
		this->state_iter = 0;
	}

	if (this->state == FOLD_STATE_EXPERIMENT
			|| this->state == FOLD_STATE_ADD_OUTER_STATE
			|| this->state == FOLD_STATE_ADD_INNER_STATE) {
		experiment_score_activate(state_vals,
								  predicted_score,
								  scale_factor,
								  context_histories,
								  run_helper,
								  history);
	} else if (this->state == FOLD_STATE_REMOVE_INNER_INPUT) {
		remove_inner_input_score_activate(state_vals,
										  predicted_score,
										  scale_factor,
										  context_histories,
										  run_helper,
										  history);
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		remove_outer_scope_score_activate(state_vals,
										  context_histories,
										  run_helper,
										  history);
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK) {
		remove_outer_scope_network_score_activate(state_vals,
												  context_histories,
												  run_helper,
												  history);
	} else {
		clean_score_activate(state_vals,
							 context_histories,
							 history);
	}
}

void Fold::sequence_activate(Problem& problem,
							 vector<double>& state_vals,
							 vector<bool>& states_initialized,
							 double& predicted_score,
							 double& scale_factor,
							 double& sum_impact,
							 RunHelper& run_helper,
							 FoldHistory* history) {
	if (this->state == FOLD_STATE_EXPERIMENT
			|| this->state == FOLD_STATE_ADD_OUTER_STATE
			|| this->state == FOLD_STATE_ADD_INNER_STATE) {
		experiment_sequence_activate(problem,
									 state_vals,
									 states_initialized,
									 predicted_score,
									 scale_factor,
									 run_helper,
									 history);
	} else if (this->state == FOLD_STATE_REMOVE_INNER_INPUT) {
		remove_inner_input_sequence_activate(problem,
											 state_vals,
											 states_initialized,
											 predicted_score,
											 scale_factor,
											 run_helper,
											 history);
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		remove_outer_scope_sequence_activate(problem,
											 state_vals,
											 states_initialized,
											 predicted_score,
											 scale_factor,
											 sum_impact,
											 run_helper,
											 history);
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK) {
		remove_outer_scope_network_sequence_activate(problem,
													 state_vals,
													 states_initialized,
													 predicted_score,
													 scale_factor,
													 sum_impact,
													 run_helper,
													 history);
	} else {
		clean_sequence_activate(problem,
								state_vals,
								states_initialized,
								predicted_score,
								scale_factor,
								sum_impact,
								run_helper,
								history);
	}
}

void Fold::sequence_backprop(vector<double>& state_errors,
							 vector<bool>& states_initialized,
							 double target_val,
							 double final_diff,
							 double final_misguess,
							 double final_sum_impact,
							 double& predicted_score,
							 double& scale_factor,
							 double& scale_factor_error,
							 RunHelper& run_helper,
							 FoldHistory* history) {
	if (this->state == FOLD_STATE_EXPERIMENT
			|| this->state == FOLD_STATE_ADD_OUTER_STATE
			|| this->state == FOLD_STATE_ADD_INNER_STATE) {
		// don't split between score and sequence for backprop for experiment
		experiment_backprop(state_errors,
							states_initialized,
							target_val,
							final_diff,
							final_misguess,
							predicted_score,
							scale_factor,
							run_helper,
							history);

		if (this->state == FOLD_STATE_EXPERIMENT
				&& this->state_iter < 200000
				&& this->state_iter%10 == 0) {
			seed_train();
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_INPUT) {
		remove_inner_input_backprop(state_errors,
									states_initialized,
									target_val,
									final_diff,
									final_misguess,
									predicted_score,
									scale_factor,
									run_helper,
									history);
	} else {
		clean_sequence_backprop(state_errors,
								states_initialized,
								target_val,
								final_diff,
								final_misguess,
								final_sum_impact,
								predicted_score,
								scale_factor,
								scale_factor_error,
								run_helper,
								history);
	}
}

void Fold::score_backprop(vector<double>& state_errors,
						  double target_val,
						  double& predicted_score,
						  double& scale_factor,
						  double& scale_factor_error,
						  RunHelper& run_helper,
						  FoldHistory* history) {
	clean_score_backprop(state_errors,
						 target_val,
						 predicted_score,
						 scale_factor,
						 scale_factor_error,
						 run_helper,
						 history);
}

void Fold::experiment_increment() {
	this->state_iter++;

	if (this->state == FOLD_STATE_EXPERIMENT) {
		if (this->state_iter == 500000) {
			experiment_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_ADD_OUTER_STATE) {
		if (this->state_iter == 500000) {
			add_outer_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_ADD_INNER_STATE) {
		if (this->state_iter == 500000) {
			add_inner_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == FOLD_STATE_REMOVE_INNER_INPUT
		if (this->state_iter == 150000) {
			remove_inner_input_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	}
}

void Fold::clean_increment() {
	if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				remove_outer_scope_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK) {
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				remove_outer_scope_network_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE) {
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				remove_inner_scope_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK) {
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				remove_inner_scope_network_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_NETWORK) {
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				remove_inner_network_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_STATE) {
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				remove_inner_state_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == FOLD_STATE_CLEAR_INNER_STATE
		if (this->state_iter > 150000) {
			if (this->sub_iter >= 10000) {
				clear_inner_state_end();
			}
		} else {
			if (this->sub_iter >= 10000) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;

				this->sub_iter = 0;
				this->sum_error = 0.0;
			}
		}
	}
}

void Fold::save(ofstream& output_file,
				int scope_id,
				int scope_index) {
	output_file << this->scope_context.size() << endl;
	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		output_file << this->scope_context[c_index] << endl;
		output_file << this->node_context[c_index] << endl;
	}
	output_file << this->exit_depth << endl;

	output_file << this->sequence_length << endl;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		output_file << this->is_inner_scope[f_index] << endl;

		if (this->is_inner_scope[f_index]) {
			output_file << this->existing_scope_ids[f_index] << endl;

			output_file << this->inner_scope_scale_mods[f_index] << endl;
		} else {
			this->actions[f_index].save(output_file);
		}
	}

	output_file << this->num_score_states << endl;
	output_file << this->num_sequence_states << endl;

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			output_file << this->num_inner_inputs[f_index] << endl;
		}
	}

	output_file << this->state << endl;

	output_file << this->curr_num_new_outer_states << endl;

	output_file << this->curr_outer_state_networks_not_needed.size() << endl;
	for (map<int, vector<vector<bool>>>::iterator it = this->curr_outer_state_networks_not_needed.begin();
			it != this->curr_outer_state_networks_not_needed.end(); it++) {
		output_file << it->first << endl;

		output_file << it->second.size() << endl;
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			output_file << it->second[n_index].size() << endl;
			for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
				output_file << it->second[n_index][s_index] << endl;
			}
		}
	}

	output_file << this->curr_outer_state_networks.size() << endl;
	for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_outer_state_networks.begin();
			it != this->curr_outer_state_networks.end(); it++) {
		output_file << it->first << endl;

		output_file << it->second.size() << endl;
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			output_file << it->second[n_index].size() << endl;
			for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
				if (this->curr_outer_state_networks_not_needed.size() > 0	// may not be initialized
						&& this->curr_outer_state_networks_not_needed[it->first][n_index][s_index]) {
					// do nothing
				} else {
					ofstream outer_state_network_save_file;
					outer_state_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_outer_state_" + to_string(it->first) + "_" + to_string(n_index) + "_" + to_string(s_index) + ".txt");
					it->second[n_index][s_index]->save(outer_state_network_save_file);
					outer_state_network_save_file.close();
				}
			}
		}
	}

	ofstream starting_score_network_save_file;
	starting_score_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_starting_score.txt");
	this->curr_starting_score_network->save(starting_score_network_save_file);
	starting_score_network_save_file.close();

	for (int i_index = 0; i_index < this->sum_inner_inputs; i_index++) {
		output_file << this->curr_inner_inputs_needed[i_index] << endl;
	}

	output_file << this->clean_outer_scope_index << endl;

	output_file << this->curr_outer_scopes_needed.size() << endl;
	for (set<int>::iterator it = this->curr_outer_scopes_needed.begin();
			it != this->curr_outer_scopes_needed.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->curr_outer_contexts_needed.size() << endl;
	for (set<pair<int, int>>::iterator it = this->curr_outer_contexts_needed.begin();
			it != this->curr_outer_contexts_needed.end(); it++) {
		output_file << (*it).first << endl;
		output_file << (*it).second << endl;
	}

	output_file << this->clean_outer_node_index << endl;
	output_file << this->clean_outer_state_index << endl;

	output_file << this->curr_num_new_inner_states << endl;

	output_file << this->curr_inner_state_networks_not_needed.size() << endl;
	for (map<int, vector<vector<bool>>>::iterator it = this->curr_inner_state_networks_not_needed.begin();
			it != this->curr_inner_state_networks_not_needed.end(); it++) {
		output_file << it->first << endl;

		output_file << it->second.size() << endl;
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			output_file << it->second[n_index].size() << endl;
			for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
				output_file << it->second[n_index][s_index] << endl;
			}
		}
	}

	output_file << this->curr_inner_state_networks.size() << endl;
	for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->curr_inner_state_networks.begin();
			it != this->curr_inner_state_networks.end(); it++) {
		output_file << it->first << endl;

		output_file << it->second.size() << endl;
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			output_file << it->second[n_index].size() << endl;
			for (int s_index = 0; s_index < (int)it->second[n_index].size(); s_index++) {
				if (this->curr_inner_state_networks_not_needed.size() > 0	// may not be initialized
						&& this->curr_inner_state_networks_not_needed[it->first][n_index][s_index]) {
					// do nothing
				} else {
					ofstream inner_state_network_save_file;
					inner_state_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_inner_state_" + to_string(it->first) + "_" + to_string(n_index) + "_" + to_string(s_index) + ".txt");
					it->second[n_index][s_index]->save(inner_state_network_save_file);
					inner_state_network_save_file.close();
				}
			}
		}
	}

	output_file << this->clean_inner_scope_index << endl;

	output_file << this->curr_inner_scopes_needed.size() << endl;
	for (set<int>::iterator it = this->curr_inner_scopes_needed.begin();
			it != this->curr_inner_scopes_needed.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->curr_inner_contexts_needed.size() << endl;
	for (set<pair<int, int>>::iterator it = this->curr_inner_contexts_needed.begin();
			it != this->curr_inner_contexts_needed.end(); it++) {
		output_file << (*it).first << endl;
		output_file << (*it).second << endl;
	}

	output_file << this->clean_inner_node_index << endl;
	output_file << this->clean_inner_state_index << endl;

	output_file << this->clean_inner_step_index << endl;

	int num_inner_networks = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states;
	int total_num_states = this->sum_inner_inputs
		+ this->curr_num_new_inner_states
		+ this->num_sequence_states
		+ this->curr_num_new_outer_states;

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < num_inner_networks; s_index++) {
			output_file << this->curr_state_networks_not_needed[f_index][s_index] << endl;
		}
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < num_inner_networks; s_index++) {
			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
				ofstream state_network_save_file;
				state_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_state_" + to_string(f_index) + "_" + to_string(s_index) + ".txt");
				this->curr_state_networks[f_index][s_index]->save(state_network_save_file);
				state_network_save_file.close();
			}
		}
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		ofstream score_network_save_file;
		score_network_save_file.open("saves/nns/fold_" + to_string(scope_id) + "_" + to_string(scope_index) + "_score_" + to_string(f_index) + ".txt");
		this->curr_score_networks[f_index]->save(score_network_save_file);
		score_network_save_file.close();
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < total_num_states; s_index++) {
			output_file << this->curr_state_not_needed_locally[f_index][s_index] << endl;
		}
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		output_file << this->curr_num_states_cleared[f_index] << endl;
	}
}

FoldHistory::FoldHistory(Fold* fold) {
	this->fold = fold;

	this->starting_score_network_history = NULL;
	this->test_starting_score_network_history = NULL;
}

FoldHistory::~FoldHistory() {
	for (int n_index = 0; n_index < (int)this->outer_state_network_histories.size(); n_index++) {
		for (int o_index = 0; o_index < (int)this->outer_state_network_histories[n_index].size(); o_index++) {
			if (this->outer_state_network_histories[n_index][o_index] != NULL) {
				delete this->outer_state_network_histories[n_index][o_index];
			}
		}
	}

	if (this->starting_score_network_history != NULL) {
		delete this->starting_score_network_history;
	}

	for (int n_index = 0; n_index < (int)this->test_outer_state_network_histories.size(); n_index++) {
		for (int o_index = 0; o_index < (int)this->test_outer_state_network_histories[n_index].size(); o_index++) {
			if (this->test_outer_state_network_histories[n_index][o_index] != NULL) {
				delete this->test_outer_state_network_histories[n_index][o_index];
			}
		}
	}

	if (this->test_starting_score_network_history != NULL) {
		delete this->test_starting_score_network_history;
	}

	for (int f_index = 0; f_index < (int)this->state_network_histories.size(); f_index++) {
		for (int s_index = 0; s_index < (int)this->state_network_histories[f_index].size(); s_index++) {
			if (this->state_network_histories[f_index][s_index] != NULL) {
				delete this->state_network_histories[f_index][s_index];
			}
		}
	}

	for (int f_index = 0; f_index < (int)this->inner_scope_histories.size(); f_index++) {
		if (this->inner_scope_histories[f_index] != NULL) {
			delete this->inner_scope_histories[f_index];
		}
	}

	for (int f_index = 0; f_index < (int)this->score_network_histories.size(); f_index++) {
		if (this->score_network_histories[f_index] != NULL) {
			delete this->score_network_histories[f_index];
		}
	}

	for (int f_index = 0; f_index < (int)this->inner_state_network_histories.size(); f_index++) {
		for (int n_index = 0; n_index < (int)this->inner_state_network_histories[f_index].size(); n_index++) {
			for (int s_index = 0; s_index < (int)this->inner_state_network_histories[f_index][n_index].size(); s_index++) {
				if (this->inner_state_network_histories[f_index][n_index][s_index] != NULL) {
					delete this->inner_state_network_histories[f_index][n_index][s_index];
				}
			}
		}
	}
}
