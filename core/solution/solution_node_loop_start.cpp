#include "solution_node_loop_start.h"

using namespace std;

void SolutionNodeLoopStart::reset() override {
	this->states_on->clear();
}

SolutionNode* SolutionNodeLoopStart::activate(Problem& problem,
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
		if (randuni() < (2.0/this->average_future_nodes)) {
			if (rand()%2 == 0) {
				explore_node = this;
				explore_type = EXPLORE_TYPE_STATE;
			} else {
				explore_node = this;
				explore_type = EXPLORE_TYPE_PATH;
			}
		}
	}

	if (explore_node == this) {
		if (explore_type == EXPLORE_TYPE_STATE) {
			// explore state
		}
		else if (explore_type == EXPLORE_TYPE_PATH) {
			if (score < this->average) {
				// explore path
			}
		}
	}

	for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
		state_vals[this->scope_states_on[o_index]] = 0.0;
		states_on[this->scope_states_on[o_index]] = true;
	}

	return this->next;
}

void SolutionNodeLoopStart::backprop(double score,
					 				 SolutionNode* explore_node,
					 				 int& explore_type,
					 				 double* potential_state_errors,
					 				 bool* potential_states_on,
					 				 vector<NetworkHistory*>& network_historys) override {
	backprop_score_network(score,
						   potential_state_errors,
						   network_historys);

	// update average, misguess, etc.
}
