#include "scope_node.h"

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

void ScopeNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);

	this->scope->experiment_start_activate(obs,
										   wrapper);
}

void ScopeNode::experiment_exit_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories[wrapper->scope_histories.size() - 2];

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->scope_history = wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

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

			if (wrapper->partial_state.size() > 0) {
				uniform_int_distribution<int> add_noise_distribution(0, 9);
				if (add_noise_distribution(generator) == 0) {
					for (int i_index = 0; i_index < (int)this->init_networks[n_index]->init_states.size(); i_index++) {
						int state = this->init_networks[n_index]->init_states[i_index];
						normal_distribution<double> distribution(0.0, wrapper->solution->state_diffs[state]);
						wrapper->partial_state[state] += distribution(generator);
					}
				}

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
