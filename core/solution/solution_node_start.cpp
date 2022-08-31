#include "solution_node_start.h"

#include <iostream>
#include <random>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeStart::SolutionNodeStart(Solution* solution) {
	this->solution = solution;

	this->node_index = 0;
	this->node_type = NODE_TYPE_START;

	this->network_inputs_state_indexes.push_back(0);
	this->network_inputs_state_indexes.push_back(1);

	this->score_network = new Network(3, 12, 1);
	this->certainty_network = new Network(3, 12, 1);

	this->node_weight = 0.0;

	this->start_states.push_back(0);
	this->start_states.push_back(1);

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_score_network = NULL;
	this->explore_jump_certainty_network = NULL;
	this->explore_halt_score_network = NULL;
	this->explore_halt_certainty_network = NULL;
	this->explore_no_halt_score_network = NULL;
	this->explore_no_halt_certainty_network = NULL;

	this->node_is_on = true;
}

SolutionNodeStart::SolutionNodeStart(Solution* solution,
									 int node_index,
									 ifstream& save_file) {
	this->solution = solution;

	this->node_index = 0;
	this->node_type = NODE_TYPE_START;

	load_score_network(save_file);

	this->start_states.push_back(0);
	this->start_states.push_back(1);

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_score_network = NULL;
	this->explore_jump_certainty_network = NULL;
	this->explore_halt_score_network = NULL;
	this->explore_halt_certainty_network = NULL;
	this->explore_no_halt_score_network = NULL;
	this->explore_no_halt_certainty_network = NULL;

	this->node_is_on = true;
}

SolutionNodeStart::~SolutionNodeStart() {
	delete this->score_network;
	delete this->certainty_network;
}

void SolutionNodeStart::reset() {
	// do nothing
}

void SolutionNodeStart::add_potential_state(vector<int> potential_state_indexes,
											SolutionNode* explore_node) {
	// should not happen
}

void SolutionNodeStart::extend_with_potential_state(vector<int> potential_state_indexes,
													vector<int> new_state_indexes,
													SolutionNode* explore_node) {
	// should not happen
}

void SolutionNodeStart::delete_potential_state(vector<int> potential_state_indexes,
											   SolutionNode* explore_node) {
	// should not happen
}

void SolutionNodeStart::clear_potential_state() {
	// do nothing
}

SolutionNode* SolutionNodeStart::activate(Problem& problem,
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

	for (int s_index = 0; s_index < (int)this->start_states.size(); s_index++) {
		state_vals[this->start_states[s_index]] = 0.0;
		states_on[this->start_states[s_index]] = true;
	}

	loop_scopes.push_back(this);
	loop_scope_counts.push_back(1);

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
				if (this->next->node_type != NODE_TYPE_END) {
					find_potential_jumps(this->next,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);
				}

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

	if (explore_node != NULL) {
		return explore_node;
	} else {
		return this->next;
	}
}

void SolutionNodeStart::backprop(double score,
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

void SolutionNodeStart::save(ofstream& save_file) {
	save_score_network(save_file);
}

void SolutionNodeStart::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		save_file << this->next->node_index << endl;
	}
}
