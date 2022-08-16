#include "solution_node_loop_start.h"

using namespace std;

void SolutionNodeLoopStart::reset() override {
	this->states_on->clear();
}

SolutionNode* SolutionNodeLoopStart::tune(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  vector<NetworkHistory*>& network_historys) override {
	tune_score_network(problem,
					   state_vals,
					   states_on,
					   network_historys);
	
	for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
		state_vals[this->scope_states_on[o_index]] = 0.0;
		states_on[this->scope_states_on[o_index]] = true;
	}

	return this->next;
}

void SolutionNodeLoopStart::tune_update(double score,
					 					double* state_errors,
					 					bool* states_on,
					 					vector<NetworkHistory*>& network_historys) override {
	for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
		states_on[this->scope_states_on[o_index]] = false;
	}

	tune_update_score_network(score,
							  state_errors,
							  states_on,
							  network_historys);
}

void SolutionNodeLoopStart::increment() override {
	increment_score_network();
}
