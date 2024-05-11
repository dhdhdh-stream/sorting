#include "eval.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

double Eval::activate(Problem* problem,
					  RunHelper& run_helper) {
	int existing_num_actions_until_random = solution->num_actions_until_random;
	solution->num_actions_until_random = -1;

	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->subscope;
	inner_context.back().node = NULL;

	ScopeHistory* root_history = new ScopeHistory(this->subscope);
	inner_context.back().scope_history = root_history;

	this->subscope->activate(
		problem,
		inner_context,
		run_helper,
		root_history);

	run_helper.num_decisions++;

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = root_history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != root_history->node_histories.end()) {
			switch (this->input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					if (info_scope_node_history->is_positive) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
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
		}
	}

	double score = this->average_score;
	for (int i_index = 0; i_index < (int)this->linear_input_indexes.size(); i_index++) {
		score += input_vals[this->linear_input_indexes[i_index]] * this->linear_weights[i_index];
	}
	if (this->network != NULL) {
		vector<vector<double>> network_input_vals(this->network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[this->network_input_indexes[i_index][v_index]];
			}
		}
		this->network->activate(network_input_vals);
		score += this->network->output->acti_vals[0];
	}

	delete root_history;

	solution->num_actions_until_random = existing_num_actions_until_random;

	return score;
}
