#include "action_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->num_actions++;

	for (int f_index = 0; f_index < (int)this->impacted_factors.size(); f_index++) {
		this->parent->invalidate_factor(wrapper->scope_histories.back(),
										this->impacted_factors[f_index]);
	}

	wrapper->node_context.back() = this->next_node;
}
