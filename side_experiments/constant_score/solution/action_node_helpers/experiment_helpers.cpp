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

void ActionNode::experiment_step_callback(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	uniform_int_distribution<int> drop_distribution(0, 9);
	bool is_drop = drop_distribution(generator) == 0;

	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	this->action_network->activate(wrapper->state);

	if (!is_drop) {
		this->action_network->activate(wrapper->partial_state);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			ActionNetworkHistory* action_network_history = new ActionNetworkHistory(this->action_network);
			this->action_network->save(action_network_history);
			wrapper->partial_network_histories.push_back(action_network_history);
		}
	}

	this->obs_network->activate(wrapper->state,
								obs);

	if (!is_drop) {
		this->obs_network->activate(wrapper->partial_state,
									obs);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			ObsNetworkHistory* obs_network_history = new ObsNetworkHistory(this->obs_network);
			this->obs_network->save(obs_network_history);
			wrapper->partial_network_histories.push_back(obs_network_history);
		}
	}

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);

			if (!is_drop) {
				this->init_networks[n_index]->activate(wrapper->partial_state,
													   obs);
				if (wrapper->run_type != RUN_TYPE_EXPLORE) {
					InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
					this->init_networks[n_index]->save(init_network_history);
					wrapper->partial_network_histories.push_back(init_network_history);
				}
			}
		}
	}

	if (this->dependencies.size() > 0) {
		history->state = wrapper->partial_state;
		history->obs = obs;
	}

	wrapper->node_context.back() = this->next_node;

	if (wrapper->run_type != RUN_TYPE_EXPLORE) {
		if (!is_drop) {
			this->score_network->activate(wrapper->partial_state);
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->score_network);
			this->score_network->save(score_network_history);
			wrapper->partial_network_histories.push_back(score_network_history);
		}
	}

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			obs,
			wrapper);
	}
}
