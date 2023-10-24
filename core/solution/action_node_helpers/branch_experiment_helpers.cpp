#include "action_node.h"

#include <iostream>

#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::branch_experiment_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		ActionNodeHistory* history) {
	problem.perform_action(this->action);
	history->obs_snapshot = problem.get_observation();

	for (int n_index = 0; n_index < (int)this->experiment_hook_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_state_scope_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->experiment_hook_state_node_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<int, StateStatus>::iterator it = context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()]
				.experiment_state_vals.find(this->experiment_hook_state_indexes[n_index]);
			if (it == context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()].experiment_state_vals.end()) {
				it = context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()].experiment_state_vals
					.insert({this->experiment_hook_state_indexes[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_state_defs[n_index]->networks[this->experiment_hook_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}
}
