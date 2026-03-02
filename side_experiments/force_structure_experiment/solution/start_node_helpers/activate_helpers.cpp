#include "start_node.h"

#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	StartNodeHistory* history = new StartNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;
}

void StartNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 vector<ScopeHistory*>& scope_histories,
					 vector<AbstractNode*>& node_context,
					 int& num_actions) {
	ScopeHistory* scope_history = scope_histories.back();

	StartNodeHistory* history = new StartNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	node_context.back() = this->next_node;
}
