#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_network.h"
#include "globals.h"
#include "init_network.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->run_num_actions++;
}

void ActionNode::experiment_step_callback(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories.push_back(history);

	history->obs = obs;

	this->action_network->activate(wrapper->states.back());

	this->obs_network->activate(wrapper->states.back(),
								obs);

	history->init_is_match = vector<bool>(this->init_networks.size());
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->states.back(),
												   obs);

			history->init_is_match[n_index] = true;
		} else {
			history->init_is_match[n_index] = false;
		}
	}

	if (!this->is_generic) {
		if (this->dependencies.size() > 0) {
			history->state = wrapper->states.back();
		}

		wrapper->node_context.back() = this->next_node;

		if (this->experiment != NULL) {
			if (this->experiment->diversity_index == -1
					|| this->experiment->diversity_index == wrapper->diversity_index) {
				this->experiment->experiment_check_activate(
					obs,
					wrapper);
			}
		}
	}
}
