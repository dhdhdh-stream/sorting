#include "action_node.h"

#include <iostream>

#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::branch_experiment_train_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	problem.perform_action(this->action);
	double obs_snapshot = problem.get_observation();

	for (int n_index = 0; n_index < (int)this->experiment_hook_score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_score_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_score_state_scope_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->experiment_hook_score_state_node_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()]
				.experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
			if (it == context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals.end()) {
				it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals
					.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
			state_network->activate(obs_snapshot,
									it->second);
		}
	}

	for (int h_index = 0; h_index < (int)this->test_hook_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != context[context.size()-this->test_hook_scope_contexts[h_index].size()+c_index].scope_id
						|| this->test_hook_node_contexts[h_index][c_index] != context[context.size()-this->test_hook_scope_contexts[h_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			context[context.size()-this->test_hook_scope_contexts[h_index].size()]
				.scope_history->test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
			context[context.size()-this->test_hook_scope_contexts[h_index].size()]
				.scope_history->test_obs_vals.push_back(obs_snapshot);
		}
	}
}

void ActionNode::branch_experiment_simple_activate(
		Problem& problem) {
	problem.perform_action(this->action);
}
