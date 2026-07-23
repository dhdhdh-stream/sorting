#include "scope.h"

#include <iostream>

#include "globals.h"
#include "init_network.h"
#include "negate_network.h"
#include "obs_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::experiment_start_activate(vector<double>& obs,
									  SolutionWrapper* wrapper) {
	uniform_int_distribution<int> partial_distribution(0, 9);

	for (int n_index = 0; n_index < (int)this->start_negate_networks.size(); n_index++) {
		this->start_negate_networks[n_index]->activate(wrapper->state);

		this->start_negate_networks[n_index]->activate(wrapper->partial_state);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			NegateNetworkHistory* negate_network_history = new NegateNetworkHistory(this->start_negate_networks[n_index]);
			wrapper->partial_network_histories.push_back(negate_network_history);
		}
	}

	this->start_obs_network->activate(wrapper->state,
									  obs);

	if (partial_distribution(generator) != 0) {
		this->start_obs_network->activate(wrapper->partial_state,
										  obs);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			ObsNetworkHistory* obs_network_history = new ObsNetworkHistory(this->start_obs_network);
			this->start_obs_network->save(obs_network_history);
			wrapper->partial_network_histories.push_back(obs_network_history);
		}
	}

	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->start_init_network_scope_contexts[n_index],
									this->start_init_network_node_contexts[n_index])) {
			this->start_init_networks[n_index]->activate(wrapper->state,
														 obs);

			if (partial_distribution(generator) != 0) {
				this->start_init_networks[n_index]->activate(wrapper->partial_state,
															 obs);
				if (wrapper->run_type != RUN_TYPE_EXPLORE) {
					InitNetworkHistory* init_network_history = new InitNetworkHistory(this->start_init_networks[n_index]);
					this->start_init_networks[n_index]->save(init_network_history);
					wrapper->partial_network_histories.push_back(init_network_history);
				}
			}
		}
	}

	if (this->dependencies.size() > 0) {
		ScopeHistory* scope_history = wrapper->scope_histories.back();
		scope_history->state = wrapper->partial_state;
		scope_history->obs = obs;
	}
}
