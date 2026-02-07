#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::result_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->result_num_actions++;

	wrapper->result_node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->result_check_activate(
			this,
			false,
			wrapper);
	}
}
