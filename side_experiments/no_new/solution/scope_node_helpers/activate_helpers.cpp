#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

void ScopeNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);

	wrapper->states.push_back(Eigen::VectorXf());
	wrapper->states.back().resize(this->scope->num_states);
	wrapper->states.back().setConstant(0.0);

	this->in_network->activate(wrapper->states[wrapper->states.size()-2],
							   wrapper->states.back());

	this->scope->start_activate(obs,
								wrapper);
}

void ScopeNode::exit_step(vector<double>& obs,
						  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories[wrapper->scope_histories.size() - 2];

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories.push_back(history);

	history->scope_history = wrapper->scope_histories.back();

	this->out_network->activate(wrapper->states.back(),
								wrapper->states[wrapper->states.size()-2]);

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();

	wrapper->states.pop_back();

	wrapper->node_context.back() = this->next_node;
}
