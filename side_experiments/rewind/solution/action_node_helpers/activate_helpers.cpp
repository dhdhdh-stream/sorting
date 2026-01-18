#include "action_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	action = this->action;
	is_next = true;

	wrapper->num_actions++;

	wrapper->node_context.back() = this->next_node;
}

void ActionNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  vector<ScopeHistory*>& scope_histories,
					  vector<AbstractNode*>& node_context,
					  int& num_actions) {
	ScopeHistory* scope_history = scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	action = this->action;
	is_next = true;

	num_actions++;

	node_context.back() = this->next_node;
}
