#include "start_node.h"

#include "abstract_experiment.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
