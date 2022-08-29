#include "solution_node_loop_end.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeLoopEnd::SolutionNodeLoopEnd(SolutionNode* parent,
										 int node_index) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_END;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	int input_size = 1 + (int)this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->halt_networks_inputs_state_indexes = parent->network_inputs_state_indexes;

	this->halt_network = parent->explore_halt_network;
	parent->explore_halt_network = NULL;

	this->no_halt_network = parent->explore_no_halt_network;
	parent->explore_no_halt_network = NULL;

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

SolutionNodeLoopEnd::SolutionNodeLoopEnd(Solution* solution,
										 int node_index,
										 ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_END;

	if (node_index == 1) {
		this->score_network = NULL;

		this->average_unique_future_nodes = 1;
		this->average_score = 0.0;
		this->average_misguess = 1.0;

		this->temp_node_state = TEMP_NODE_STATE_NOT;

		this->halt_network = NULL;
		this->no_halt_network = NULL;
	} else {
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

		string halt_networks_inputs_state_indexes_size_line;
		getline(save_file, halt_networks_inputs_state_indexes_size_line);
		int halt_networks_inputs_state_indexes_size = stoi(halt_networks_inputs_state_indexes_size_line);
		for (int i_index = 0; i_index < halt_networks_inputs_state_indexes_size; i_index++) {
			string state_index_line;
			getline(save_file, state_index_line);
			this->halt_networks_inputs_state_indexes.push_back(stoi(state_index_line));
		}

		string halt_network_name = "../saves/nns/halt_" + to_string(this->node_index) \
			+ "_" + to_string(this->solution->id) + ".txt";
		ifstream halt_network_save_file;
		halt_network_save_file.open(halt_network_name);
		this->halt_network = new Network(halt_network_save_file);
		halt_network_save_file.close();

		string no_halt_network_name = "../saves/nns/no_halt_" + to_string(this->node_index) \
			+ "_" + to_string(this->solution->id) + ".txt";
		ifstream no_halt_network_save_file;
		no_halt_network_save_file.open(no_halt_network_name);
		this->no_halt_network = new Network(no_halt_network_save_file);
		no_halt_network_save_file.close();
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

SolutionNodeLoopEnd::~SolutionNodeLoopEnd() {
	delete this->halt_score_network;
	delete this->halt_certainty_network;

	delete this->no_halt_score_network;
	delete this->no_halt_certainty_network;
}

void SolutionNodeLoopEnd::reset() {
	this->node_is_on = false;
}

void SolutionNodeLoopEnd::add_potential_state(vector<int> potential_state_indexes,
											  SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->halt_networks_potential_inputs_state_indexes.push_back(
			potential_state_indexes[ps_index]);

		this->halt_score_network->add_potential();
		this->halt_certainty_network->add_potential();
		this->no_halt_score_network->add_potential();
		this->no_halt_certainty_network->add_potential();
	}

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->add_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeLoopEnd::extend_with_potential_state(vector<int> potential_state_indexes,
													  vector<int> new_state_indexes,
													  SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->halt_networks_potential_inputs_state_indexes.size(); pi_index++) {
			if (this->halt_networks_potential_inputs_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->halt_networks_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				
				this->halt_score_network->extend_with_potential(pi_index);
				this->halt_certainty_network->extend_with_potential(pi_index);
				this->no_halt_score_network->extend_with_potential(pi_index);
				this->no_halt_certainty_network->extend_with_potential(pi_index);

				this->halt_networks_potential_inputs_state_indexes.erase(
					this->halt_networks_potential_inputs_state_indexes.begin() + pi_index);

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

void SolutionNodeLoopEnd::delete_potential_state(vector<int> potential_state_indexes,
												 SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->halt_networks_potential_inputs_state_indexes.size(); pi_index++) {
			if (this->halt_networks_potential_inputs_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->halt_score_network->delete_potential(pi_index);
				this->halt_certainty_network->delete_potential(pi_index);
				this->no_halt_score_network->delete_potential(pi_index);
				this->no_halt_certainty_network->delete_potential(pi_index);

				this->halt_networks_potential_inputs_state_indexes.erase(
					this->halt_networks_potential_inputs_state_indexes.begin() + pi_index);

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

SolutionNode* SolutionNodeLoopEnd::activate(Problem& problem,
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
											vector<bool>& explore_loop_decisions,
											bool save_for_display,
											ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

	bool should_halt;
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  true,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  true,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		activate_networks_with_potential(
			problem,
			state_vals,
			states_on,
			loop_scopes,
			loop_scope_counts,
			potential_state_vals,
			potential_state_indexes,,
			true,
			network_historys,
			should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		activate_networks_with_potential(
			problem,
			state_vals,
			states_on,
			loop_scopes,
			loop_scope_counts,
			potential_state_vals,
			potential_state_indexes,,
			false,
			network_historys,
			should_halt);
	}

	if (iter_explore_node == this) {
		if (should_halt) {
			explore_loop_decisions.push_back(true);
		} else {
			explore_loop_decisions.push_back(false);
		}
	}

	if (should_halt) {
		for (int o_index = 0; o_index < (int)this->start->loop_states.size(); o_index++) {
			states_on[this->start->loop_states[o_index]] = false;
		}

		loop_scopes.pop_back();
		loop_scope_counts.pop_back();

		bool is_first_explore = false;
		if (iter_explore_type == EXPLORE_TYPE_NONE) {
			if (randuni() < this->node_weight) {
				if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
					vector<int> available_state;
					for (int s_index = 0; s_index < this->solution->current_state_counter; s_index++) {
						if (states_on[s_index]) {
							available_state.push_back(s_index);
						}
					}

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
								available_state,
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
								available_state,
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
								available_state,
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
								available_state,
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
							available_state,
							-1);
					}

					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_EXPLORE;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_LEARN_PATH;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_MEASURE_PATH;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_LEARN_LOOP;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_MEASURE_LOOP;
				}

				is_first_explore = true;
			}
		}

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
	} else {
		return this->start;
	}
}

void SolutionNodeLoopEnd::backprop(double score,
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
	if (iter_explore_node == this) {
		if (explore_loop_decisions.size() > 0 && explore_loop_decisions.back() == true) {
			explore_backprop(score,
							 misguess,
							 state_errors,
							 states_on,
							 iter_explore_node,
							 potential_state_errors,
							 network_historys,
							 explore_decisions);
		}
		explore_loop_decisions.pop_back();
	}

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
			states_on[this->start->loop_states[s_index]] = true;
		}
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->halt_network) {
			for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
				state_errors[this->start->loop_states[s_index]] = 0.0;
			}
		}

		backprop_networks(score,
						  misguess,
						  state_errors,
						  states_on,
						  network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
			states_on[this->start->loop_states[s_index]] = true;
		}
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->halt_network) {
			for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
				state_errors[this->start->loop_states[s_index]] = 0.0;
			}
		}

		backprop_networks_errors_with_no_weight_change(
			score,
			misguess,
			state_errors,
			states_on,
			network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
			states_on[this->start->loop_states[s_index]] = true;
		}
		if (network_historys.size() > 0) {
			NetworkHistory* network_history = network_historys.back();
			if (network_history->network == this->halt_network) {
				for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
					state_errors[this->start->loop_states[s_index]] = 0.0;
				}
			}
		}

		backprop_networks_with_potential(score,
										 misguess,
										 potential_state_errors,
										 network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		// do nothing
	}
}

