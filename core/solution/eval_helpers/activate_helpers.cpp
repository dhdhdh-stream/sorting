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

void Eval::activate_start(Problem* problem,
						  RunHelper& run_helper,
						  EvalHistory* history) {
	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->start_subscope;
	inner_context.back().node = NULL;

	history->start_scope_history = new ScopeHistory(this->start_subscope);
	inner_context.back().scope_history = history->start_scope_history;

	this->start_subscope->activate(
		problem,
		inner_context,
		run_helper,
		history->start_scope_history);
}

void Eval::activate_end(Problem* problem,
						RunHelper& run_helper,
						EvalHistory* history) {
	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->end_subscope;
	inner_context.back().node = NULL;

	history->end_scope_history = new ScopeHistory(this->end_subscope);
	inner_context.back().scope_history = history->end_scope_history;

	this->end_subscope->activate(
		problem,
		inner_context,
		run_helper,
		history->end_scope_history);
}

double Eval::calc_impact(RunHelper& run_helper,
						 EvalHistory* history) {
	run_helper.num_decisions++;

	vector<double> input_vals(this->end_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->end_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it;
		if (this->end_is_start[i_index]) {
			it = history->start_scope_history->node_histories.find(this->end_input_node_contexts[i_index]);
		} else {
			it = history->end_scope_history->node_histories.find(this->end_input_node_contexts[i_index]);
		}
		if (it != root_history->node_histories.end()) {
			switch (this->end_input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->end_input_obs_indexes[i_index]];
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

	double score = this->end_average_score;
	for (int i_index = 0; i_index < (int)this->end_linear_input_indexes.size(); i_index++) {
		score += input_vals[this->end_linear_input_indexes[i_index]] * this->end_linear_weights[i_index];
	}
	if (this->end_network != NULL) {
		vector<vector<double>> network_input_vals(this->end_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->end_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->end_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->end_network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[this->end_network_input_indexes[i_index][v_index]];
			}
		}
		this->end_network->activate(network_input_vals);
		score += this->end_network->output->acti_vals[0];
	}

	return score;
}
