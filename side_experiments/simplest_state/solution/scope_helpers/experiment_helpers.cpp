#include "scope.h"

#include <iostream>

#include "globals.h"
#include "init_network.h"
#include "negate_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::experiment_start_activate(vector<double>& obs,
									  SolutionWrapper* wrapper) {
	for (int n_index = 0; n_index < (int)this->start_negate_networks.size(); n_index++) {
		this->start_negate_networks[n_index]->activate(wrapper->state);
		if (wrapper->run_type != RUN_TYPE_EXPLORE) {
			NegateNetworkHistory* negate_network_history = new NegateNetworkHistory(this->start_negate_networks[n_index]);
			this->start_negate_networks[n_index]->save(negate_network_history);
			wrapper->network_histories.push_back(negate_network_history);
		}

		if (wrapper->partial_state.size() > 0) {
			this->start_negate_networks[n_index]->activate(wrapper->partial_state);
			NegateNetworkHistory* negate_network_history = new NegateNetworkHistory(this->start_negate_networks[n_index]);
			this->start_negate_networks[n_index]->save(negate_network_history);
			wrapper->partial_network_histories.push_back(negate_network_history);
		}
	}

	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->start_init_network_scope_contexts[n_index],
									this->start_init_network_node_contexts[n_index])) {
			this->start_init_networks[n_index]->activate(wrapper->state,
														 obs);
			if (wrapper->run_type != RUN_TYPE_EXPLORE) {
				InitNetworkHistory* init_network_history = new InitNetworkHistory(this->start_init_networks[n_index]);
				this->start_init_networks[n_index]->save(init_network_history);
				wrapper->network_histories.push_back(init_network_history);
			}

			if (wrapper->partial_state.size() > 0) {
				uniform_int_distribution<int> add_noise_distribution(0, 9);
				if (add_noise_distribution(generator) == 0) {
					for (int i_index = 0; i_index < (int)this->start_init_networks[n_index]->init_states.size(); i_index++) {
						int state = this->start_init_networks[n_index]->init_states[i_index];
						normal_distribution<double> distribution(0.0, wrapper->solution->state_diffs[state]);
						wrapper->partial_state[state] += distribution(generator);
					}
				}

				this->start_init_networks[n_index]->activate(wrapper->partial_state,
															 obs);
				InitNetworkHistory* init_network_history = new InitNetworkHistory(this->start_init_networks[n_index]);
				this->start_init_networks[n_index]->save(init_network_history);
				wrapper->partial_network_histories.push_back(init_network_history);
			}
		}
	}

	if (this->dependencies.size() > 0) {
		ScopeHistory* scope_history = wrapper->scope_histories.back();
		scope_history->obs = obs;
	}
}
