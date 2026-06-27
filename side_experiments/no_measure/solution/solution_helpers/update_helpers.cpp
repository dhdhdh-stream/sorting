#include "solution_helpers.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void update_helper(ScopeHistory* scope_history,
				   double target_val,
				   set<BranchNode*>& hit_original,
				   set<BranchNode*>& hit_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				update_helper(scope_node_history->scope_history,
							  target_val,
							  hit_original,
							  hit_branch);
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

					hit_branch.insert(branch_node);
				} else {
					branch_node->original_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->original_network->output->acti_vals[0];
					branch_node->original_network->backprop(error);

					hit_original.insert(branch_node);
				}
			}
			break;
		}
	}
}

void update_helper(set<BranchNode*>& hit_original,
				   set<BranchNode*>& hit_branch) {
	for (set<BranchNode*>::iterator it = hit_original.begin();
			it != hit_original.end(); it++) {
		BranchNode* branch_node = *it;
		branch_node->original_network->update();
	}

	for (set<BranchNode*>::iterator it = hit_branch.begin();
			it != hit_branch.end(); it++) {
		BranchNode* branch_node = *it;
		branch_node->branch_network->update();
	}
}
