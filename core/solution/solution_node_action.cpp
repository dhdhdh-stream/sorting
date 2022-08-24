#include "solution_node_action.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeAction::SolutionNodeAction(SolutionNode* parent,
									   int node_index,
									   Action action) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_ACTION;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	int input_size = 1 + (int)this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->action = action;

	for (int s_index = 0; s_index < (int)this->network_inputs_state_indexes.size(); s_index++) {
		this->state_network_inputs_state_indexes.push_back(this->network_inputs_state_indexes);
		Network* new_state_network = new Network(input_size,
												 4*input_size,
												 1);
		this->state_networks.push_back(new_state_network);
		this->state_networks_target_states.push_back(this->network_inputs_state_indexes[s_index]);
	}

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;
	this->explore_state_state = EXPLORE_STATE_STATE_LEARN;
	this->explore_state_iter_index = 0;

	this->explore_if_network = NULL;
	this->explore_halt_network = NULL;
	this->explore_no_halt_network = NULL;

	this->has_explored_state = false;

	this->node_is_on = false;
}

SolutionNodeAction::SolutionNodeAction(Solution* solution,
									   int node_index,
									   ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_ACTION;

	string network_inputs_state_indexes_size_line;
	getline(save_file, network_inputs_state_indexes_size_line);
	int network_inputs_state_indexes_size = stoi(network_inputs_state_indexes_size_line);
	for (int s_index = 0; s_index < network_inputs_state_indexes_size; s_index++) {
		string state_index_line;
		getline(save_file, state_index_line);
		this->network_inputs_state_indexes.push_back(stoi(state_index_line));
	}

	string score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network = new Network(score_network_save_file);
	score_network_save_file.close();

	string average_unique_future_nodes_line;
	getline(save_file, average_unique_future_nodes_line);
	this->average_unique_future_nodes = stof(average_unique_future_nodes_line);

	string average_score_line;
	getline(save_file, average_score_line);
	this->average_score = stof(average_score_line);

	string average_misguess_line;
	getline(save_file, average_misguess_line);
	this->average_misguess = stof(average_misguess_line);

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	Action action(save_file);
	this->action = action;

	string num_state_networks_line;
	getline(save_file, num_state_networks_line);
	int num_state_networks = stoi(num_state_networks_line);
	for (int s_index = 0; s_index < num_state_networks; s_index++) {
		string num_inputs_line;
		getline(save_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		vector<int> input_state_indexes;
		for (int si_index = 0; si_index < num_inputs; si_index++) {
			string state_index_line;
			getline(save_file, state_index_line);
			int state_index = stoi(state_index_line);
			input_state_indexes.push_back(state_index);
		}
		this->state_network_inputs_state_indexes.push_back(input_state_indexes);

		string target_state_line;
		getline(save_file, target_state_line);
		int target_state = stoi(target_state_line);
		this->state_networks_target_states.push_back(target_state);

		string state_network_name = "../saves/nns/state_" + to_string(this->node_index) \
			+ "_" + to_string(s_index) + "_" + to_string(this->solution->id) + ".txt";
		ifstream state_network_save_file;
		state_network_save_file.open(state_network_name);
		Network* state_network = new Network(state_network_save_file);
		state_network_save_file.close();
		this->state_networks.push_back(state_network);
	}

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;
	this->explore_state_state = EXPLORE_STATE_STATE_LEARN;
	this->explore_state_iter_index = 0;

	this->explore_if_network = NULL;
	this->explore_halt_network = NULL;
	this->explore_no_halt_network = NULL;

	this->has_explored_state = false;

	this->node_is_on = false;
}

SolutionNodeAction::~SolutionNodeAction() {
	delete this->score_network;

	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		delete this->state_networks[s_index];
	}
}

void SolutionNodeAction::reset() {
	this->node_is_on = false;
}

