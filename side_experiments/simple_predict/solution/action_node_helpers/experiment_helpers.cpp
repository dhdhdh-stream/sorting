#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->num_actions++;
}

void ActionNode::experiment_step_callback(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories.push_back(history);

	this->obs_network->activate(wrapper->states.back(),
								obs);

	if (wrapper->iters_since_update >= UPDATE_NUM_ITERS) {
		history->obs = obs;
	}

	if (!this->is_generic) {
		wrapper->node_context.back() = this->next_node;

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			this->experiments[e_index]->experiment_check_activate(
				obs,
				wrapper);
		}
	}
}
