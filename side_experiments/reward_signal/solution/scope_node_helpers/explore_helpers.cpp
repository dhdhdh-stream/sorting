#include "scope_node.h"

#include <iostream>

#include "explore.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::explore_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);
	wrapper->explore_context.push_back(NULL);
}

void ScopeNode::explore_exit_step(SolutionWrapper* wrapper) {
	this->scope->back_activate(wrapper);

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->explore_context.pop_back();

	wrapper->node_context.back() = this->next_node;

	if (this->explore != NULL) {
		this->explore->check_activate(wrapper);
	}
}
