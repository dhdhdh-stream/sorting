#include "solution_helpers.h"

using namespace std;

void update_scores(ScopeHistory* scope_history,
 				   double target_val) {
 	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
 			it != scope_history->node_histories.end(); it++) {
 		if (it->second->node->last_updated_run_index != run_index) {
 			it->second->node->last_updated_run_index = run_index;
 			it->second->node->num_measure++;
 			it->second->node->sum_score += target_val;
 		}
 
 		if (it->second->node->type == NODE_TYPE_SCOPE) {
 			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
 
 			update_scores(scope_node_history->scope_history,
 						  target_val);
 		}
 	}
 }

void fetch_input_helper(RunHelper& run_helper,
						ScopeHistory* scope_history,
						Input& input,
						int l_index,
						bool& hit,
						double& obs) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.node_context[l_index]);
	if (it == scope_history->node_histories.end()) {
		hit = false;
		obs = 0.0;
	} else {
		if (l_index == (int)input.scope_context.size()-1) {
			switch (it->second->node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						hit = true;
						obs = 1.0;
					} else {
						hit = true;
						obs = -1.0;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
					if (input.factor_index == -1) {
						hit = true;
						obs = obs_node_history->obs_history[input.obs_index];
					} else {
						if (!obs_node_history->factor_initialized[input.factor_index]) {
							ObsNode* obs_node = (ObsNode*)it->second->node;
							double value = obs_node->factors[input.factor_index]->back_activate(
								run_helper,
								scope_history);
							obs_node_history->factor_values[input.factor_index] = value;
							obs_node_history->factor_initialized[input.factor_index] = true;
						}
						hit = true;
						obs = obs_node_history->factor_values[input.factor_index];
					}
				}
				break;
			}
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_input_helper(run_helper,
							   scope_node_history->scope_history,
							   input,
							   l_index+1,
							   hit,
							   obs);
		}
	}
}
