#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
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
	if (solution->num_actions_until_experiment != -1
			&& run_helper.num_actions_after_experiment_to_skip > 0) {
		uniform_int_distribution<int> distribution(0, 1);
		if (distribution(generator) == 0) {
			curr_node = this->branch_next_node;
		} else {
			curr_node = this->original_next_node;
		}
	} else {
		run_helper.num_decisions++;

		BranchNodeHistory* history = new BranchNodeHistory();
		history->index = (int)node_histories.size();
		node_histories[this] = history;

		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
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

		double original_score = this->original_average_score;
		for (int i_index = 0; i_index < (int)this->linear_original_input_indexes.size(); i_index++) {
			original_score += input_vals[this->linear_original_input_indexes[i_index]] * this->linear_original_weights[i_index];
		}
		if (this->original_network != NULL) {
			vector<vector<double>> original_network_input_vals(this->original_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
				original_network_input_vals[i_index] = vector<double>(this->original_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->original_network_input_indexes[i_index].size(); v_index++) {
					original_network_input_vals[i_index][v_index] = input_vals[this->original_network_input_indexes[i_index][v_index]];
				}
			}
			this->original_network->activate(original_network_input_vals);
			original_score += this->original_network->output->acti_vals[0];
		}

		double branch_score = this->branch_average_score;
		for (int i_index = 0; i_index < (int)this->linear_branch_input_indexes.size(); i_index++) {
			branch_score += input_vals[this->linear_branch_input_indexes[i_index]] * this->linear_branch_weights[i_index];
		}
		if (this->branch_network != NULL) {
			vector<vector<double>> branch_network_input_vals(this->branch_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->branch_network_input_indexes.size(); i_index++) {
				branch_network_input_vals[i_index] = vector<double>(this->branch_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->branch_network_input_indexes[i_index].size(); v_index++) {
					branch_network_input_vals[i_index][v_index] = input_vals[this->branch_network_input_indexes[i_index][v_index]];
				}
			}
			this->branch_network->activate(branch_network_input_vals);
			branch_score += this->branch_network->output->acti_vals[0];
		}

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
			if (this->experiments[e_index]->is_branch == history->is_branch) {
				bool is_selected = this->experiments[e_index]->activate(
					curr_node,
					problem,
					context,
					run_helper);
				if (is_selected) {
					return;
				}
			}
		}
	}
}
