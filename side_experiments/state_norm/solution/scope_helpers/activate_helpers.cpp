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
		int state_index = this->start_negate_networks[n_index]->state_index;
		wrapper->state_norms(state_index) = 0.0;
		wrapper->state(state_index) = 0.0;
	}

	wrapper->num_actions++;
	wrapper->state_norms = wrapper->state_norms.array() + 1.0;

	this->start_obs_network->activate(wrapper->state_norms,
									  wrapper->state,
									  obs);

	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->start_init_network_scope_contexts[n_index],
									this->start_init_network_node_contexts[n_index])) {
			this->start_init_networks[n_index]->activate(wrapper->state_norms,
														 wrapper->state,
														 obs);
		}
	}
}