void SolutionNodeAction::add_potential_state(vector<int> potential_state_indexes,
											 SolutionNode* scope) {
	add_potential_state_for_score_network(potential_state_indexes);

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		vector<int> potential_inputs_state_indexes;
		for (int i_index = 0; i_index < ps_index+1; i_index++) {
			potential_inputs_state_indexes.push_back(potential_state_indexes[i_index]);
		}

		int input_size = 1 + (int)this->network_inputs_state_indexes.size() + ps_index + 1;
		Network* new_potential_state_network = new Network(input_size,
														   input_size*4,
														   1);

		this->potential_potential_inputs_state_indexes.push_back(
			potential_inputs_state_indexes);
		this->potential_state_networks.push_back(new_potential_state_network);
		this->potential_state_networks_target_states.push_back(potential_state_indexes[ps_index]);
	}

	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeAction::extend_with_potential_state(vector<int> potential_state_indexes,
													 vector<int> new_state_indexes,
													 SolutionNode* scope) {
	extend_state_for_score_network(potential_state_indexes,
								   new_state_indexes);

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int t_index = 0; t_index < (int)this->potential_state_networks_target_states.size(); t_index++) {
			if (this->potential_state_networks_target_states[t_index]
					== potential_state_indexes[ps_index]) {
				vector<int> new_state_network_inputs_state_indexes(
					this->network_inputs_state_indexes);
				for (int i_index = 0; i_index < (int)this->potential_potential_inputs_state_indexes[t_index].size(); i_index++) {
					for (int ns_index = 0; ns_index < (int)potential_state_indexes.size(); ns_index++) {
						if (potential_state_indexes[ns_index]
								== this->potential_potential_inputs_state_indexes[t_index][i_index]) {
							new_state_network_inputs_state_indexes.push_back(new_state_indexes[ns_index]);
							break;
						}
					}
				}
				this->state_network_inputs_state_indexes.push_back(new_state_network_inputs_state_indexes);
				this->state_networks.push_back(this->potential_state_networks[t_index]);
				this->state_networks_target_states.push_back(new_state_indexes[ps_index]);

				break;
			}
		}
	}

	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											scope);
}

void SolutionNodeAction::reset_potential_state(vector<int> potential_state_indexes,
											   SolutionNode* scope) {
	reset_potential_state_for_score_network(potential_state_indexes);

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int t_index = 0; t_index < (int)this->potential_state_networks_target_states.size(); t_index++) {
			if (this->potential_state_networks_target_states[t_index]
					== potential_state_indexes[ps_index]) {
				this->potential_state_networks[t_index]->reset();
			}
		}
	}

	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeAction::clear_potential_state() {
	clear_potential_state_for_score_network();

	this->potential_potential_inputs_state_indexes.clear();
	for (int sn_index = 0; sn_index < (int)this->potential_state_networks.size(); sn_index++) {
		delete this->potential_state_networks[sn_index];
	}
	this->potential_state_networks.clear();
	this->potential_state_networks_target_states.clear();
}

SolutionNode* SolutionNodeAction::activate(Problem& problem,
										   double* state_vals,
										   bool* states_on,
										   vector<SolutionNode*>& loop_scopes,
										   vector<int>& loop_scope_counts,
										   bool is_first_time,
										   int& iter_explore_type,
										   SolutionNode*& iter_explore_node,
										   double* potential_state_vals,
										   bool* potential_states_on,
										   vector<NetworkHistory*>& network_historys,
										   vector<double>& guesses,
										   vector<int>& explore_decisions,
										   vector<double>& explore_diffs,
										   vector<bool>& explore_loop_decisions) {
	if (iter_explore_type == EXPLORE_TYPE_NONE && is_first_time) {
		if (randuni() < (1.0/this->average_unique_future_nodes)) {
			if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
				int rand_index = rand()%3;
				if (rand_index == 0) {
					SolutionNode* inclusive_end;
					SolutionNode* non_inclusive_end;
					find_scope_end(this, inclusive_end, non_inclusive_end);
					
					geometric_distribution<int> seq_length_dist(0.2);
					int seq_length;
					if (this == inclusive_end) {
						seq_length = 1 + seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = NULL;
						this->explore_end_inclusive = NULL;
						this->explore_end_non_inclusive = this->next;
					} else {
						seq_length = seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = this->next;
						this->explore_end_inclusive = inclusive_end;
						this->explore_end_non_inclusive = non_inclusive_end;
					}

					try_path.clear();

					normal_distribution<double> write_val_dist(0.0, 2.0);
					for (int i = 0; i < seq_length; i++) {
						Action a(write_val_dist(generator), rand()%3);
						try_path.push_back(a);
					}

					this->path_explore_type = PATH_EXPLORE_TYPE_JUMP;
				} else if (rand_index == 1) {
					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					find_potential_jumps(this,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);
					int random_index = rand()%(int)potential_inclusive_jump_ends.size();

					geometric_distribution<int> seq_length_dist(0.2);
					int seq_length;
					if (this == potential_inclusive_jump_ends[random_index]) {
						seq_length = 1 + seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = NULL;
						this->explore_end_inclusive = NULL;
						this->explore_end_non_inclusive = this->next;
					} else {
						seq_length = seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = this->next;
						this->explore_end_inclusive = potential_inclusive_jump_ends[random_index];
						this->explore_end_non_inclusive = potential_non_inclusive_jump_ends[random_index];
					}

					try_path.clear();

					normal_distribution<double> write_val_dist(0.0, 2.0);
					for (int i = 0; i < seq_length; i++) {
						Action a(write_val_dist(generator), rand()%3);
						try_path.push_back(a);
					}

					this->path_explore_type = PATH_EXPLORE_TYPE_JUMP;
				} else {
					vector<SolutionNode*> potential_non_inclusive_loop_starts;
					vector<SolutionNode*> potential_inclusive_loop_starts;
					find_potential_loops(this,
										 potential_non_inclusive_loop_starts,
										 potential_inclusive_loop_starts);
					int random_index = rand()%(int)potential_non_inclusive_loop_starts.size();

					this->explore_start_non_inclusive = potential_non_inclusive_loop_starts[random_index];
					this->explore_start_inclusive = potential_inclusive_loop_starts[random_index];
					this->explore_end_inclusive = this;
					this->explore_end_non_inclusive = this->next;
					
					this->path_explore_type = PATH_EXPLORE_TYPE_LOOP;
				}

				if (rand()%2 == 0) {
					this->path_target_type = PATH_TARGET_TYPE_GOOD;
				} else {
					this->path_target_type = PATH_TARGET_TYPE_BAD;
				}

				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_EXPLORE;
				this->explore_path_used = false;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_PATH;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_PATH;
			}
		}
	}

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								true,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								true,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		activate_state_networks_with_potential(problem,
											   state_vals,
											   states_on,
											   potential_state_vals,
											   potential_states_on,
											   true,
											   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		activate_state_networks_with_potential(problem,
											   state_vals,
											   states_on,
											   potential_state_vals,
											   potential_states_on,
											   false,
											   network_historys);
	}

	problem.perform_action(this->action);

	double score = activate_score_network_helper(problem,
												 state_vals,
												 states_on,
												 loop_scopes,
												 loop_scope_counts,
												 iter_explore_type,
												 iter_explore_node,
												 potential_state_vals,
												 potential_states_on,
												 network_historys,
												 guesses);

	SolutionNode* explore_node = NULL;
	if (iter_explore_node == this) {
		explore_node = explore(score,
							   problem,
							   state_vals,
							   states_on,
							   loop_scopes,
							   loop_scope_counts,
							   iter_explore_type,
							   iter_explore_node,
							   potential_state_vals,
							   potential_states_on,
							   network_historys,
							   guesses,
							   explore_decisions,
							   explore_diffs);
	}

	if (explore_node != NULL) {
		return explore_node;
	} else {
		return this->next;
	}
}

