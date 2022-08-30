#include "solution_node_action.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeAction::SolutionNodeAction(Solution* solution,
									   int node_index,
									   Action action,
									   vector<int> available_state) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_ACTION;

	this->score_network_inputs_state_indexes = available_state;

	int input_size = 1 + (int)this->score_network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);

	this->node_weight = 0.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->action = action;

	for (int s_index = 0; s_index < (int)available_state.size(); s_index++) {
		this->state_network_inputs_state_indexes.push_back(available_state);
		Network* new_state_network = new Network(input_size,
												 4*input_size,
												 1);
		this->state_networks.push_back(new_state_network);
		this->state_networks_target_states.push_back(available_state[s_index]);
	}

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_network = NULL;
	this->explore_no_jump_network = NULL;
	this->explore_halt_network = NULL;
	this->explore_no_halt_network = NULL;

	this->node_is_on = false;
}

SolutionNodeAction::SolutionNodeAction(Solution* solution,
									   int node_index,
									   ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_ACTION;

	string score_network_inputs_state_indexes_size_line;
	getline(save_file, score_network_inputs_state_indexes_size_line);
	int score_network_inputs_state_indexes_size = stoi(score_network_inputs_state_indexes_size_line);
	for (int s_index = 0; s_index < score_network_inputs_state_indexes_size; s_index++) {
		string state_index_line;
		getline(save_file, state_index_line);
		this->score_network_inputs_state_indexes.push_back(stoi(state_index_line));
	}

	string score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network = new Network(score_network_save_file);
	score_network_save_file.close();

	string node_weight_line;
	getline(save_file, node_weight_line);
	this->node_weight = stof(node_weight_line);

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

	this->explore_jump_network = NULL;
	this->explore_no_jump_network = NULL;
	this->explore_halt_network = NULL;
	this->explore_no_halt_network = NULL;

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
											 SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->score_network_inputs_potential_state_indexes.push_back(
			potential_state_indexes[ps_index]);
		this->score_network->add_potential();
	}

	if (potential_state_indexes.size() > 2) {
		cout << "PRE ERROR" << endl;
		exit(1);
	}
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		vector<int> potential_inputs_state_indexes;
		for (int i_index = 0; i_index < ps_index+1; i_index++) {
			potential_inputs_state_indexes.push_back(potential_state_indexes[i_index]);
		}

		int input_size = 1 + (int)this->score_network_inputs_state_indexes.size() + ps_index + 1;
		Network* new_potential_state_network = new Network(input_size,
														   input_size*4,
														   1);

		this->potential_inputs_state_indexes.push_back(
			this->score_network_inputs_state_indexes);
		this->potential_potential_inputs_state_indexes.push_back(
			potential_inputs_state_indexes);
		this->potential_state_networks.push_back(new_potential_state_network);
		this->potential_state_networks_target_states.push_back(potential_state_indexes[ps_index]);
	}

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->add_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeAction::extend_with_potential_state(vector<int> potential_state_indexes,
													 vector<int> new_state_indexes,
													 SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->score_network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->score_network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->score_network_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				this->score_network->extend_with_potential(pi_index);
				this->score_network_inputs_potential_state_indexes.erase(
					this->score_network_inputs_potential_state_indexes.begin() + pi_index);
				break;
			}
		}
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int t_index = 0; t_index < (int)this->potential_state_networks_target_states.size(); t_index++) {
			if (this->potential_state_networks_target_states[t_index]
					== potential_state_indexes[ps_index]) {
				vector<int> new_state_network_inputs_state_indexes(
					this->potential_inputs_state_indexes[t_index]);
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
				this->potential_inputs_state_indexes.erase(
					this->potential_inputs_state_indexes.begin() + t_index);
				this->potential_potential_inputs_state_indexes.erase(
					this->potential_potential_inputs_state_indexes.begin() + t_index);
				
				this->state_networks.push_back(this->potential_state_networks[t_index]);
				this->potential_state_networks.erase(
					this->potential_state_networks.begin() + t_index);
				
				this->state_networks_target_states.push_back(new_state_indexes[ps_index]);
				this->potential_state_networks_target_states.erase(
					this->potential_state_networks_target_states.begin() + t_index);

				break;
			}
		}
	}

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											explore_node);
}

