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
	if (!this->is_generic) {
		if (wrapper->run_type == RUN_TYPE_DAMAGE) {
			uniform_int_distribution<int> damage_distribution(0, 19);
			if (damage_distribution(generator) == 0) {
				wrapper->node_context.back() = this->next_node;
				return;
			}
		}
	}

	action = this->action;
	is_next = true;

	wrapper->run_num_actions++;
}

void ActionNode::experiment_step_callback(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	uniform_int_distribution<int> drop_distribution(0, 19);
	if (this->is_generic) {
		history->is_drop = false;
	} else {
		history->is_drop = drop_distribution(generator) == 0;
	}

	this->action_network->activate(wrapper->states.back());

	if (!history->is_drop) {
		this->action_network->activate(wrapper->partial_states.back());
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			history->action_network_history = new ActionNetworkHistory(this->action_network);
			this->action_network->save(history->action_network_history);
		}
	}

	this->obs_network->activate(wrapper->states.back(),
								obs);

	if (!history->is_drop) {
		this->obs_network->activate(wrapper->partial_states.back(),
									obs);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			history->obs_network_history = new ObsNetworkHistory(this->obs_network);
			this->obs_network->save(history->obs_network_history);
		}
	}

	if (wrapper->run_type != RUN_TYPE_EXPLORE) {
		history->init_network_histories = vector<InitNetworkHistory*>(this->init_networks.size(), NULL);
	}
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->states.back(),
												   obs);

			if (!history->is_drop) {
				this->init_networks[n_index]->activate(wrapper->partial_states.back(),
													   obs);
				if (wrapper->run_type != RUN_TYPE_EXPLORE) {
					history->init_network_histories[n_index] = new InitNetworkHistory(this->init_networks[n_index]);
					this->init_networks[n_index]->save(history->init_network_histories[n_index]);
				}
			}
		}
	}

	if (!this->is_generic) {
		if (this->dependencies.size() > 0) {
			history->state = wrapper->partial_states.back();
			history->obs = obs;
		}

		wrapper->node_context.back() = this->next_node;

		if (this->experiment != NULL) {
			this->experiment->experiment_check_activate(
				obs,
				wrapper);
		}
	}
}
