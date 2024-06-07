#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	run_helper.num_decisions++;

	BranchNodeHistory* history = new BranchNodeHistory();
	history->index = (int)node_histories.size();
	node_histories[this] = history;

	vector<double> original_input_vals(this->original_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->original_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
			this->original_input_node_contexts[i_index]);
		if (it != context.back().scope_history->node_histories.end()) {
			switch (it->first->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					original_input_vals[i_index] = action_node_history->obs_snapshot[this->original_input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					original_input_vals[i_index] = scope_node_history->obs_snapshot[this->original_input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						original_input_vals[i_index] = 1.0;
					} else {
						original_input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					if (info_scope_node_history->is_positive) {
						original_input_vals[i_index] = 1.0;
					} else {
						original_input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
					if (info_branch_node_history->is_branch) {
						original_input_vals[i_index] = 1.0;
					} else {
						original_input_vals[i_index] = -1.0;
					}
				}
				break;
			}
		}
	}
	this->original_network->activate(original_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double original_score = this->original_network->output->acti_vals[0];
	#endif /* MDEBUG */

	vector<double> branch_input_vals(this->branch_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->branch_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
			this->branch_input_node_contexts[i_index]);
		if (it != context.back().scope_history->node_histories.end()) {
			switch (it->first->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					branch_input_vals[i_index] = action_node_history->obs_snapshot[this->branch_input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					branch_input_vals[i_index] = scope_node_history->obs_snapshot[this->branch_input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						branch_input_vals[i_index] = 1.0;
					} else {
						branch_input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					if (info_scope_node_history->is_positive) {
						branch_input_vals[i_index] = 1.0;
					} else {
						branch_input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
					if (info_branch_node_history->is_branch) {
						branch_input_vals[i_index] = 1.0;
					} else {
						branch_input_vals[i_index] = -1.0;
					}
				}
				break;
			}
		}
	}
	this->branch_network->activate(branch_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double branch_score = this->branch_network->output->acti_vals[0];
	#endif /* MDEBUG */

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (branch_score >= original_score) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
	}
	#endif /* MDEBUG */

	if (history->is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			history->is_branch,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}

	uniform_int_distribution<int> swap_distribution(0, run_helper.num_actions-1);
	if (swap_distribution(generator) == 0) {
		run_helper.explore_node = this;
		run_helper.explore_is_branch = history->is_branch;
	}
}
