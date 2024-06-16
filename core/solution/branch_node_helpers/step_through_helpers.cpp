#include "branch_node.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::step_through_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   vector<ContextLayer>& context,
									   RunHelper& run_helper,
									   map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	run_helper.num_decisions++;

	BranchNodeHistory* history = new BranchNodeHistory();
	history->index = (int)node_histories.size();
	node_histories[this] = history;

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != context.back().scope_history->node_histories.end()) {
			switch (it->first->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					input_vals[i_index] = scope_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
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
					input_vals[i_index] = info_branch_node_history->score;
				}
				break;
			}
		}
	}
	this->network->activate(input_vals);
	history->score = this->network->output->acti_vals[0];

	bool is_branch;
	if (history->score >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	string input_gate;
	cin >> input_gate;

	cout << "context:" << endl;
	context.back().node = this;
	for (int c_index = 0; c_index < (int)context.size(); c_index++) {
		cout << c_index << ": " << context[c_index].scope->id << " " << context[c_index].node->id << endl;
	}
	context.back().node = NULL;
	problem->print();
	cout << "BranchNode" << endl;
	cout << "score: " << history->score << endl;

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}
