#include "eval.h"

#include "action_node.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"

using namespace std;

void Eval::activate_start(Problem* problem,
						  RunHelper& run_helper,
						  EvalHistory* history) {
	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->subscope;
	inner_context.back().node = NULL;

	inner_context.back().scope_history = history->scope_history;

	this->subscope->activate(this->subscope->nodes[0],
							 problem,
							 inner_context,
							 run_helper,
							 history->scope_history);

	history->start_eval_index = history->scope_history->node_histories.size();

	this->subscope->activate(this->subscope->nodes[1],
							 problem,
							 inner_context,
							 run_helper,
							 history->scope_history);
}

void Eval::activate_end(Problem* problem,
						RunHelper& run_helper,
						EvalHistory* history) {
	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->subscope;
	inner_context.back().node = NULL;

	inner_context.back().scope_history = history->scope_history;

	history->end_orientation_index = history->scope_history->node_histories.size();

	this->subscope->activate(this->subscope->nodes[2],
							 problem,
							 inner_context,
							 run_helper,
							 history->scope_history);

	history->end_eval_index = history->scope_history->node_histories.size();

	this->subscope->activate(this->subscope->nodes[3],
							 problem,
							 inner_context,
							 run_helper,
							 history->scope_history);
}

double Eval::calc_impact(EvalHistory* history) {
	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = history->scope_history->node_histories.find(this->input_node_contexts[i_index]);
		if (it != history->scope_history->node_histories.end()) {
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
	this->network->activate(input_vals);
	double score = this->network->output->acti_vals[0];

	return score;
}
