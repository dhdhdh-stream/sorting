#include "start_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::result_step(vector<double>& obs,
							int& action,
							bool& is_next,
							SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->result_scope_histories.back();

	scope_history->pre_obs_history = wrapper->problem->get_world();

	wrapper->result_node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->result_check_activate(
			this,
			false,
			wrapper);
	}
}
