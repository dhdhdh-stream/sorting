#include "solution_node_loop_end.h"

#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeLoopEnd::SolutionNodeLoopEnd(Solution* solution) {
	this->solution = solution;

	this->node_index = 1;
	this->node_type = NODE_TYPE_LOOP_END;

	this->score_network = NULL;

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->halt_network = NULL;
	this->no_halt_network = NULL;
}

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
	this->score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(time(NULL)) + ".txt";

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->halt_networks_inputs_state_indexes = parent->network_inputs_state_indexes;

	this->halt_network = parent->explore_halt_network;
	parent->explore_halt_network = NULL;
	this->halt_network_name = "../saves/nns/halt_" + to_string(this->node_index) \
		+ "_" + to_string(time(NULL)) + ".txt";

	this->no_halt_network = parent->explore_no_halt_network;
	parent->explore_no_halt_network = NULL;
	this->no_halt_network_name = "../saves/nns/no_halt_" + to_string(this->node_index) \
		+ "_" + to_string(time(NULL)) + ".txt";
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

		string score_network_name_line;
		getline(save_file, score_network_name_line);
		boost::algorithm::trim(score_network_name_line);
		this->score_network_name = score_network_name_line;

		ifstream score_network_save_file;
		score_network_save_file.open(this->score_network_name);
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

		string halt_networks_inputs_state_indexes_size_line;
		getline(save_file, halt_networks_inputs_state_indexes_size_line);
		int halt_networks_inputs_state_indexes_size = stoi(halt_networks_inputs_state_indexes_size_line);
		for (int i_index = 0; i_index < halt_networks_inputs_state_indexes_size; i_index++) {
			string state_index_line;
			getline(save_file, state_index_line);
			this->halt_networks_inputs_state_indexes.push_back(stoi(state_index_line));
		}

		string halt_network_name_line;
		getline(save_file, halt_network_name_line);
		boost::algorithm::trim(halt_network_name_line);
		this->halt_network_name = halt_network_name_line;

		ifstream halt_network_save_file;
		halt_network_save_file.open(halt_network_name_line);
		this->halt_network = new Network(halt_network_save_file);
		halt_network_save_file.close();

		string no_halt_network_name_line;
		getline(save_file, no_halt_network_name_line);
		boost::algorithm::trim(no_halt_network_name_line);
		this->no_halt_network_name = no_halt_network_name_line;

		ifstream no_halt_network_save_file;
		no_halt_network_save_file.open(no_halt_network_name_line);
		this->no_halt_network = new Network(no_halt_network_save_file);
		no_halt_network_save_file.close();
	}
}

SolutionNodeLoopEnd::~SolutionNodeLoopEnd() {
	if (this->score_network != NULL) {
		delete this->score_network;
	}
	if (this->halt_network != NULL) {
		delete this->halt_network;
	}
	if (this->no_halt_network != NULL) {
		delete this->no_halt_network;
	}
}

void SolutionNodeLoopEnd::reset() {
	// do nothing
}

void SolutionNodeLoopEnd::add_potential_state(vector<int> potential_state_indexes,
											  SolutionNode* scope) {
	if (this->node_index == 1) {
		return;
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->halt_networks_potential_inputs_state_indexes.push_back(
			potential_state_indexes[ps_index]);

		this->halt_network->add_potential();
		this->no_halt_network->add_potential();
	}

	if (this->start == scope) {
		return;
	}

	add_potential_state_for_score_network(potential_state_indexes);

	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopEnd::extend_with_potential_state(vector<int> potential_state_indexes,
													  vector<int> new_state_indexes,
													  SolutionNode* scope) {
	if (this->node_index == 1) {
		return;
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->halt_networks_potential_inputs_state_indexes.size(); pi_index++) {
			if (this->halt_networks_potential_inputs_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->halt_networks_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				
				this->halt_network->extend_with_potential(pi_index);
				this->no_halt_network->extend_with_potential(pi_index);

				break;
			}
		}
	}

	if (this->start == scope) {
		return;
	}

	extend_state_for_score_network(potential_state_indexes,
								   new_state_indexes);

	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											scope);
}

