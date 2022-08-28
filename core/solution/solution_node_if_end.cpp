#include "solution_node_if_end.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeIfEnd::SolutionNodeIfEnd(SolutionNode* parent,
									 int node_index) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_IF_END;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	int input_size = 1 + (int)this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

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

SolutionNodeIfEnd::SolutionNodeIfEnd(Solution* solution,
									 int node_index,
									 ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_IF_END;

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

SolutionNodeIfEnd::~SolutionNodeIfEnd() {
	delete this->score_network;
}

void SolutionNodeIfEnd::reset() {
	this->node_is_on = false;
}

void SolutionNodeIfEnd::add_potential_state(vector<int> potential_state_indexes,
											SolutionNode* explore_node) {
	add_potential_state_for_score_network(potential_state_indexes);

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->add_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeIfEnd::extend_with_potential_state(vector<int> potential_state_indexes,
													vector<int> new_state_indexes,
													SolutionNode* explore_node) {
	extend_state_for_score_network(potential_state_indexes,
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

void SolutionNodeIfEnd::delete_potential_state(vector<int> potential_state_indexes,
											   SolutionNode* explore_node) {
	delete_potential_state_for_score_network(potential_state_indexes);

	if (this == explore_node) {
		return;
	}
	if (this->next->node_type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->delete_potential_state(potential_state_indexes, explore_node);
}

SolutionNode* SolutionNodeIfEnd::activate(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  vector<SolutionNode*>& loop_scopes,
										  vector<int>& loop_scope_counts,
										  int& iter_explore_type,
										  SolutionNode*& iter_explore_node,
										  IterExplore*& iter_explore,
										  double& previous_predicted_score,
										  double* potential_state_vals,
										  bool* potential_states_on,
										  vector<NetworkHistory*>& network_historys,
										  vector<vector<double>>& guesses,
										  vector<int>& explore_decisions,
										  vector<double>& explore_diffs,
										  vector<bool>& explore_loop_decisions,
										  bool save_for_display,
										  std::ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

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
							available_state);
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
							available_state);
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
							available_state);
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
							available_state);
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
						available_state);
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
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_LOOP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_LOOP;
			}
		}
	}

	SolutionNode* explore_node = NULL;
	if (iter_explore_node == this) {
		explore_node = explore(problem,
							   state_vals,
							   states_on,
							   loop_scopes,
							   loop_scope_counts,
							   iter_explore_type,
							   iter_explore_node,
							   iter_explore,
							   previous_predicted_score,
							   potential_state_vals,
							   potential_states_on,
							   network_historys,
							   explore_decisions,
							   explore_diffs);
	}

	if (explore_node != NULL) {
		return explore_node;
	} else {
		return this->next;
	}
}

void SolutionNodeIfEnd::backprop(double score,
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
	explore_backprop(score,
					 misguess,
					 state_errors,
					 states_on,
					 iter_explore_node,
					 potential_state_errors,
					 potential_states_on,
					 network_historys,
					 explore_decisions,
					 explore_diffs);
}

void SolutionNodeIfEnd::save(ofstream& save_file) {
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

void SolutionNodeIfEnd::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		save_file << this->next->node_index << endl;
	}
}
