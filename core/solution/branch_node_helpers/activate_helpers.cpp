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

	vector<double> original_input_vals(this->original_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->original_input_scope_contexts.size(); i_index++) {
		if (this->original_input_scope_contexts[i_index].size() > 0) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->original_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->original_input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								original_input_vals[i_index] = action_node_history->obs_snapshot[this->original_input_obs_indexes[i_index]];
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
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
			}
		}
	}
	this->original_network->activate(original_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double original_score = this->original_network->output->acti_vals[0];
	#endif /* MDEBUG */

	vector<double> branch_input_vals(this->branch_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->branch_input_scope_contexts.size(); i_index++) {
		if (this->branch_input_scope_contexts[i_index].size() > 0) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->branch_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->branch_input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								branch_input_vals[i_index] = action_node_history->obs_snapshot[this->branch_input_obs_indexes[i_index]];
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
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
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
}