void SolutionNodeAction::delete_potential_state(vector<int> potential_state_indexes,
												SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->score_network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->score_network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->score_network->delete_potential(pi_index);
				this->score_network_inputs_potential_state_indexes.erase(
					this->score_network_inputs_potential_state_indexes.begin() + pi_index);
				break;
			}
		}
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int t_index = 0; t_index < (int)this->potential_state_networks_target_states.size(); t_index++) {
			if (this->potential_state_networks_target_states[t_index]
					== potential_state_indexes[ps_index]) {
				this->potential_inputs_state_indexes.erase(
					this->potential_inputs_state_indexes.begin() + t_index);
				this->potential_potential_inputs_state_indexes.erase(
					this->potential_potential_inputs_state_indexes.begin() + t_index);

				delete this->potential_state_networks[t_index];
				this->potential_state_networks.erase(
					this->potential_state_networks.begin() + t_index);

				this->potential_state_networks_target_states.erase(
					this->potential_state_networks_target_states.begin() + t_index);

				break;
			}
		}
	}

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->delete_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeAction::clear_potential_state() {
	this->score_network_inputs_potential_state_indexes.clear();
	this->score_network->remove_potentials();

	this->potential_inputs_state_indexes.clear();
	this->potential_potential_inputs_state_indexes.clear();
	for (int sn_index = 0; sn_index < (int)this->potential_state_networks.size(); sn_index++) {
		delete this->potential_state_networks[sn_index];
	}
	this->potential_state_networks.clear();
	this->potential_state_networks_target_states.clear();
}

