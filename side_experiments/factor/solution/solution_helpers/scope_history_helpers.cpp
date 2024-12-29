#include "solution_helpers.h"

using namespace std;

void update_scores(ScopeHistory* scope_history,
				   double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		it->second->node->num_measure++;
		it->second->node->sum_score += target_val;

		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

			update_scores(scope_node_history->scope_history,
						  target_val);
		}
	}
}

void gather_factors(ScopeHistory* scope_history,
					map<pair<int,int>, double>& factors) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_OBS) {
			ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
			ObsNode* obs_node = (ObsNode*)it->second->node;

			for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
				if (!obs_node_history->factor_initialized[f_index]) {
					double value = obs_node->factors[f_index]->back_activate(scope_history);
					obs_node_history->factor_values[f_index] = value;
				}
				factors[{obs_node->id, f_index}] = obs_node_history->factor_values[f_index];
			}
		}
	}
}

void fetch_input_helper(ScopeHistory* scope_history,
						pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>& input,
						int l_index,
						double& obs) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.first.second[l_index]);
	if (it != scope_history->node_histories.end()) {
		if (l_index == (int)input.first.first.size()-1) {
			switch (it->second->node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						obs = 1.0;
					} else {
						obs = -1.0;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
					if (input.second.first == -1) {
						obs = obs_node_history->obs_history[input.second.second];
					} else {
						if (!obs_node_history->factor_initialized[input.second.first]) {
							ObsNode* obs_node = (ObsNode*)it->second->node;
							double value = obs_node->factors[input.second.first]->back_activate(scope_history);
							obs_node_history->factor_values[input.second.first] = value;
						}
						obs = obs_node_history->factor_values[input.second.first];
					}
				}
				break;
			}
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_input_helper(scope_node_history->scope_history,
							   input,
							   l_index+1,
							   obs);
		}
	}
}