void SolutionNodeAction::backprop(double score,
								  double misguess,
								  double* state_errors,
								  bool* states_on,
								  int& iter_explore_type,
								  SolutionNode*& iter_explore_node,
								  double* potential_state_errors,
								  bool* potential_states_on,
								  vector<NetworkHistory*>& network_historys,
								  vector<int>& explore_decisions,
								  vector<double>& explore_diffs,
								  vector<bool>& explore_loop_decisions) {
	backprop_explore_and_score_network_helper(score,
											  misguess,
											  state_errors,
											  states_on,
											  iter_explore_type,
											  iter_explore_node,
											  potential_state_errors,
											  potential_states_on,
											  network_historys,
											  explore_decisions,
											  explore_diffs);

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_state_networks(state_errors,
								states_on,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		if (this->temp_node_state == TEMP_NODE_STATE_LEARN) {
			backprop_state_networks(state_errors,
									states_on,
									network_historys);
		} else {
			backprop_state_networks_errors_with_no_weight_change(
				state_errors,
				states_on,
				network_historys);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		backprop_state_networks_with_potential(potential_state_errors,
											   potential_states_on,
											   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
	}
}

void SolutionNodeAction::save(ofstream& save_file) {
	save_file << this->network_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		save_file << this->network_inputs_state_indexes[i_index] << endl;
	}

	string score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->average_unique_future_nodes << endl;
	save_file << this->average_score << endl;
	save_file << this->average_misguess << endl;

	this->action.save(save_file);

	save_file << this->state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		save_file << this->state_network_inputs_state_indexes[s_index].size() << endl;
		for (int si_index = 0; si_index < (int)this->state_network_inputs_state_indexes[s_index].size(); si_index++) {
			save_file << this->state_network_inputs_state_indexes[s_index][si_index] << endl;
		}
		save_file << this->state_networks_target_states[s_index] << endl;
	
		string state_network_name = "../saves/nns/state_" + to_string(this->node_index) \
			+ "_" + to_string(s_index) + "_" + to_string(this->solution->id) + ".txt";
		ofstream state_network_save_file;
		state_network_save_file.open(state_network_name);
		this->state_networks[s_index]->save(state_network_save_file);
		state_network_save_file.close();
	}
}

void SolutionNodeAction::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		this->action.save(save_file);
		save_file << this->next->node_index << endl;
	}
}

