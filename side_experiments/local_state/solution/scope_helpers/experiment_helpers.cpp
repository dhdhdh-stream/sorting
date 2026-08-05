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

	this->start_obs_network->activate(wrapper->states.back(),
									  obs);

	scope_history->init_is_match = vector<bool>(this->start_init_networks.size());
	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->start_init_network_scope_contexts[n_index],
									this->start_init_network_node_contexts[n_index])) {
			this->start_init_networks[n_index]->activate(wrapper->states.back(),
														 obs);

			scope_history->init_is_match[n_index] = true;
		} else {
			scope_history->init_is_match[n_index] = false;
		}
	}

	if (this->dependencies.size() > 0) {
		scope_history->state = wrapper->states.back();
		scope_history->obs = obs;
	}
}
