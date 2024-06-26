#include "eval_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

double calc_score(ScopeHistory* scope_history) {
	Scope* scope = (Scope*)scope_history->scope;

	vector<double> input_vals(scope->eval_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)scope->eval_input_scope_contexts.size(); i_index++) {
		int curr_layer = 0;
		AbstractScopeHistory* curr_scope_history = scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				scope->eval_input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)scope->eval_input_scope_contexts[i_index].size()-1) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							input_vals[i_index] = action_node_history->obs_snapshot[scope->eval_input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							input_vals[i_index] = branch_node_history->score;
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}
	scope->eval_network->activate(input_vals);
	double score = scope->eval_network->output->acti_vals[0];

	return score;
}
