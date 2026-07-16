#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "init_network.h"
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
	if (wrapper->run_type == RUN_TYPE_EXISTING) {
		uniform_int_distribution<int> add_noise_distribution(0, 9);
		if (add_noise_distribution(generator) == 0) {
			for (int s_index = 0; s_index < wrapper->solution->num_states; s_index++) {
				normal_distribution<double> distribution(0.0, wrapper->solution->state_diffs[s_index]);
				wrapper->partial_state[s_index] += distribution(generator);
			}
		}
	}

	action = this->action;
	is_next = true;

	wrapper->num_actions++;
}

void ActionNode::experiment_step_callback(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
			if (wrapper->run_type != RUN_TYPE_EXPLORE) {
				InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
				this->init_networks[n_index]->save(init_network_history);
				wrapper->network_histories.push_back(init_network_history);
			}

			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				this->prev_init_networks[n_index]->activate(wrapper->prev_state,
															obs);
			}

			if (wrapper->run_type == RUN_TYPE_EXISTING) {
				this->init_networks[n_index]->activate(wrapper->partial_state,
													   obs);
				InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
				this->init_networks[n_index]->save(init_network_history);
				wrapper->partial_network_histories.push_back(init_network_history);
			}
		}
	}

	if (this->dependencies.size() > 0) {
		history->obs = obs;
	}

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->experiment_check_activate(
			obs,
			wrapper);
	}
}
