#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
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

	if (this->verify_key != NULL) {
		cout << "this->id: " << this->id << endl;

		cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
		cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
		problem->print();

		cout << "context scope" << endl;
		for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			cout << c_index << ": " << context[c_index].scope->id << endl;
		}
		cout << "context node" << endl;
		for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			cout << c_index << ": " << context[c_index].node->id << endl;
		}

		if (this->verify_scores[0] != history->score) {
			cout << "this->verify_scores[0]: " << this->verify_scores[0] << endl;
			cout << "history->score: " << history->score << endl;

			cout << "seed: " << seed << endl;

			throw invalid_argument("branch node verify fail");
		}

		this->verify_scores.erase(this->verify_scores.begin());
	}

	bool is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	if (this->verify_key != NULL) {
		cout << "is_branch: " << is_branch << endl;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}

#endif /* MDEBUG */