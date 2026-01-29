#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "decision_tree.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	action = this->action;
	is_next = true;

	wrapper->num_actions++;

	// temp
	map<int, DecisionTree*>::iterator it = wrapper->solution->action_impact_networks.find(this->action);
	if (it != wrapper->solution->action_impact_networks.end()) {
		history->obs_history = obs;
		history->curr_impact = wrapper->curr_impact;

		wrapper->curr_impact += it->second->activate(obs);
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
