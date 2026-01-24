#include "solution_helpers.h"

#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void eval_curr_tunnel_helper(ScopeHistory* scope_history,
							 SolutionWrapper* wrapper,
							 double& sum_vals) {
	Scope* scope = scope_history->scope;

	if (scope == wrapper->curr_tunnel) {
		scope->post_network->activate(scope_history->post_obs_history);
		sum_vals += scope->post_network->output->acti_vals[0];
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == wrapper->curr_tunnel) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				if (it->second->node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					eval_curr_tunnel_helper(scope_node_history->scope_history,
											wrapper,
											sum_vals);
				}
			}
		}
	}
}
