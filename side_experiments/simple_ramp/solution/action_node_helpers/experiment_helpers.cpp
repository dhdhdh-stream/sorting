#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->num_actions++;

	wrapper->node_context.back() = this->next_node;
}