void SolutionNodeLoopEnd::reset_potential_state(vector<int> potential_state_indexes,
												SolutionNode* scope) {
	if (this->node_index == 1) {
		return;
	}

	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->halt_networks_potential_inputs_state_indexes.size(); pi_index++) {
			if (this->halt_networks_potential_inputs_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->halt_network->reset_potential(pi_index);
				this->no_halt_network->reset_potential(pi_index);
			}
		}
	}

	if (this->start == scope) {
		return;
	}

	reset_potential_state_for_score_network(potential_state_indexes);

	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopEnd::clear_potential_state() {
	if (this->node_index != 1) {
		this->halt_networks_potential_inputs_state_indexes.clear();

		this->halt_network->remove_potentials();
		this->no_halt_network->remove_potentials();

		clear_potential_state_for_score_network();
	}
}

SolutionNode* SolutionNodeLoopEnd::activate(Problem& problem,
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
	if (this->node_index == 1) {
		loop_scopes.pop_back();
		loop_scope_counts.pop_back();

		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
		// let scope start handle potentials differently for now

		return NULL;
	}

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

	double score;
	bool should_halt;
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  true,
						  network_historys,
						  score,
						  should_halt);
		guesses.push_back(score);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  score,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  score,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  true,
						  network_historys,
						  score,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		activate_networks_with_potential(
			problem,
			state_vals,
			states_on,
			loop_scopes,
			loop_scope_counts,
			potential_state_vals,
			potential_states_on,
			true,
			network_historys,
			score,
			should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		activate_networks(problem,
						  state_vals,
						  states_on,
						  loop_scopes,
						  loop_scope_counts,
						  false,
						  network_historys,
						  score,
						  should_halt);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		activate_networks_with_potential(
			problem,
			state_vals,
			states_on,
			loop_scopes,
			loop_scope_counts,
			potential_state_vals,
			potential_states_on,
			false,
			network_historys,
			score,
			should_halt);
		guesses.push_back(score);
	}

	if (iter_explore_node == this) {
		if (should_halt) {
			explore_loop_decisions.push_back(true);
		} else {
			explore_loop_decisions.push_back(false);
		}
	}

	if (should_halt) {
		loop_scopes.pop_back();
		loop_scope_counts.pop_back();

		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
		// let scope start handle potentials differently for now

		SolutionNode* explore_node = NULL;
		if (iter_explore_node == this) {
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

			// use halt_network misguess instead of score_network misguess
			if (iter_explore_type == EXPLORE_TYPE_RE_EVAL
					|| iter_explore_type == EXPLORE_TYPE_MEASURE_PATH
					|| iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
				guesses.pop_back();
			}
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
								   bool* potential_states_on,
								   vector<NetworkHistory*>& network_historys,
								   vector<int>& explore_decisions,
								   vector<double>& explore_diffs,
								   vector<bool>& explore_loop_decisions) {
	if (explore_loop_decisions.back() == true) {
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
	}
	explore_loop_decisions.pop_back();

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
			states_on[this->start->scope_states_on[s_index]] = true;
		}
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->halt_network) {
			for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
				state_errors[this->start->scope_states_on[s_index]] = 0.0;
			}
		}

		backprop_networks(score,
						  state_errors,
						  states_on,
						  network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
			states_on[this->start->scope_states_on[s_index]] = true;
		}
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->halt_network) {
			for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
				state_errors[this->start->scope_states_on[s_index]] = 0.0;
			}
		}

		backprop_networks_errors_with_no_weight_change(
			score,
			state_errors,
			states_on,
			network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
			states_on[this->start->scope_states_on[s_index]] = true;
		}
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->halt_network) {
			for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
				state_errors[this->start->scope_states_on[s_index]] = 0.0;
			}

			if (iter_explore_node == this->start) {
				for (int ps_index = 0; ps_index < (int)this->start->scope_potential_states.size(); ps_index++) {
					potential_state_errors[this->start->scope_potential_states[ps_index]] = 0.0;
				}
			}
		}

		backprop_networks_with_potential(score,
										 potential_state_errors,
										 network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
	}
}

