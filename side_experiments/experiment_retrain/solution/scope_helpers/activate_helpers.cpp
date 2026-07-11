#include "scope.h"

#include <iostream>

#include "init_network.h"
#include "negate_network.h"
#include "obs_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::start_activate(vector<double>& obs,
						   SolutionWrapper* wrapper) {
	for (int n_index = 0; n_index < (int)this->start_negate_networks.size(); n_index++) {
		this->start_negate_networks[n_index]->activate(wrapper->state);
	}

	this->start_obs_network->activate(wrapper->state,
									  obs);

	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->start_init_network_scope_contexts[n_index],
									this->start_init_network_node_contexts[n_index])) {
			this->start_init_networks[n_index]->activate(wrapper->state,
														 obs);
		}
	}
}
