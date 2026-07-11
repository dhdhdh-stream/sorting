#include "noop_node.h"

#include <iostream>

#include "init_network.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void NoopNode::verify_step(vector<double>& obs,
						   int& action,
						   bool& is_next,
						   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	NoopNodeHistory* history = new NoopNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
		}
	}

	// temp
	if (wrapper->starting_run_seed == 131) {
		cout << "this->id: " << this->id << endl;
		cout << "wrapper->state:";
		for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
			cout << " " << wrapper->state[s_index];
		}
		cout << endl;
		cout << "obs:";
		for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
			cout << " " << obs[o_index];
		}
		cout << endl;
	}

	wrapper->node_context.back() = this->next_node;
}
