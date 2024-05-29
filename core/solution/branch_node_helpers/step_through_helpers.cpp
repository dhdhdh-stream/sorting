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
	double original_score = this->original_network->output->acti_vals[0];

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
	double branch_score = this->branch_network->output->acti_vals[0];

	if (branch_score >= original_score) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
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
	cout << "is_branch: " << history->is_branch << endl;

	if (history->is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}
