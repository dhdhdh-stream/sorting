#include "obs_node.h"

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::result_step(vector<double>& obs,
						  int& action,
						  bool& is_next,
						  SolutionWrapper* wrapper) {
	wrapper->result_node_context.back() = this->next_node;
}
