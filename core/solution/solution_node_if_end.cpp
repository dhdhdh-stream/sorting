#include "solution_node_if_end.h"

using namespace std;

SolutionNodeIfEnd::SolutionNodeIfEnd(SolutionNode* parent,
									 int node_index) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_IF_END;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	int input_size = 1 + this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;
}

SolutionNodeIfEnd::~SolutionNodeIfEnd() {
	delete this->score_network;
}

void SolutionNodeIfEnd::reset() override {
	// do nothing
}

void SolutionNodeIfEnd::add_potential_state(vector<int> potential_state_indexes,
											SolutionNode* scope) override {
	if (this->start == scope) {
		return;
	}

	add_potential_state_for_score_network(potential_state_indexes);

	if (this->next->type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeIfEnd::extend_with_potential_state(vector<int> potential_state_indexes,
													vector<int> new_state_indexes,
													SolutionNode* scope) override {
	if (this->start == scope) {
		return;
	}

	extend_state_for_score_network(potential_state_indexes);

	if (this->next->type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											scope);
}

void SolutionNodeIfEnd::reset_potential_state(vector<int> potential_state_indexes,
											  SolutionNode* scope) override {
	if (this->start == scope) {
		return;
	}

	reset_potential_state_for_score_network(potential_state_indexes);

	if (this->next->type == NODE_TYPE_IF_END) {
		return;
	}
	this->next->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeIfEnd::clear_potential_state() override {
	clear_potential_state_for_score_network();
}

SolutionNode* SolutionNodeIfEnd::activate(Problem& problem,
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
	if (visited_count == 0 && explore_node == NULL) {
		if (randuni() < (1.0/this->average_unique_future_nodes)) {
			if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
				int rand_index = rand()%3;
				if (rand_index == 0) {
					SolutionNode* inclusive_end;
					SolutionNode* non_inclusive_end;
					find_scope_end(this, inclusive_end, non_inclusive_end);
					
					geometric_distribution<int> seq_length_dist(0.0, 0.2);
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
					int random_index = rand()%potential_inclusive_jump_ends.size();

					geometric_distribution<int> seq_length_dist(0.0, 0.2);
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
					int random_index = rand()%potential_non_inclusive_loop_starts.size();

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
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_PATH;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_PATH;
			}
		}
	}

	for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
		states_on[this->start->scope_states_on[o_index]] = false;
	}
	// let scope start handle potentials differently for now

	double score = activate_score_network_helper(problem,
												 state_vals,
												 states_on,
												 iter_explore_type,
												 iter_explore_node,
												 potential_state_errors,
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

void SolutionNodeIfEnd::backprop(double score,
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
		for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
			states_on[this->start->scope_states_on[s_index]] = true;
			state_errors[this->start->scope_states_on[s_index]] = 0.0;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
			states_on[this->start->scope_states_on[s_index]] = true;
			state_errors[this->start->scope_states_on[s_index]] = 0.0;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		for (int s_index = 0; s_index < (int)this->start->scope_states_on.size(); s_index++) {
			states_on[this->start->scope_states_on[s_index]] = true;
			state_errors[this->start->scope_states_on[s_index]] = 0.0;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
	}
}
