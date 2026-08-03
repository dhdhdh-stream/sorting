#include "scope.h"

#include <iostream>

#include "globals.h"
#include "init_network.h"
#include "obs_network.h"
#include "score_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::experiment_start_activate(vector<double>& obs,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	uniform_int_distribution<int> drop_distribution(0, 19);
	scope_history->is_drop = drop_distribution(generator) == 0;

	this->start_obs_network->activate(wrapper->states.back(),
									  obs);

	if (!scope_history->is_drop) {
		this->start_obs_network->activate(wrapper->partial_states.back(),
										  obs);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			scope_history->start_obs_network_history = new ObsNetworkHistory(this->start_obs_network);
			this->start_obs_network->save(scope_history->start_obs_network_history);
		}
	}

	if (wrapper->run_type != RUN_TYPE_EXPLORE) {
		scope_history->start_init_network_histories = vector<InitNetworkHistory*>(this->start_init_networks.size(), NULL);
	}
	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->start_init_network_scope_contexts[n_index],
									this->start_init_network_node_contexts[n_index])) {
			this->start_init_networks[n_index]->activate(wrapper->states.back(),
														 obs);

			if (!scope_history->is_drop) {
				this->start_init_networks[n_index]->activate(wrapper->partial_states.back(),
															 obs);
				if (wrapper->run_type != RUN_TYPE_EXPLORE) {
					scope_history->start_init_network_histories[n_index] = new InitNetworkHistory(this->start_init_networks[n_index]);
					this->start_init_networks[n_index]->save(scope_history->start_init_network_histories[n_index]);
				}
			}
		}
	}

	if (this->dependencies.size() > 0) {
		scope_history->state = wrapper->partial_states.back();
		scope_history->obs = obs;
	}
}
