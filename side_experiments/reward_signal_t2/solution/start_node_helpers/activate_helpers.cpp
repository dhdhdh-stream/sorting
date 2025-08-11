#include "start_node.h"

#include "solution_wrapper.h"

using namespace std;

void StartNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	wrapper->node_context.back() = this->next_node;
}