void SolutionNodeLoopEnd::save(ofstream& save_file) {
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

	save_file << this->halt_networks_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
		save_file << this->halt_networks_inputs_state_indexes[i_index] << endl;
	}

	string halt_network_name = "../saves/nns/halt_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream halt_network_save_file;
	halt_network_save_file.open(halt_network_name);
	this->halt_network->save(halt_network_save_file);
	halt_network_save_file.close();

	string no_halt_network_name = "../saves/nns/no_halt_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream no_halt_network_save_file;
	no_halt_network_save_file.open(no_halt_network_name);
	this->no_halt_network->save(no_halt_network_save_file);
	no_halt_network_save_file.close();
}

void SolutionNodeLoopEnd::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		save_file << this->start->node_index << endl;
		save_file << this->next->node_index << endl;
	}
}

void SolutionNodeLoopEnd::activate_networks(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											bool backprop,
											vector<NetworkHistory*>& network_historys,
											bool& should_halt) {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->halt_networks_inputs_state_indexes[i_index]]);
		} else {
			inputs.push_back(0.0);
		}
	}

	// loop_scopes.back() == this
	if (loop_scope_counts.back() >= 20) {
		if (backprop) {
			this->halt_score_network->mtx.lock();
			this->halt_score_network->activate(inputs, network_historys);
			this->halt_score_network->mtx.unlock();

			this->halt_certainty_network->mtx.lock();
			this->halt_certainty_network->activate(inputs, network_historys);
			this->halt_certainty_network->mtx.unlock();
		}

		should_halt = true;
		return;
	}

	if (backprop) {
		vector<NetworkHistory*> halt_score_history;
		this->halt_score_network->mtx.lock();
		this->halt_score_network->activate(inputs, halt_score_history);
		double halt_score = this->halt_score_network->output->acti_vals[0];
		this->halt_score_network->mtx.unlock();

		vector<NetworkHistory*> halt_certainty_history;
		this->halt_certainty_network->mtx.lock();
		this->halt_certainty_network->activate(inputs, halt_certainty_history);
		double halt_certainty = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double pinned_halt_certainty = max(halt_certainty, 0.0);
		double halt_curr_combined = pinned_halt_score - pinned_halt_certainty;

		vector<NetworkHistory*> no_halt_score_history;
		this->no_halt_score_network->mtx.lock();
		this->no_halt_score_network->activate(inputs, no_halt_score_history);
		double no_halt_score = this->no_halt_score_network->output->acti_vals[0];
		this->no_halt_score_network->mtx.unlock();

		vector<NetworkHistory*> no_halt_certainty_history;
		this->no_halt_certainty_network->mtx.lock();
		this->no_halt_certainty_network->activate(inputs, no_halt_certainty_history);
		double no_halt_certainty = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double pinned_no_halt_certainty = max(no_halt_certainty, 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - pinned_no_halt_certainty;

		if (no_halt_curr_combined > halt_curr_combined || rand()%20 == 0) {
			network_historys.push_back(no_halt_score_history[0]);
			delete halt_score_history[0];
			network_historys.push_back(no_halt_certainty_history[0]);
			delete halt_certainty_history[0];

			should_halt = false;
			return;
		} else {
			network_historys.push_back(halt_score_history[0]);
			delete no_halt_score_history[0];
			network_historys.push_back(halt_certainty_history[0]);
			delete no_halt_certainty_history[0];

			should_halt = true;
			return;
		}
	} else {
		this->halt_score_network->mtx.lock();
		this->halt_score_network->activate(inputs);
		double halt_score = this->halt_score_network->output->acti_vals[0];
		this->halt_score_network->mtx.unlock();

		this->halt_certainty_network->mtx.lock();
		this->halt_certainty_network->activate(inputs);
		double halt_certainty = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double pinned_halt_certainty = max(halt_certainty, 0.0);
		double halt_curr_combined = pinned_halt_score - pinned_halt_certainty;

		this->no_halt_score_network->mtx.lock();
		this->no_halt_score_network->activate(inputs);
		double no_halt_score = this->no_halt_score_network->output->acti_vals[0];
		this->no_halt_score_network->mtx.unlock();

		this->no_halt_certainty_network->mtx.lock();
		this->no_halt_certainty_network->activate(inputs);
		double no_halt_certainty = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double pinned_no_halt_certainty = max(no_halt_certainty, 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - pinned_no_halt_certainty;

		if (no_halt_curr_combined > halt_curr_combined || rand()%20 == 0) {
			should_halt = false;
			return;
		} else {
			should_halt = true;
			return;
		}
	}
}

void SolutionNodeLoopEnd::backprop_networks(double score,
											double misguess,
											double* state_errors,
											bool* states_on,
											std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	if (certainty_network_history->network == this->halt_certainty_network) {
		this->halt_certainty_network->mtx.lock();

		certainty_network_history->reset_weights();

		vector<double> certainty_errors;
		certainty_errors.push_back(misguess - this->halt_certainty_network->output->acti_vals[0]);
		this->halt_certainty_network->backprop(certainty_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->halt_certainty_network->input->errors[1+i_index];
			}
			this->halt_certainty_network->input->errors[1+i_index] = 0.0;
		}

		this->halt_certainty_network->mtx.unlock();

		delete certainty_network_history;
		network_historys.pop_back();

		NetworkHistory* score_network_history = network_historys.back();

		this->halt_score_network->mtx.lock();

		network_history->reset_weights();

		vector<double> score_errors;
		if (score == 1.0) {
			if (this->halt_score_network->output->acti_vals[0] < 1.0) {
				score_errors.push_back(1.0 - this->halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		} else {
			if (this->halt_score_network->output->acti_vals[0] > 0.0) {
				score_errors.push_back(0.0 - this->halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		}
		this->halt_score_network->backprop(score_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->halt_score_network->input->errors[1+i_index];
			}
			this->halt_score_network->input->errors[1+i_index] = 0.0;
		}

		this->halt_score_network->mtx.unlock();

		delete score_network_history;
		network_historys.pop_back();
	} else if (certainty_network_history->network == this->no_halt_certainty_network) {
		this->no_halt_certainty_network->mtx.lock();

		certainty_network_history->reset_weights();

		vector<double> certainty_errors;
		certainty_errors.push_back(misguess - this->no_halt_certainty_network->output->acti_vals[0]);
		this->no_halt_certainty_network->backprop(certainty_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->no_halt_certainty_network->input->errors[1+i_index];
			}
			this->no_halt_certainty_network->input->errors[1+i_index] = 0.0;
		}

		this->no_halt_certainty_network->mtx.unlock();

		delete certainty_network_history;
		network_historys.pop_back();

		NetworkHistory* score_network_history = network_historys.back();

		this->no_halt_score_network->mtx.lock();

		score_network_history->reset_weights();

		vector<double> score_errors;
		if (score == 1.0) {
			if (this->no_halt_score_network->output->acti_vals[0] < 1.0) {
				score_errors.push_back(1.0 - this->no_halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		} else {
			if (this->no_halt_score_network->output->acti_vals[0] > 0.0) {
				score_errors.push_back(0.0 - this->no_halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		}
		this->no_halt_score_network->backprop(score_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->no_halt_score_network->input->errors[1+i_index];
			}
			this->no_halt_score_network->input->errors[1+i_index] = 0.0;
		}

		this->no_halt_score_network->mtx.unlock();

		delete score_network_history;
		network_historys.pop_back();
	}
}

void SolutionNodeLoopEnd::backprop_networks_errors_with_no_weight_change(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	if (network_history->network == this->halt_certainty_network) {
		this->halt_certainty_network->mtx.lock();

		certainty_network_history->reset_weights();

		vector<double> certainty_errors;
		certainty_errors.push_back(misguess - this->halt_certainty_network->output->acti_vals[0]);
		this->halt_certainty_network->backprop_errors_with_no_weight_change(certainty_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->halt_certainty_network->input->errors[1+i_index];
			}
			this->halt_certainty_network->input->errors[1+i_index] = 0.0;
		}

		this->halt_certainty_network->mtx.unlock();

		delete certainty_network_history;
		network_historys.pop_back();

		NetworkHistory* score_network_history = network_historys.back();

		this->halt_score_network->mtx.lock();

		score_network_history->reset_weights();

		vector<double> score_errors;
		if (score == 1.0) {
			if (this->halt_score_network->output->acti_vals[0] < 1.0) {
				score_errors.push_back(1.0 - this->halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		} else {
			if (this->halt_score_network->output->acti_vals[0] > 0.0) {
				score_errors.push_back(0.0 - this->halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		}
		this->halt_score_network->backprop_errors_with_no_weight_change(score_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->halt_score_network->input->errors[1+i_index];
			}
			this->halt_score_network->input->errors[1+i_index] = 0.0;
		}

		this->halt_score_network->mtx.unlock();

		delete score_network_history;
		network_historys.pop_back();
	} else if (network_history->network == this->no_halt_certainty_network) {
		this->no_halt_certainty_network->mtx.lock();

		certainty_network_history->reset_weights();

		vector<double> certainty_errors;
		certainty_errors.push_back(misguess - this->no_halt_certainty_network->output->acti_vals[0]);
		this->no_halt_certainty_network->backprop_errors_with_no_weight_change(certainty_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->no_halt_certainty_network->input->errors[1+i_index];
			}
			this->no_halt_certainty_network->input->errors[1+i_index] = 0.0;
		}

		this->no_halt_certainty_network->mtx.unlock();

		delete certainty_network_history;
		network_historys.pop_back();

		NetworkHistory* score_network_history = network_historys.back();

		this->no_halt_score_network->mtx.lock();

		score_network_history->reset_weights();

		vector<double> score_errors;
		if (score == 1.0) {
			if (this->no_halt_score_network->output->acti_vals[0] < 1.0) {
				score_errors.push_back(1.0 - this->no_halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		} else {
			if (this->no_halt_score_network->output->acti_vals[0] > 0.0) {
				score_errors.push_back(0.0 - this->no_halt_score_network->output->acti_vals[0]);
			} else {
				score_errors.push_back(0.0);
			}
		}
		this->no_halt_score_network->backprop_errors_with_no_weight_change(score_errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->no_halt_score_network->input->errors[1+i_index];
			}
			this->no_halt_score_network->input->errors[1+i_index] = 0.0;
		}

		this->no_halt_score_network->mtx.unlock();

		delete score_network_history;
		network_historys.pop_back();
	}
}

void SolutionNodeLoopEnd::activate_networks_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		vector<SolutionNode*>& loop_scopes,
		vector<int>& loop_scope_counts,
		double* potential_state_vals,
		vector<int>& potential_state_indexes,
		bool backprop,
		vector<NetworkHistory*>& network_historys,
		bool& should_halt) {
	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->halt_networks_potential_inputs_state_indexes.size(); p_index++) {
		if (potential_state_indexes[0] == this->halt_networks_potential_inputs_state_indexes[p_index]) {
			for (int i = 0; i < 2; i++) {
				potentials_on.push_back(p_index+i);
				potential_vals.push_back(potential_state_vals[i]);
			}
			break;
		}
	}

	if (potentials_on.size() == 0) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  should_halt);
		return;
	}

	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->halt_networks_inputs_state_indexes[i_index]]);
		} else {
			inputs.push_back(0.0);
		}
	}

	// loop_scopes.back() == this
	if (loop_scope_counts.back() >= 20) {
		if (backprop) {
			this->halt_network->mtx.lock();
			this->halt_network->activate(inputs,
										 potentials_on,
										 potential_vals,
										 network_historys);
			this->halt_network->mtx.unlock();

			this->halt_certainty_network->mtx.lock();
			this->halt_certainty_network->activate(inputs,
												   potentials_on,
												   potential_vals,
												   network_historys);
			this->halt_certainty_network->mtx.unlock();
		}

		should_halt = true;
		return;
	}

	if (backprop) {
		vector<NetworkHistory*> halt_score_history;
		this->halt_score_network->mtx.lock();
		this->halt_score_network->activate(inputs,
										   potentials_on,
										   potential_vals,
										   halt_score_history);
		double halt_score = this->halt_score_network->output->acti_vals[0];
		this->halt_score_network->mtx.unlock();

		vector<NetworkHistory*> halt_certainty_history;
		this->halt_certainty_network->mtx.lock();
		this->halt_certainty_network->activate(inputs,
											   potentials_on,
											   potential_vals,
											   halt_certainty_history);
		double halt_certainty = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double pinned_halt_certainty = max(halt_certainty, 0.0);
		double halt_curr_combined = pinned_halt_score - pinned_halt_certainty;

		vector<NetworkHistory*> no_halt_score_history;
		this->no_halt_score_network->mtx.lock();
		this->no_halt_score_network->activate(inputs,
											  potentials_on,
											  potential_vals,
											  no_halt_score_history);
		double no_halt_score = this->no_halt_score_network->output->acti_vals[0];
		this->no_halt_score_network->mtx.unlock();

		vector<NetworkHistory*> no_halt_certainty_history;
		this->no_halt_certainty_network->mtx.lock();
		this->no_halt_certainty_network->activate(inputs,
												  potentials_on,
												  potential_vals,
												  no_halt_certainty_history);
		double no_halt_certainty = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double pinned_no_halt_certainty = max(no_halt_certainty, 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - pinned_no_halt_certainty;

		if (no_halt_curr_combined > halt_curr_combined || rand()%20 == 0) {
			network_historys.push_back(no_halt_score_history[0]);
			delete halt_score_history[0];
			network_historys.push_back(no_halt_certainty_history[0]);
			delete halt_certainty_history[0];

			should_halt = false;
			return;
		} else {
			network_historys.push_back(halt_score_history[0]);
			delete no_halt_score_history[0];
			network_historys.push_back(halt_certainty_history[0]);
			delete no_halt_certainty_history[0];

			should_halt = true;
			return;
		}
	} else {
		this->halt_score_network->mtx.lock();
		this->halt_score_network->activate(inputs,
										   potentials_on,
										   potential_vals);
		double halt_score = this->halt_score_network->output->acti_vals[0];
		this->halt_score_network->mtx.unlock();

		this->halt_certainty_network->mtx.lock();
		this->halt_certainty_network->activate(inputs,
											   potentials_on,
											   potential_vals);
		double halt_certainty = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double pinned_halt_certainty = max(halt_certainty, 0.0);
		double halt_curr_combined = pinned_halt_score - pinned_halt_certainty;

		this->no_halt_score_network->mtx.lock();
		this->no_halt_score_network->activate(inputs,
											  potentials_on,
											  potential_vals);
		double no_halt_score = this->no_halt_score_network->output->acti_vals[0];
		this->no_halt_score_network->mtx.unlock();

		this->no_halt_certainty_network->mtx.lock();
		this->no_halt_certainty_network->activate(inputs,
												  potentials_on,
												  potential_vals);
		double no_halt_certainty = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double pinned_no_halt_certainty = max(no_halt_certainty, 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - pinned_no_halt_certainty;

		if (no_halt_curr_combined > halt_curr_combined || rand()%20 == 0) {
			should_halt = false;
			return;
		} else {
			should_halt = true;
			return;
		}
	}
}

void SolutionNodeLoopEnd::backprop_networks_with_potential(
		double score,
		double misguess,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	if (network_historys.size() > 0) {
		NetworkHistory* certainty_network_history = network_historys.back();

		if (certainty_network_history->network == this->halt_certainty_network) {
			this->halt_certainty_network->mtx.lock();

			certainty_network_history->reset_weights();

			vector<int> certainty_potentials_on = certainty_network_history->potentials_on;

			vector<double> certainty_errors;
			certainty_errors.push_back(misguess - this->halt_certainty_network->output->acti_vals[0]);
			this->halt_certainty_network->backprop(certainty_errors, certainty_potentials_on);

			for (int o_index = 0; o_index < (int)certainty_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->halt_certainty_network->
					potential_inputs[certainty_potentials_on[o_index]]->errors[0];
				this->halt_certainty_network->potential_inputs[certainty_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->halt_certainty_network->mtx.unlock();

			delete certainty_network_history;
			network_historys.pop_back();

			NetworkHistory* score_network_history = network_historys.back();

			this->halt_score_network->mtx.lock();

			score_network_history->reset_weights();

			vector<int> score_potentials_on = score_network_history->potentials_on;

			vector<double> score_errors;
			if (score == 1.0) {
				if (this->halt_score_network->output->acti_vals[0] < 1.0) {
					score_errors.push_back(1.0 - this->halt_score_network->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (this->halt_score_network->output->acti_vals[0] > 0.0) {
					score_errors.push_back(0.0 - this->halt_score_network->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			}
			this->halt_score_network->backprop(score_errors, score_potentials_on);

			for (int o_index = 0; o_index < (int)score_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->halt_score_network->
					potential_inputs[score_potentials_on[o_index]]->errors[0];
				this->halt_score_network->potential_inputs[score_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->halt_score_network->mtx.unlock();

			delete score_network_history;
			network_historys.pop_back();
		} else if (network_history->network == this->no_halt_network) {
			this->no_halt_certainty_network->mtx.lock();

			certainty_network_history->reset_weights();

			vector<int> certainty_potentials_on = certainty_network_history->potentials_on;

			vector<double> certainty_errors;
			certainty_errors.push_back(misguess - this->no_halt_certainty_network->output->acti_vals[0]);
			this->no_halt_certainty_network->backprop(certainty_errors, certainty_potentials_on);

			for (int o_index = 0; o_index < (int)certainty_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->no_halt_certainty_network->
					potential_inputs[certainty_potentials_on[o_index]]->errors[0];
				this->no_halt_certainty_network->potential_inputs[certainty_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->no_halt_certainty_network->mtx.unlock();

			delete certainty_network_history;
			network_historys.pop_back();

			NetworkHistory* score_network_history = network_historys.back();

			this->no_halt_score_network->mtx.lock();

			score_network_history->reset_weights();

			vector<int> score_potentials_on = score_network_history->potentials_on;

			vector<double> score_errors;
			if (score == 1.0) {
				if (this->no_halt_score_network->output->acti_vals[0] < 1.0) {
					score_errors.push_back(1.0 - this->no_halt_score_network->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (this->no_halt_score_network->output->acti_vals[0] > 0.0) {
					score_errors.push_back(0.0 - this->no_halt_score_network->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			}
			this->no_halt_score_network->backprop(score_errors, score_potentials_on);

			for (int o_index = 0; o_index < (int)score_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->no_halt_score_network->
					potential_inputs[score_potentials_on[o_index]]->errors[0];
				this->no_halt_score_network->potential_inputs[score_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->no_halt_score_network->mtx.unlock();

			delete score_network_history;
			network_historys.pop_back();
		}
	}
}
