#include "start_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	wrapper->node_context.back() = this->next_node;
}