void SolutionNodeAction::activate_state_networks(Problem& problem,
												 double* state_vals,
												 bool* states_on,
												 bool backprop,
												 vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[sn_index][i_index]]) {
					state_network_inputs.push_back(state_vals[this->state_network_inputs_state_indexes[sn_index][i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}

			if (backprop) {
				this->state_networks[sn_index]->mtx.lock();
				this->state_networks[sn_index]->activate(state_network_inputs, network_historys);
				state_vals[this->state_networks_target_states[sn_index]] = \
					this->state_networks[sn_index]->output->acti_vals[0];
				this->state_networks[sn_index]->mtx.unlock();
			} else {
				this->state_networks[sn_index]->mtx.lock();
				this->state_networks[sn_index]->activate(state_network_inputs);
				state_vals[this->state_networks_target_states[sn_index]] = \
					this->state_networks[sn_index]->output->acti_vals[0];
				this->state_networks[sn_index]->mtx.unlock();
			}
		}
	}
}

void SolutionNodeAction::backprop_state_networks(double* state_errors,
												 bool* states_on,
												 vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[
				this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->backprop(state_network_errors);

			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[sn_index][i_index]]) {
					if (this->state_network_inputs_state_indexes[sn_index][i_index]
							== this->state_networks_target_states[sn_index]) {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] = \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					} else {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] += \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					}
				}
				this->state_networks[sn_index]->input->errors[1 + i_index] = 0.0;
			}

			this->state_networks[sn_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}

void SolutionNodeAction::backprop_state_networks_errors_with_no_weight_change(
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[
				this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->backprop_errors_with_no_weight_change(state_network_errors);

			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[sn_index][i_index]]) {
					if (this->state_network_inputs_state_indexes[sn_index][i_index]
							== this->state_networks_target_states[sn_index]) {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] = \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					} else {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] += \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					}
				}
				this->state_networks[sn_index]->input->errors[1 + i_index] = 0.0;
			}

			this->state_networks[sn_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}

void SolutionNodeAction::activate_state_networks_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		bool* potential_states_on,
		bool backprop,
		vector<NetworkHistory*>& network_historys) {
	activate_state_networks(problem,
							state_vals,
							states_on,
							false,
							network_historys);

	for (int p_index = 0; p_index < (int)this->potential_state_networks_target_states.size(); p_index++) {
		if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
				if (states_on[this->network_inputs_state_indexes[i_index]]) {
					state_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}
			for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes[p_index].size(); pi_index++) {
				if (potential_states_on[this->potential_potential_inputs_state_indexes[p_index][pi_index]]) {
					state_network_inputs.push_back(potential_state_vals[this->potential_potential_inputs_state_indexes[p_index][pi_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}
			if (backprop) {
				this->potential_state_networks[p_index]->mtx.lock();
				this->potential_state_networks[p_index]->activate(state_network_inputs, network_historys);
				potential_state_vals[this->potential_state_networks_target_states[p_index]] = \
					this->potential_state_networks[p_index]->output->acti_vals[0];
				this->potential_state_networks[p_index]->mtx.unlock();
			} else {
				this->potential_state_networks[p_index]->mtx.lock();
				this->potential_state_networks[p_index]->activate(state_network_inputs);
				potential_state_vals[this->potential_state_networks_target_states[p_index]] = \
					this->potential_state_networks[p_index]->output->acti_vals[0];
				this->potential_state_networks[p_index]->mtx.unlock();
			}
		}
	}
}

void SolutionNodeAction::backprop_state_networks_with_potential(
		double* potential_state_errors,
		bool* potential_states_on,
		vector<NetworkHistory*>& network_historys) {
	for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
		if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->potential_state_networks[p_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(potential_state_errors[
				this->potential_state_networks_target_states[p_index]]);
			this->potential_state_networks[p_index]->backprop(state_network_errors);

			for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes[p_index].size(); pi_index++) {
				if (potential_states_on[this->potential_potential_inputs_state_indexes[p_index][pi_index]]) {
					if (this->potential_potential_inputs_state_indexes[p_index][pi_index]
							== this->potential_state_networks_target_states[p_index]) {
						potential_state_errors[this->potential_potential_inputs_state_indexes[p_index][pi_index]] = \
							this->potential_state_networks[p_index]->input->errors[
								1 + this->network_inputs_state_indexes.size() + pi_index];
					} else {
						potential_state_errors[this->potential_potential_inputs_state_indexes[p_index][pi_index]] += \
							this->potential_state_networks[p_index]->input->errors[
								1 + this->network_inputs_state_indexes.size() + pi_index];
					}
				}

				this->potential_state_networks[p_index]->input->errors[
					1 + this->network_inputs_state_indexes.size() + pi_index] = 0.0;
			}

			this->potential_state_networks[p_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}
