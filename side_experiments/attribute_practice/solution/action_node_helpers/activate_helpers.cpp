#include "action_node.h"

#include <iostream>

#include "decision_tree.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
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

		double val = it->second->activate(obs);
		wrapper->curr_impact += val;

		// temp
		cout << "this->action: " << this->action << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << obs[5 * y_index + x_index] << " ";
			}
			cout << endl;
		}
		cout << "val: " << val << endl;
		cout << "history->curr_impact: " << history->curr_impact << endl;
		cout << endl;
	}

	wrapper->node_context.back() = this->next_node;
}
