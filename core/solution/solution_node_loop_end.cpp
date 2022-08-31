#include "solution_node_loop_end.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeLoopEnd::SolutionNodeLoopEnd(SolutionNode* parent,
										 int node_index,
										 vector<int> loop_states) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_END;

	this->network_inputs_state_indexes = parent->explore_network_inputs_state_indexes;

	int input_size = 1 + (int)this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);
	this->certainty_network = new Network(input_size,
										  4*input_size,
										  1);

	this->node_weight = 0.0;

	this->halt_networks_inputs_state_indexes = parent->explore_network_inputs_state_indexes;
	this->halt_networks_inputs_state_indexes.insert(this->halt_networks_inputs_state_indexes.begin(),
		loop_states.begin(), loop_states.end());

	this->halt_score_network = parent->explore_halt_score_network;
	parent->explore_halt_score_network = NULL;
	this->halt_certainty_network = parent->explore_halt_certainty_network;
	parent->explore_halt_certainty_network = NULL;

	this->no_halt_score_network = parent->explore_no_halt_score_network;
	parent->explore_no_halt_score_network = NULL;
	this->no_halt_certainty_network = parent->explore_no_halt_certainty_network;
	parent->explore_no_halt_certainty_network = NULL;

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_score_network = NULL;
	this->explore_jump_certainty_network = NULL;
	this->explore_halt_score_network = NULL;
	this->explore_halt_certainty_network = NULL;
	this->explore_no_halt_score_network = NULL;
	this->explore_no_halt_certainty_network = NULL;

	this->node_is_on = false;
}

SolutionNodeLoopEnd::SolutionNodeLoopEnd(Solution* solution,
										 int node_index,
										 ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_END;

	load_score_network(save_file);

	string halt_networks_inputs_state_indexes_size_line;
	getline(save_file, halt_networks_inputs_state_indexes_size_line);
	int halt_networks_inputs_state_indexes_size = stoi(halt_networks_inputs_state_indexes_size_line);
	for (int i_index = 0; i_index < halt_networks_inputs_state_indexes_size; i_index++) {
		string state_index_line;
		getline(save_file, state_index_line);
		this->halt_networks_inputs_state_indexes.push_back(stoi(state_index_line));
	}

	string halt_score_network_name = "../saves/nns/halt_score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream halt_score_network_save_file;
	halt_score_network_save_file.open(halt_score_network_name);
	this->halt_score_network = new Network(halt_score_network_save_file);
	halt_score_network_save_file.close();

	string halt_certainty_network_name = "../saves/nns/halt_certainty_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream halt_certainty_network_save_file;
	halt_certainty_network_save_file.open(halt_certainty_network_name);
	this->halt_certainty_network = new Network(halt_certainty_network_save_file);
	halt_certainty_network_save_file.close();

	string no_halt_score_network_name = "../saves/nns/no_halt_score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream no_halt_score_network_save_file;
	no_halt_score_network_save_file.open(no_halt_score_network_name);
	this->no_halt_score_network = new Network(no_halt_score_network_save_file);
	no_halt_score_network_save_file.close();

	string no_halt_certainty_network_name = "../saves/nns/no_halt_certainty_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream no_halt_certainty_network_save_file;
	no_halt_certainty_network_save_file.open(no_halt_certainty_network_name);
	this->no_halt_certainty_network = new Network(no_halt_certainty_network_save_file);
	no_halt_certainty_network_save_file.close();

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_score_network = NULL;
	this->explore_jump_certainty_network = NULL;
	this->explore_halt_score_network = NULL;
	this->explore_halt_certainty_network = NULL;
	this->explore_no_halt_score_network = NULL;
	this->explore_no_halt_certainty_network = NULL;

	this->node_is_on = false;
}

SolutionNodeLoopEnd::~SolutionNodeLoopEnd() {
	delete this->score_network;
	delete this->certainty_network;

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

	// TODO: break when adding state to scope

	score_network_add_potential_state(potential_state_indexes);

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

	// TODO: break when adding state to scope

	score_network_extend_with_potential_state(potential_state_indexes,
											  new_state_indexes);

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

	// TODO: break when adding state to scope

	score_network_delete_potential_state(potential_state_indexes);

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->delete_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeLoopEnd::clear_potential_state() {
	this->halt_networks_potential_inputs_state_indexes.clear();

	this->halt_score_network->remove_potentials();
	this->halt_certainty_network->remove_potentials();
	this->no_halt_score_network->remove_potentials();
	this->no_halt_certainty_network->remove_potentials();

	score_network_clear_potential_state();
}

SolutionNode* SolutionNodeLoopEnd::activate(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											vector<bool>& loop_decisions,
											int& iter_explore_type,
											SolutionNode*& iter_explore_node,
											IterExplore*& iter_explore,
											double* potential_state_vals,
											vector<int>& potential_state_indexes,
											vector<NetworkHistory*>& network_historys,
											vector<vector<double>>& guesses,
											vector<int>& explore_decisions,
											bool save_for_display,
											ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

	bool is_first_explore = false;
	if (iter_explore_type == EXPLORE_TYPE_NONE) {
		if (randuni() < this->node_weight) {
			if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
				// clear state early/twice to set correct available_state
				for (int o_index = 0; o_index < (int)this->start->loop_states.size(); o_index++) {
					states_on[this->start->loop_states[o_index]] = false;
				}
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

	bool should_halt;
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  true,
						  false,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
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
						  is_first_explore,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  true,
						  is_first_explore,
						  network_historys,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  is_first_explore,
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
			potential_state_indexes,
			true,
			is_first_explore,
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
			potential_state_indexes,
			false,
			is_first_explore,
			network_historys,
			should_halt);
	}

	if (should_halt) {
		for (int o_index = 0; o_index < (int)this->start->loop_states.size(); o_index++) {
			states_on[this->start->loop_states[o_index]] = false;
		}

		loop_scopes.pop_back();
		loop_scope_counts.pop_back();

		activate_helper(problem,
						state_vals,
						states_on,
						iter_explore_type,
						iter_explore_node,
						potential_state_vals,
						potential_state_indexes,
						network_historys,
						guesses);

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
											guesses,
											explore_decisions);
		}

		loop_decisions.push_back(true);

		if (explore_node != NULL) {
			return explore_node;
		} else {
			return this->next;
		}
	} else {
		loop_decisions.push_back(false);

		return this->start;
	}
}

