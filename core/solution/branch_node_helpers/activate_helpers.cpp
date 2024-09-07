#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "info_branch_node.h"
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
	if (run_helper.branch_node_ancestors.find(this) != run_helper.branch_node_ancestors.end()) {
		curr_node = this->original_next_node;
	} else {
		run_helper.branch_node_ancestors.insert(this);

		run_helper.num_decisions++;

		BranchNodeHistory* history = new BranchNodeHistory();
		history->index = (int)node_histories.size();
		node_histories[this] = history;

		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			AbstractScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
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
		this->network->activate(input_vals);
		history->score = this->network->output->acti_vals[0];

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (history->score >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		#endif /* MDEBUG */

		if (is_branch) {
			curr_node = this->branch_next_node;
		} else {
			curr_node = this->original_next_node;
		}

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				is_branch,
				curr_node,
				problem,
				context,
				run_helper);
			if (is_selected) {
				return;
			}
		}

		if (run_helper.experiments_seen_order.size() == 0) {
			map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.scope_nodes_seen.find({this, is_branch});
			if (it == run_helper.scope_nodes_seen.end()) {
				run_helper.scope_nodes_seen[{this, is_branch}] = 1;
			} else {
				it->second++;
			}
		}
	}
}