// SolutionNode* SolutionNodeAction::activate(Problem& problem,
// 										   double* state_vals,
// 										   bool* states_on,
// 										   vector<SolutionNode*>& loop_scopes,
// 										   vector<int>& loop_scope_counts,
// 										   int& iter_explore_type,
// 										   SolutionNode*& iter_explore_node,
// 										   IterExplore*& iter_explore,
// 										   double* potential_state_vals,
// 										   vector<int>& potential_state_indexes,
// 										   vector<NetworkHistory*>& network_historys,
// 										   vector<vector<double>>& guesses,
// 										   vector<int>& explore_decisions,
// 										   vector<bool>& explore_loop_decisions,
// 										   bool save_for_display,
// 										   ofstream& display_file) {
SolutionNode* SolutionNodeAction::activate(Problem& problem,
										   double* state_vals,
										   bool* states_on,
										   vector<SolutionNode*>& loop_scopes,
										   vector<int>& loop_scope_counts,
										   int& iter_explore_type,
										   SolutionNode*& iter_explore_node,
										   IterExplore*& iter_explore,
										   double* potential_state_vals,
										   vector<int>& potential_state_indexes,
										   vector<NetworkHistory*>& network_historys,
										   vector<vector<double>>& guesses,
										   vector<int>& explore_decisions,
										   vector<bool>& explore_loop_decisions) {
	// if (save_for_display) {
	// 	display_file << this->node_index << endl;
	// 	this->action.save(display_file);
	// }

	bool is_first_explore = false;
	if (iter_explore_type == EXPLORE_TYPE_NONE) {
		if (randuni() < this->node_weight) {
			if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
				int rand_index = rand()%3;
				if (rand_index == 0) {
					SolutionNode* inclusive_end;
					SolutionNode* non_inclusive_end;
					find_scope_end(this, inclusive_end, non_inclusive_end);
					
					geometric_distribution<int> seq_length_dist(0.2);
					normal_distribution<double> write_val_dist(0.0, 2.0);
					vector<Action> try_path;
					if (this == inclusive_end) {
						int seq_length = 1 + seq_length_dist(generator);
						for (int i = 0; i < seq_length; i++) {
							Action a(write_val_dist(generator), rand()%3);
							try_path.push_back(a);
						}

						iter_explore = new IterExplore(
							ITER_EXPLORE_TYPE_JUMP,
							try_path,
							this,
							NULL,
							NULL,
							this->next,
							this->score_network_inputs_state_indexes,
							-1);
					} else {
						int seq_length = seq_length_dist(generator);
						for (int i = 0; i < seq_length; i++) {
							Action a(write_val_dist(generator), rand()%3);
							try_path.push_back(a);
						}

						iter_explore = new IterExplore(
							ITER_EXPLORE_TYPE_JUMP,
							try_path,
							this,
							this->next,
							inclusive_end,
							non_inclusive_end,
							this->score_network_inputs_state_indexes,
							-1);
					}
				} else if (rand_index == 1) {
					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					find_potential_jumps(this,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);
					int random_index = rand()%(int)potential_inclusive_jump_ends.size();

					geometric_distribution<int> seq_length_dist(0.2);
					normal_distribution<double> write_val_dist(0.0, 2.0);
					vector<Action> try_path;
					if (this == potential_inclusive_jump_ends[random_index]) {
						int seq_length = 1 + seq_length_dist(generator);
						for (int i = 0; i < seq_length; i++) {
							Action a(write_val_dist(generator), rand()%3);
							try_path.push_back(a);
						}

						iter_explore = new IterExplore(
							ITER_EXPLORE_TYPE_JUMP,
							try_path,
							this,
							NULL,
							NULL,
							this->next,
							this->score_network_inputs_state_indexes,
							-1);
					} else {
						int seq_length = seq_length_dist(generator);
						for (int i = 0; i < seq_length; i++) {
							Action a(write_val_dist(generator), rand()%3);
							try_path.push_back(a);
						}

						iter_explore = new IterExplore(
							ITER_EXPLORE_TYPE_JUMP,
							try_path,
							this,
							this->next,
							potential_inclusive_jump_ends[random_index],
							potential_non_inclusive_jump_ends[random_index],
							this->score_network_inputs_state_indexes,
							-1);
					}
				} else {
					vector<SolutionNode*> potential_non_inclusive_loop_starts;
					vector<SolutionNode*> potential_inclusive_loop_starts;
					find_potential_loops(this,
										 potential_non_inclusive_loop_starts,
										 potential_inclusive_loop_starts);
					int random_index = rand()%(int)potential_non_inclusive_loop_starts.size();

					vector<Action> empty_try_path;
					iter_explore = new IterExplore(
						ITER_EXPLORE_TYPE_LOOP,
						empty_try_path,
						potential_non_inclusive_loop_starts[random_index],
						potential_inclusive_loop_starts[random_index],
						this,
						this->next,
						this->score_network_inputs_state_indexes,
						-1);
				}

				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_EXPLORE;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_JUMP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_JUMP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
				potential_state_indexes = this->explore_loop_states;
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_LOOP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP) {
				potential_state_indexes = this->explore_loop_states;
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_LOOP;
			}

			is_first_explore = true;
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
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								true,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		activate_state_networks_with_potential(problem,
											   state_vals,
											   states_on,
											   potential_state_vals,
											   potential_state_indexes,
											   true,
											   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		activate_state_networks_with_potential(problem,
											   state_vals,
											   states_on,
											   potential_state_vals,
											   potential_state_indexes,
											   false,
											   network_historys);
	}

	problem.perform_action(this->action);

	double score;
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		score = activate_score_network(problem,
									   state_vals,
									   states_on,
									   true,
									   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		score = activate_score_network(problem,
									   state_vals,
									   states_on,
									   false,
									   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		score = activate_score_network(problem,
									   state_vals,
									   states_on,
									   false,
									   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		score = activate_score_network(problem,
									   state_vals,
									   states_on,
									   true,
									   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		score = activate_score_network(problem,
									   state_vals,
									   states_on,
									   false,
									   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		score = activate_score_network_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_state_indexes,
			true,
			network_historys);
	} else {
		// iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP
		score = activate_score_network_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_state_indexes,
			false,
			network_historys);
	}
	guesses.back().push_back(score);

	SolutionNode* explore_node = NULL;
	if (iter_explore_node == this) {
		explore_node = explore_activate(problem,
										state_vals,
										states_on,
										loop_scopes,
										loop_scope_counts,
										iter_explore_type,
										iter_explore_node,
										iter_explore,
										is_first_explore,
										potential_state_vals,
										potential_state_indexes,
										network_historys,
										explore_decisions);
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
								  vector<int>& potential_state_indexes,
								  vector<NetworkHistory*>& network_historys,
								  vector<int>& explore_decisions,
								  vector<bool>& explore_loop_decisions) {
	explore_backprop(score,
					 misguess,
					 state_errors,
					 states_on,
					 iter_explore_node,
					 potential_state_errors,
					 network_historys,
					 explore_decisions);

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_score_network(score,
							   state_errors,
							   states_on,
							   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		if (this->temp_node_state == TEMP_NODE_STATE_LEARN) {
			backprop_score_network(score,
								   state_errors,
								   states_on,
								   network_historys);
		} else {
			backprop_score_network_errors_with_no_weight_change(
				score,
				state_errors,
				states_on,
				network_historys);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		backprop_score_network_with_potential(score,
											  potential_state_errors,
											  network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		// do nothing
	}

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_state_networks(state_errors,
								states_on,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
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
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		backprop_state_networks_with_potential(potential_state_errors,
											   potential_state_indexes,
											   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		// do nothing
	}
}

void SolutionNodeAction::save(ofstream& save_file) {
	save_file << this->score_network_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		save_file << this->score_network_inputs_state_indexes[i_index] << endl;
	}

	string score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->node_weight << endl;

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
	for (int sn_index = (int)this->state_networks_target_states.size() - 1; sn_index >= 0; sn_index--) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			if (network_history->network != this->state_networks[sn_index]) {
				cout << "ERROR: state_network backprop mismatch" << endl;
			}

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
	for (int sn_index = (int)this->state_networks_target_states.size() - 1; sn_index >= 0; sn_index--) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			if (network_history->network != this->state_networks[sn_index]) {
				cout << "ERROR: state_network backprop mismatch" << endl;
			}

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
		vector<int>& potential_state_indexes,
		bool backprop,
		vector<NetworkHistory*>& network_historys) {
	activate_state_networks(problem,
							state_vals,
							states_on,
							false,
							network_historys);

	for (int p_index = 0; p_index < (int)this->potential_state_networks_target_states.size(); p_index++) {
		for (int i_index = 0; i_index < 2; i_index++) {
			if (potential_state_indexes[i_index] == this->potential_state_networks_target_states[p_index]) {
				vector<double> state_network_inputs;
				double curr_observations = problem.get_observation();
				state_network_inputs.push_back(curr_observations);
				for (int i_index = 0; i_index < (int)this->potential_inputs_state_indexes[p_index].size(); i_index++) {
					if (states_on[this->potential_inputs_state_indexes[p_index][i_index]]) {
						state_network_inputs.push_back(state_vals[this->potential_inputs_state_indexes[p_index][i_index]]);
					} else {
						state_network_inputs.push_back(0.0);
					}
				}
				for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes[p_index].size(); pi_index++) {
					state_network_inputs.push_back(potential_state_vals[pi_index]);
				}

				if (backprop) {
					this->potential_state_networks[p_index]->mtx.lock();
					this->potential_state_networks[p_index]->activate(state_network_inputs, network_historys);
					potential_state_vals[i_index] = this->potential_state_networks[p_index]->output->acti_vals[0];
					this->potential_state_networks[p_index]->mtx.unlock();
				} else {
					this->potential_state_networks[p_index]->mtx.lock();
					this->potential_state_networks[p_index]->activate(state_network_inputs);
					potential_state_vals[i_index] = this->potential_state_networks[p_index]->output->acti_vals[0];
					this->potential_state_networks[p_index]->mtx.unlock();
				}
			}
		}
	}
}

void SolutionNodeAction::backprop_state_networks_with_potential(
		double* potential_state_errors,
		vector<int>& potential_state_indexes,
		vector<NetworkHistory*>& network_historys) {
	for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
		for (int i_index = 0; i_index < 2; i_index++) {
			if (potential_state_indexes[i_index] == this->potential_state_networks_target_states[p_index]) {
				NetworkHistory* network_history = network_historys.back();

				this->potential_state_networks[p_index]->mtx.lock();

				if (network_history->network != this->potential_state_networks[p_index]) {
					cout << "ERROR: potential_state_network backprop mismatch" << endl;
				}

				network_history->reset_weights();

				vector<double> state_network_errors;
				state_network_errors.push_back(potential_state_errors[
					this->potential_state_networks_target_states[p_index]]);
				this->potential_state_networks[p_index]->backprop(state_network_errors);

				for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes[p_index].size(); pi_index++) {
					if (this->potential_potential_inputs_state_indexes[p_index][pi_index]
							== this->potential_state_networks_target_states[p_index]) {
						potential_state_errors[pi_index] = this->potential_state_networks[p_index]->input->errors[
							1 + this->potential_inputs_state_indexes[p_index].size() + pi_index];
					} else {
						potential_state_errors[pi_index] += this->potential_state_networks[p_index]->input->errors[
							1 + this->potential_inputs_state_indexes[p_index].size() + pi_index];
					}

					this->potential_state_networks[p_index]->input->errors[
						1 + this->potential_inputs_state_indexes[p_index].size() + pi_index] = 0.0;
				}

				this->potential_state_networks[p_index]->mtx.unlock();

				delete network_history;
				network_historys.pop_back();
			}
		}
	}
}

double SolutionNodeAction::activate_score_network(Problem& problem,
												  double* state_vals,
												  bool* states_on,
												  bool backprop,
												  vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs, network_historys);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	}

	return score;
}

void SolutionNodeAction::backprop_score_network(double score,
												double* state_errors,
												bool* states_on,
												vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->score_network->mtx.lock();

	if (network_history->network != this->score_network) {
		cout << "ERROR: score_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> score_network_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	}
	this->score_network->backprop(score_network_errors);

	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			state_errors[this->score_network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNodeAction::backprop_score_network_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->score_network->mtx.lock();

	if (network_history->network != this->score_network) {
		cout << "ERROR: score_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> score_network_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	}
	this->score_network->backprop_errors_with_no_weight_change(score_network_errors);

	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			state_errors[this->score_network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNodeAction::activate_score_network_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		vector<int>& potential_state_indexes,
		bool backprop,
		vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->score_network_inputs_potential_state_indexes.size(); p_index++) {
		if (potential_state_indexes[0] == this->score_network_inputs_potential_state_indexes[p_index]) {
			for (int i = 0; i < 2; i++) {
				potentials_on.push_back(p_index+i);
				potential_vals.push_back(potential_state_vals[i]);
			}
			break;
		}
	}

	double score;
	if (potentials_on.size() > 0) {
		if (backprop) {
			this->score_network->mtx.lock();
			this->score_network->activate(score_network_inputs,
										  potentials_on,
										  potential_vals,
										  network_historys);
			score = this->score_network->output->acti_vals[0];
			this->score_network->mtx.unlock();
		} else {
			this->score_network->mtx.lock();
			this->score_network->activate(score_network_inputs,
										  potentials_on,
										  potential_vals);
			score = this->score_network->output->acti_vals[0];
			this->score_network->mtx.unlock();
		}
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	}

	return score;
}

void SolutionNodeAction::backprop_score_network_with_potential(
		double score,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	if (network_historys.size() > 0) {
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->score_network) {
			this->score_network->mtx.lock();

			network_history->reset_weights();

			vector<int> potentials_on = network_history->potentials_on;

			vector<double> score_network_errors;
			if (score == 1.0) {
				if (this->score_network->output->acti_vals[0] < 1.0) {
					score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
				} else {
					score_network_errors.push_back(0.0);
				}
			} else {
				if (this->score_network->output->acti_vals[0] > 0.0) {
					score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
				} else {
					score_network_errors.push_back(0.0);
				}
			}
			this->score_network->backprop(score_network_errors,
										  potentials_on);

			for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->score_network->potential_inputs[potentials_on[o_index]]->errors[0];
				this->score_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->score_network->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}