void SolutionNodeLoopEnd::backprop(double score,
								   double misguess,
								   double* state_errors,
								   bool* states_on,
								   vector<bool>& loop_decisions,
								   int& iter_explore_type,
								   SolutionNode*& iter_explore_node,
								   double* potential_state_errors,
								   vector<int>& potential_state_indexes,
								   vector<NetworkHistory*>& network_historys,
								   vector<int>& explore_decisions) {
	if (loop_decisions.back() == true) {
		explore_backprop(score,
						 misguess,
						 state_errors,
						 states_on,
						 iter_explore_node,
						 potential_state_errors,
						 network_historys,
						 explore_decisions);

		backprop_helper(score,
						misguess,
						state_errors,
						states_on,
						iter_explore_type,
						iter_explore_node,
						potential_state_errors,
						network_historys);
	}
	loop_decisions.pop_back();

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		for (int s_index = 0; s_index < (int)this->start->loop_states.size(); s_index++) {
			states_on[this->start->loop_states[s_index]] = true;
		}
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->halt_certainty_network) {
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
		if (network_history->network == this->halt_certainty_network) {
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
			if (network_history->network == this->halt_certainty_network) {
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
	save_score_network(save_file);

	save_file << this->halt_networks_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
		save_file << this->halt_networks_inputs_state_indexes[i_index] << endl;
	}

	string halt_score_network_name = "../saves/nns/halt_score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream halt_score_network_save_file;
	halt_score_network_save_file.open(halt_score_network_name);
	this->halt_score_network->save(halt_score_network_save_file);
	halt_score_network_save_file.close();

	string halt_certainty_network_name = "../saves/nns/halt_certainty_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream halt_certainty_network_save_file;
	halt_certainty_network_save_file.open(halt_certainty_network_name);
	this->halt_certainty_network->save(halt_certainty_network_save_file);
	halt_certainty_network_save_file.close();

	string no_halt_score_network_name = "../saves/nns/no_halt_score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream no_halt_score_network_save_file;
	no_halt_score_network_save_file.open(no_halt_score_network_name);
	this->no_halt_score_network->save(no_halt_score_network_save_file);
	no_halt_score_network_save_file.close();

	string no_halt_certainty_network_name = "../saves/nns/no_halt_certainty_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream no_halt_certainty_network_save_file;
	no_halt_certainty_network_save_file.open(no_halt_certainty_network_name);
	this->no_halt_certainty_network->save(no_halt_certainty_network_save_file);
	no_halt_certainty_network_save_file.close();
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
											bool is_first_explore,
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
	if (is_first_explore || loop_scope_counts.back() >= 20) {
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
		double halt_misguess = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double halt_curr_combined = pinned_halt_score - halt_misguess;

		vector<NetworkHistory*> no_halt_score_history;
		this->no_halt_score_network->mtx.lock();
		this->no_halt_score_network->activate(inputs, no_halt_score_history);
		double no_halt_score = this->no_halt_score_network->output->acti_vals[0];
		this->no_halt_score_network->mtx.unlock();

		vector<NetworkHistory*> no_halt_certainty_history;
		this->no_halt_certainty_network->mtx.lock();
		this->no_halt_certainty_network->activate(inputs, no_halt_certainty_history);
		double no_halt_misguess = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - no_halt_misguess;

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
		double halt_misguess = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double halt_curr_combined = pinned_halt_score - halt_misguess;

		this->no_halt_score_network->mtx.lock();
		this->no_halt_score_network->activate(inputs);
		double no_halt_score = this->no_halt_score_network->output->acti_vals[0];
		this->no_halt_score_network->mtx.unlock();

		this->no_halt_certainty_network->mtx.lock();
		this->no_halt_certainty_network->activate(inputs);
		double no_halt_misguess = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - no_halt_misguess;

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

	if (certainty_network_history->network == this->halt_certainty_network) {
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
	} else if (certainty_network_history->network == this->no_halt_certainty_network) {
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
		bool is_first_explore,
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
						  is_first_explore,
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
	if (is_first_explore || loop_scope_counts.back() >= 20) {
		if (backprop) {
			this->halt_score_network->mtx.lock();
			this->halt_score_network->activate(inputs,
											   potentials_on,
											   potential_vals,
											   network_historys);
			this->halt_score_network->mtx.unlock();

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
		double halt_misguess = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double halt_curr_combined = pinned_halt_score - halt_misguess;

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
		double no_halt_misguess = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - no_halt_misguess;

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
		double halt_misguess = this->halt_certainty_network->output->acti_vals[0];
		this->halt_certainty_network->mtx.unlock();

		double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
		double halt_curr_combined = pinned_halt_score - halt_misguess;

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
		double no_halt_misguess = this->no_halt_certainty_network->output->acti_vals[0];
		this->no_halt_certainty_network->mtx.unlock();

		double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
		double no_halt_curr_combined = pinned_no_halt_score - no_halt_misguess;

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
		} else if (certainty_network_history->network == this->no_halt_certainty_network) {
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
