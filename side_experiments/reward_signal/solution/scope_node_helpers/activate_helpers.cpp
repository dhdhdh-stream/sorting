#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	wrapper->scope_histories.back()->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);
}

void ScopeNode::exit_step(SolutionWrapper* wrapper) {
	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();

	for (int f_index = 0; f_index < (int)this->impacted_factors.size(); f_index++) {
		this->parent->invalidate_factor(wrapper->scope_histories.back(),
										this->impacted_factors[f_index]);
	}

	wrapper->node_context.back() = this->next_node;
}
