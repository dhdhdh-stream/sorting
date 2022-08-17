#include "solution_node_if_end.h"

using namespace std;

void SolutionNodeIfEnd::reset() override {
	// do nothing
}

SolutionNode* SolutionNodeIfEnd::activate(Problem& problem,
										  double* state_vals,
										  bool* states_on,
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
}

void SolutionNodeIfEnd::increment(SolutionNode* explore_node,
								  int& explore_type,
								  bool* potential_states_on) override {
	increment_score_network();
}
