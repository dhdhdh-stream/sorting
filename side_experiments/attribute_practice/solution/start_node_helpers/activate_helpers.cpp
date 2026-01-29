#include "start_node.h"

#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	// if (this->parent->pre_network != NULL) {
	// 	this->parent->pre_network->activate(obs);
	// 	wrapper->curr_impact += this->parent->pre_network->output->acti_vals[0];
	// }

	StartNodeHistory* history = new StartNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;
}
