#include "solution_helpers.h"

#include "branch_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void retrain_helper(ScopeHistory* scope_history,
					double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				retrain_helper(scope_node_history->scope_history,
							   target_val);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					branch_node->branch_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->branch_network->output->acti_vals[0];
					branch_node->branch_network->backprop(error);
				} else {
					branch_node->original_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->original_network->output->acti_vals[0];
					branch_node->original_network->backprop(error);
				}
			}
			break;
		}
	}
}
