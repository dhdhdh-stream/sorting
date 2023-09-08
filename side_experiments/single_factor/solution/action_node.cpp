#include "action_node.h"

using namespace std;



void ActionNode::activate(vector<double>& flat_vals,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  vector<vector<AbstractNodeHistory*>>& node_histories) {
	if (run_helper.phase == EXPLORE_PHASE_NONE
			|| run_helper.phase == EXPLORE_PHASE_EXPLORE) {
		ActionNodeHistory* action_node_history = new ActionNodeHistory(this);
		action_node_history->obs_snapshot = flat_vals[0];
		node_histories.back().push_back(action_node_history);
	}

	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		if (context.back().state_weights[this->state_network_indexes[n_index]] != 0.0) {
			this->state_networks[n_index]->activate(flat_vals[0],
													context.back().state_vals->at(this->state_network_indexes[n_index]));
		}
	}

	if (run_helper.phase == EXPLORE_PHASE_EXPERIMENT) {
		Explore* explore = run_helper.explore_history->explore;

		for (int s_index = 0; s_index < (int)this->experiment_hook_state_indexes.size(); s_index++) {
			bool matches_context = true;
			if (this->experiment_hook_scope_contexts[s_index].size() > context.size()) {
				matches_context = false;
			} else {
				for (int c_index = 0; c_index < (int)this->experiment_hook_scope_contexts[s_index].size()-1; c_index++) {
					if (this->experiment_hook_scope_contexts[s_index][c_index] != context[context.size() - this->experiment_hook_scope_contexts[s_index].size() + c_index].scope_id
							|| this->experiment_hook_node_contexts[s_index][c_index] != context[context.size() - this->experiment_hook_scope_contexts[s_index].size() + c_index].node_id) {
						matches_context = false;
						break;
					}
				}
			}

			if (matches_context) {
				StateNetwork* state_network = explore->state_networks[this->experiment_hook_state_indexes[s_index]][this->experiment_hook_network_indexes[s_index]];
				state_network->activate(flat_vals[0],
										run_helper.explore_history->state_vals[this->experiment_hook_state_indexes[s_index]]);
			}
		}

		if (this->experiment_hook_test_index != -1) {
			bool matches_context = true;
			if (this->experiment_hook_test_scope_contexts.size() > context.size()) {
				matches_context = false;
			} else {
				for (int c_index = 0; c_index < (int)this->experiment_hook_test_scope_contexts.size()-1; c_index++) {
					if (this->experiment_hook_test_scope_contexts[c_index] != context[context.size() - this->experiment_hook_test_scope_contexts.size() + c_index].scope_id
							|| this->experiment_hook_test_node_contexts[c_index] != context[context.size() - this->experiment_hook_test_scope_contexts.size() + c_index].node_id) {
						matches_context = false;
						break;
					}
				}
			}

			if (matches_context) {
				run_helper.explore_history->test_obs_vals[this->experiment_hook_test_index].push_back(flat_vals[0]);
			}
		}
	}

	flat_vals.erase(flat_vals.begin());
}