void SolutionNodeLoopEnd::save(ofstream& save_file) {
	if (this->node_index == 1) {
		return;
	}

	save_file << this->network_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		save_file << this->network_inputs_state_indexes[i_index] << endl;
	}

	save_file << this->score_network_name << endl;
	ofstream score_network_save_file;
	score_network_save_file.open(this->score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->average_unique_future_nodes << endl;
	save_file << this->average_score << endl;
	save_file << this->average_misguess << endl;

	save_file << this->halt_networks_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
		save_file << this->halt_networks_inputs_state_indexes[i_index] << endl;
	}

	save_file << this->halt_network_name << endl;
	ofstream halt_network_save_file;
	halt_network_save_file.open(this->halt_network_name);
	this->halt_network->save(halt_network_save_file);
	halt_network_save_file.close();

	save_file << this->no_halt_network_name << endl;
	ofstream no_halt_network_save_file;
	no_halt_network_save_file.open(this->no_halt_network_name);
	this->no_halt_network->save(no_halt_network_save_file);
	no_halt_network_save_file.close();
}

void SolutionNodeLoopEnd::activate_networks(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											bool backprop,
											vector<NetworkHistory*>& network_historys,
											double& score,
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
			this->halt_network->mtx.lock();
			this->halt_network->activate(inputs, network_historys);
			score = this->halt_network->output->acti_vals[0];
			this->halt_network->mtx.unlock();
		} else {
			this->halt_network->mtx.lock();
			this->halt_network->activate(inputs);
			score = this->halt_network->output->acti_vals[0];
			this->halt_network->mtx.unlock();
		}

		should_halt = true;
		return;
	}

	if (backprop) {
		vector<NetworkHistory*> halt_history;
		this->halt_network->mtx.lock();
		this->halt_network->activate(inputs, halt_history);
		double halt_score = this->halt_network->output->acti_vals[0];
		this->halt_network->mtx.unlock();

		vector<NetworkHistory*> no_halt_history;
		this->no_halt_network->mtx.lock();
		this->no_halt_network->activate(inputs, no_halt_history);
		double no_halt_score = this->no_halt_network->output->acti_vals[0];
		this->no_halt_network->mtx.unlock();

		if (no_halt_score > halt_score || rand()%20 == 0) {
			network_historys.push_back(no_halt_history[0]);
			delete halt_history[0];

			score = no_halt_score;
			should_halt = false;
			return;
		} else {
			network_historys.push_back(halt_history[0]);
			delete no_halt_history[0];

			score = halt_score;
			should_halt = true;
			return;
		}
	} else {
		this->halt_network->mtx.lock();
		this->halt_network->activate(inputs);
		double halt_score = this->halt_network->output->acti_vals[0];
		this->halt_network->mtx.unlock();

		this->no_halt_network->mtx.lock();
		this->no_halt_network->activate(inputs);
		double no_halt_score = this->no_halt_network->output->acti_vals[0];
		this->no_halt_network->mtx.unlock();

		if (no_halt_score > halt_score || rand()%20 == 0) {
			score = no_halt_score;
			should_halt = false;
			return;
		} else {
			score = halt_score;
			should_halt = true;
			return;
		}
	}
}

