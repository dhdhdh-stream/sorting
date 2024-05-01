#include "eval.h"

#include "action_node.h"
#include "branch_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

double Eval::activate(Problem* problem,
					  RunHelper& run_helper) {
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

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = root_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->input_node_contexts[i_index].size()-1) {
					if (it->first->type == NODE_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
					} else {
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
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

	return score;
}
