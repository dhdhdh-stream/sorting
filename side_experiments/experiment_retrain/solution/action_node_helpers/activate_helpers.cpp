#include "action_node.h"

#include <iostream>

#include "init_network.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->num_actions++;
}

void ActionNode::step_callback(vector<double>& obs,
							   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	this->obs_network->activate(wrapper->state,
								obs);

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		this->init_networks[n_index]->activate(wrapper->state,
											   obs);
	}

	wrapper->node_context.back() = this->next_node;
}
