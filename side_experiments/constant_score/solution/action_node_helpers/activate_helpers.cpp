#include "action_node.h"

#include <iostream>

#include "action_network.h"
#include "globals.h"
#include "init_network.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "score_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_DAMAGE) {
		uniform_int_distribution<int> damage_distribution(0, 19);
		if (damage_distribution(generator) == 0) {
			wrapper->node_context.back() = this->next_node;
			return;
		}
	}

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

	this->action_network->activate(wrapper->state);

	this->obs_network->activate(wrapper->state,
								obs);

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
		}
	}

	wrapper->node_context.back() = this->next_node;

	// temp
	this->score_network->activate(wrapper->state);
	cout << "this->score_network->output->acti_vals(0): " << this->score_network->output->acti_vals(0) << endl;
}
