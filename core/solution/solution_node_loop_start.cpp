#include "solution_node_loop_start.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeLoopStart::SolutionNodeLoopStart(Solution* solution) {
	this->solution = solution;

	this->node_index = 0;
	this->node_type = NODE_TYPE_LOOP_START;

	this->score_network = new Network(1, 4, 1);

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	for (int i = 0; i < 6; i++) {
		this->explore_state_scores.push_back(0.0);
		this->explore_state_misguesses.push_back(0.0);
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

SolutionNodeLoopStart::SolutionNodeLoopStart(SolutionNode* parent,
											 int node_index) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_START;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	int input_size = 1 + (int)this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	for (int i = 0; i < 6; i++) {
		this->explore_state_scores.push_back(0.0);
		this->explore_state_misguesses.push_back(0.0);
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

SolutionNodeLoopStart::SolutionNodeLoopStart(Solution* solution,
											 int node_index,
											 ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_START;

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

	for (int i = 0; i < 6; i++) {
		this->explore_state_scores.push_back(0.0);
		this->explore_state_misguesses.push_back(0.0);
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

SolutionNodeLoopStart::~SolutionNodeLoopStart() {
	delete this->score_network;
}

void SolutionNodeLoopStart::reset() {
	this->node_is_on = false;
}

void SolutionNodeLoopStart::add_potential_state(vector<int> potential_state_indexes,
												SolutionNode* explore_node) {
	// this can't be explore node
	this->next->add_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeLoopStart::extend_with_potential_state(vector<int> potential_state_indexes,
														vector<int> new_state_indexes,
														SolutionNode* explore_node) {
	// this can't be explore node
	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											explore_node);
}

void SolutionNodeLoopStart::reset_potential_state(vector<int> potential_state_indexes,
												  SolutionNode* explore_node) {
	// this can't be explore node
	this->next->reset_potential_state(potential_state_indexes, explore_node);
}

SolutionNode* SolutionNodeLoopStart::activate(Problem& problem,
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

	if (loop_scopes.back() == this) {
		loop_scope_counts.back()++;
	} else {
		for (int o_index = 0; o_index < (int)this->loop_states.size(); o_index++) {
			state_vals[this->loop_states[o_index]] = 0.0;
			states_on[this->loop_states[o_index]] = true;
		}

		loop_scopes.push_back(this);
		loop_scope_counts.push_back(1);
	}

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

				vector<SolutionNode*> potential_inclusive_jump_ends;
				vector<SolutionNode*> potential_non_inclusive_jump_ends;
				potential_inclusive_jump_ends.push_back(this);
				potential_non_inclusive_jump_ends.push_back(this->next);
				find_potential_jumps(this->next,
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

				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_EXPLORE;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_JUMP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_JUMP;
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
}

void SolutionNodeLoopStart::backprop(double score,
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
		for (int o_index = 0; o_index < (int)this->loop_states.size(); o_index++) {
			states_on[this->loop_states[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		for (int o_index = 0; o_index < (int)this->loop_states.size(); o_index++) {
			states_on[this->loop_states[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		for (int o_index = 0; o_index < (int)this->loop_states.size(); o_index++) {
			states_on[this->loop_states[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
	}
}

void SolutionNodeLoopStart::save(ofstream& save_file) {
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
}

void SolutionNodeLoopStart::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		save_file << this->next->node_index << endl;
		save_file << this->loop_states.size() << endl;
		for (int s_index = 0; s_index < (int)this->loop_states.size(); s_index++) {
			save_file << this->loop_states[s_index] << endl;
		}
	}
}
