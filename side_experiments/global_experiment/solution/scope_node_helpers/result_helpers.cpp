#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::result_step(vector<double>& obs,
							int& action,
							bool& is_next,
							SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->result_scope_histories.push_back(inner_scope_history);
	wrapper->result_node_context.push_back(this->scope->nodes[0]);
	wrapper->result_experiment_context.push_back(NULL);
}

void ScopeNode::result_exit_step(SolutionWrapper* wrapper) {
	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	wrapper->result_node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->result_check_activate(
			this,
			false,
			wrapper);
	}
}
