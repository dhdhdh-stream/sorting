#include "start_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "long_network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void StartNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	if (this->parent->pre_network != NULL) {
		this->parent->pre_network->activate(obs);
		wrapper->curr_impact += this->parent->pre_network->output->acti_vals[0];
	}

	scope_history->pre_obs_history = obs;
	scope_history->pre_impact = wrapper->curr_impact;

	StartNodeHistory* history = new StartNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
