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

void Eval::activate(Problem* problem,
					RunHelper& run_helper,
					ScopeHistory*& scope_history) {
	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->subscope;
	inner_context.back().node = NULL;

	scope_history = new ScopeHistory(this->subscope);
	inner_context.back().scope_history = scope_history;

	this->subscope->activate(
		problem,
		inner_context,
		run_helper,
		scope_history);
}

double Eval::calc_score(RunHelper& run_helper,
						ScopeHistory* scope_history) {
	run_helper.num_decisions++;

	vector<double> input_vals(this->score_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(this->score_input_node_contexts[i_index]);
		if (it != scope_history->node_histories.end()) {
			switch (this->score_input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->score_input_obs_indexes[i_index]];
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

	double score = this->score_average_score;
	for (int i_index = 0; i_index < (int)this->score_linear_input_indexes.size(); i_index++) {
		score += input_vals[this->score_linear_input_indexes[i_index]] * this->score_linear_weights[i_index];
	}
	if (this->score_network != NULL) {
		vector<vector<double>> network_input_vals(this->score_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->score_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->score_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->score_network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[this->score_network_input_indexes[i_index][v_index]];
			}
		}
		this->score_network->activate(network_input_vals);
		score += this->score_network->output->acti_vals[0];
	}

	return score;
}

double Eval::calc_vs(RunHelper& run_helper,
					 EvalHistory* history) {
	run_helper.num_decisions++;

	vector<double> input_vals(this->vs_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it;
		bool does_contain;
		if (this->vs_input_is_start[i_index]) {
			it = history->start_scope_history->node_histories.find(this->vs_input_node_contexts[i_index]);
			if (it != history->start_scope_history->node_histories.end()) {
				does_contain = true;
			} else {
				does_contain = false;
			}
		} else {
			it = history->end_scope_history->node_histories.find(this->vs_input_node_contexts[i_index]);
			if (it != history->end_scope_history->node_histories.end()) {
				does_contain = true;
			} else {
				does_contain = false;
			}
		}
		if (does_contain) {
			switch (this->vs_input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->vs_input_obs_indexes[i_index]];
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

	double score = this->vs_average_score;
	for (int i_index = 0; i_index < (int)this->vs_linear_input_indexes.size(); i_index++) {
		score += input_vals[this->vs_linear_input_indexes[i_index]] * this->vs_linear_weights[i_index];
	}
	if (this->vs_network != NULL) {
		vector<vector<double>> network_input_vals(this->vs_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->vs_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->vs_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->vs_network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[this->vs_network_input_indexes[i_index][v_index]];
			}
		}
		this->vs_network->activate(network_input_vals);
		score += this->vs_network->output->acti_vals[0];
	}

	return score;
}
