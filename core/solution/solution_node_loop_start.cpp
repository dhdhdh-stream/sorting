#include "solution_node_loop_start.h"

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
}

SolutionNodeLoopStart::SolutionNodeLoopStart(SolutionNode* parent,
											 int node_index) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_LOOP_START;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	int input_size = 1 + this->network_inputs_state_indexes.size();
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
}

SolutionNodeLoopStart::~SolutionNodeLoopStart() {
	delete this->score_network;
}

void SolutionNodeLoopStart::reset() override {
	this->scope_states_on.clear();
	this->scope_potential_states.clear();
}

void SolutionNodeLoopStart::add_potential_state(vector<int> potential_state_indexes,
												SolutionNode* scope) override {
	// loop start can use state from its scope
	add_potential_state_for_score_network(potential_state_indexes);

	this->next->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopStart::extend_with_potential_state(vector<int> potential_state_indexes,
														vector<int> new_state_indexes,
														SolutionNode* scope) override {
	// loop start can use state from its scope
	extend_state_for_score_network(potential_state_indexes);

	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											scope);
}

void SolutionNodeLoopStart::reset_potential_state(vector<int> potential_state_indexes,
												  SolutionNode* scope) override {
	// loop start can use state from its scope
	reset_potential_state_for_score_network(potential_state_indexes);

	this->next->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopStart::clear_potential_state() {
	clear_potential_state_for_score_network();
}

SolutionNode* SolutionNodeLoopStart::activate(Problem& problem,
											  double* state_vals,
											  bool* states_on,
											  vector<SolutionNode*>& loop_scopes,
											  vector<int>& loop_scope_counts,
											  bool is_first_time,
											  int& iter_explore_type,
											  SolutionNode* iter_explore_node,
											  double* potential_state_vals,
											  bool* potential_states_on,
											  vector<NetworkHistory*>& network_historys,
											  vector<double>& guesses,
											  vector<int>& explore_decisions,
											  vector<bool>& explore_loop_decisions) override {
	if (iter_explore_type == EXPLORE_TYPE_NONE && is_first_time) {
		if (randuni() < (2.0/this->average_unique_future_nodes)) {
			if (rand()%2 == 0) {
				if (!this->has_explored_state) {
					if (this->explore_state_state == EXPLORE_STATE_STATE_LEARN) {
						int rand_states = 1 + rand()%5;
						for (int s_index = 0; s_index < rand_states; s_index++) {
							potential_states_on[this->scope_potential_states[s_index]] = true;
						}

						iter_explore_node = this;
						iter_explore_type = EXPLORE_TYPE_LEARN_STATE;
					} else if (this->explore_state_state == EXPLORE_STATE_STATE_MEASURE) {
						int rand_states = rand()%6;
						for (int s_index = 0; s_index < rand_states; s_index++) {
							potential_states_on[this->scope_potential_states[s_index]] = true;
						}

						iter_explore_node = this;
						iter_explore_type = EXPLORE_TYPE_MEASURE_STATE;
					}
				}
			} else {
				if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
					geometric_distribution<int> seq_length_dist(0.0, 0.2);
					int seq_length;

					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					potential_inclusive_jump_ends.push_back(this);
					potential_non_inclusive_jump_ends.push_back(this->next);
					find_potential_jumps(this->next,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);

					int random_index = rand()%potential_inclusive_jump_ends.size();

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

					this->path_explore_type = PATH_EXPLORE_TYPE_JUMP;

					try_path.clear();

					normal_distribution<double> write_val_dist(0.0, 2.0);
					for (int i = 0; i < seq_length; i++) {
						Action a(write_val_dist(generator), rand()%3);
						try_path.push_back(a);
					}

					if (rand()%2 == 0) {
						this->path_target_type = PATH_TARGET_TYPE_GOOD;
					} else {
						this->path_target_type = PATH_TARGET_TYPE_BAD;
					}

					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_EXPLORE;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_LEARN_PATH;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_MEASURE_PATH;
				}
			}
		}
	}

	double score = activate_score_network_helper(problem,
												 state_vals,
												 states_on,
												 iter_explore_type,
												 iter_explore_node,
												 potential_state_errors,
												 potential_states_on,
												 network_historys,
												 guesses);

	if (loop_scopes.back() == this) {
		loop_scope_counts.back()++;
	} else {
		for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
			state_vals[this->scope_states_on[o_index]] = 0.0;
			states_on[this->scope_states_on[o_index]] = true;
		}

		if (iter_explore_node == this) {
			if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE
					|| iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
				for (int s_index = 0; s_index < (int)this->scope_potential_states.size(); s_index++) {
					potential_state_vals[this->scope_potential_states[s_index]] = 0.0;
				}
			}
		}

		loop_scopes.push_back(this);
		loop_scope_counts.push_back(1);
	}

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
							   potential_state_errors,
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

void SolutionNodeLoopStart::backprop(double score,
									 double misguess,
									 double* state_errors,
									 bool* states_on,
									 int& iter_explore_type,
									 SolutionNode* iter_explore_node,
									 double* potential_state_errors,
									 bool* potential_states_on,
									 vector<NetworkHistory*>& network_historys,
									 vector<int>& explore_decisions,
									 vector<bool>& explore_loop_decisions) override {
	backprop_explore_and_score_network_helper(score,
											  misguess,
											  state_errors,
											  states_on,
											  iter_explore_type,
											  iter_explore_node,
											  potential_state_errors,
											  potential_states_on,
											  network_historys,
											  explore_decisions);

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
	}
}
