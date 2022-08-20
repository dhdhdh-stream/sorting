#include "solution_node_if_end.h"

using namespace std;

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
	} else if (this->next->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* next_loop_start = (SolutionNodeLoopStart*)this->next;
		if (next_loop_start->loop_in == this) {
			return;
		}
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
	} else if (this->next->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* next_loop_start = (SolutionNodeLoopStart*)this->next;
		if (next_loop_start->loop_in == this) {
			return;
		}
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
	} else if (this->next->type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* next_loop_start = (SolutionNodeLoopStart*)this->next;
		if (next_loop_start->loop_in == this) {
			return;
		}
	}
	this->next->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeIfEnd::clear_potential_state() {
	clear_potential_state_for_score_network();
}

SolutionNode* SolutionNodeIfEnd::activate(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  vector<SolutionNode*>& loop_scopes,
										  vector<int>& loop_scope_counts,
										  int visited_count,
										  SolutionNode* explore_node,
										  int& explore_type,
										  double* potential_state_vals,
										  bool* potential_states_on,
										  vector<NetworkHistory*>& network_historys) override {
	double score = activate_score_network(problem,
										  state_vals,
										  states_on,
										  explore_type,
										  potential_state_vals,
										  potential_states_on,
										  network_historys);

	if (visited_count == 0 && explore_node == NULL) {
		if (randuni() < 1.0/this->average_future_nodes) {
			explore_node = this;
			explore_type = EXPLORE_TYPE_PATH;
		}
	}

	if (explore_node == this) {
		if (score < this->average) {
			// explore path
		}
	}

	return this->next;
}

void SolutionNodeIfEnd::backprop(double score,
								 SolutionNode* explore_node,
								 int& explore_type,
								 double* potential_state_errors,
								 bool* potential_states_on,
								 std::vector<NetworkHistory*>& network_historys) override {
	backprop_score_network(score,
						   potential_state_errors,
						   network_historys);

	// clear state errors
}