void SolutionNodeLoopEnd::backprop_networks(double score,
											double* state_errors,
											bool* states_on,
											std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	if (network_history->network == this->halt_network) {
		this->halt_network->mtx.lock();

		network_history->reset_weights();

		vector<double> errors;
		if (score == 1.0) {
			if (this->halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->halt_network->backprop(errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->halt_network->input->errors[1+i_index];
			}
			this->halt_network->input->errors[1+i_index] = 0.0;
		}

		this->halt_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	} else if (network_history->network == this->no_halt_network) {
		this->no_halt_network->mtx.lock();

		network_history->reset_weights();

		vector<double> errors;
		if (score == 1.0) {
			if (this->no_halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->no_halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->no_halt_network->backprop(errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->no_halt_network->input->errors[1+i_index];
			}
			this->no_halt_network->input->errors[1+i_index] = 0.0;
		}

		this->no_halt_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	}
}

void SolutionNodeLoopEnd::backprop_networks_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	if (network_history->network == this->halt_network) {
		this->halt_network->mtx.lock();

		network_history->reset_weights();

		vector<double> errors;
		if (score == 1.0) {
			if (this->halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->halt_network->backprop_errors_with_no_weight_change(errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->halt_network->input->errors[1+i_index];
			}
			this->halt_network->input->errors[1+i_index] = 0.0;
		}

		this->halt_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	} else if (network_history->network == this->no_halt_network) {
		this->no_halt_network->mtx.lock();

		network_history->reset_weights();

		vector<double> errors;
		if (score == 1.0) {
			if (this->no_halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->no_halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->no_halt_network->backprop_errors_with_no_weight_change(errors);

		for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
			if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
				state_errors[this->halt_networks_inputs_state_indexes[i_index]] += \
					this->no_halt_network->input->errors[1+i_index];
			}
			this->no_halt_network->input->errors[1+i_index] = 0.0;
		}

		this->no_halt_network->mtx.unlock();

		delete network_history;
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
		bool* potential_states_on,
		bool backprop,
		vector<NetworkHistory*>& network_historys,
		double& score,
		bool& should_halt) {
	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->halt_networks_potential_inputs_state_indexes.size(); p_index++) {
		if (potential_states_on[this->halt_networks_potential_inputs_state_indexes[p_index]]) {
			potentials_on.push_back(p_index);
			potential_vals.push_back(potential_state_vals[this->halt_networks_potential_inputs_state_indexes[p_index]]);
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
						  score,
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

	if (backprop) {
		vector<NetworkHistory*> halt_history;
		this->halt_network->mtx.lock();
		this->halt_network->activate(inputs,
									 potentials_on,
									 potential_vals,
									 halt_history);
		double halt_score = this->halt_network->output->acti_vals[0];
		this->halt_network->mtx.unlock();

		vector<NetworkHistory*> no_halt_history;
		this->no_halt_network->mtx.lock();
		this->no_halt_network->activate(inputs,
										potentials_on,
										potential_vals,
										no_halt_history);
		double no_halt_score = this->no_halt_network->output->acti_vals[0];
		this->no_halt_network->mtx.unlock();

		if (no_halt_score > halt_score || rand()%20 == 0) {
			network_historys.push_back(no_halt_history[0]);
			delete halt_history[0];

			score = no_halt_score;
			should_halt = false;
			return;
		} else {
			network_historys.push_back(halt_history[0]);
			delete no_halt_history[0];

			score = halt_score;
			should_halt = true;
			return;
		}
	} else {
		this->halt_network->mtx.lock();
		this->halt_network->activate(inputs,
									 potentials_on,
									 potential_vals);
		double halt_score = this->halt_network->output->acti_vals[0];
		this->halt_network->mtx.unlock();

		this->no_halt_network->mtx.lock();
		this->no_halt_network->activate(inputs,
										potentials_on,
										potential_vals);
		double no_halt_score = this->no_halt_network->output->acti_vals[0];
		this->no_halt_network->mtx.unlock();

		if (no_halt_score > halt_score || rand()%20 == 0) {
			score = no_halt_score;
			should_halt = false;
			return;
		} else {
			score = halt_score;
			should_halt = true;
			return;
		}
	}
}

void SolutionNodeLoopEnd::backprop_networks_with_potential(
		double score,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	if (network_history->network == this->halt_network) {
		this->halt_network->mtx.lock();

		network_history->reset_weights();

		vector<int> potentials_on = network_history->potentials_on;

		vector<double> errors;
		if (score == 1.0) {
			if (this->halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->halt_network->backprop(errors, potentials_on);

		for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
			potential_state_errors[this->halt_networks_potential_inputs_state_indexes[potentials_on[o_index]]] += \
				this->halt_network->potential_inputs[potentials_on[o_index]]->errors[0];
			this->halt_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
		}

		this->halt_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	} else if (network_history->network == this->no_halt_network) {
		this->no_halt_network->mtx.lock();

		network_history->reset_weights();

		vector<int> potentials_on = network_history->potentials_on;

		vector<double> errors;
		if (score == 1.0) {
			if (this->no_halt_network->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->no_halt_network->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->no_halt_network->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->no_halt_network->backprop(errors, potentials_on);

		for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
			potential_state_errors[this->halt_networks_potential_inputs_state_indexes[potentials_on[o_index]]] += \
				this->no_halt_network->potential_inputs[potentials_on[o_index]]->errors[0];
			this->no_halt_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
		}

		this->no_halt_network->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	}
}
