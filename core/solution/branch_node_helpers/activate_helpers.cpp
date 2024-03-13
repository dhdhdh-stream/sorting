#include "branch_node.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "network.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  RunHelper& run_helper,
						  vector<AbstractNodeHistory*>& node_histories) {
	bool matches_context = false;
	vector<int> context_match_indexes;
	if (this->is_fuzzy_match) {
		context_match_indexes.push_back((int)context.size()-1);
		int c_index = (int)this->scope_context.size()-2;
		int l_index = (int)context.size()-2;
		while (true) {
			if (c_index < 0) {
				matches_context = true;
				break;
			}

			if (l_index < 0) {
				break;
			}

			if (this->scope_context[c_index] == context[l_index].scope
					&& this->node_context[c_index] == context[l_index].node) {
				context_match_indexes.insert(context_match_indexes.begin(), l_index);
				c_index--;
			}
			l_index--;
		}
	} else {
		context_match_indexes.push_back((int)context.size()-1);
		int c_index = (int)this->scope_context.size()-2;
		int l_index = (int)context.size()-2;
		while (true) {
			if (c_index < 0) {
				matches_context = true;
				break;
			}

			if (l_index < 0) {
				break;
			}

			if (this->scope_context[c_index] == context[l_index].scope
					&& this->node_context[c_index] == context[l_index].node) {
				context_match_indexes.insert(context_match_indexes.begin(), l_index);
				c_index--;
				l_index--;
			} else {
				break;
			}
		}
	}

	if (matches_context) {
		if (this->is_pass_through) {
			curr_node = this->branch_next_node;
		} else {
			BranchNodeHistory* history = new BranchNodeHistory(this);
			node_histories.push_back(history);

			vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				if (this->input_scope_contexts[i_index].size() > 0) {
					if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
						action_node->hook_indexes.push_back(i_index);
						action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
						action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						action_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
						action_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.push_back(i_index);
						branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
						branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						branch_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
						branch_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
					}
				}
			}
			vector<Scope*> scope_context;
			vector<AbstractNode*> node_context;
			input_vals_helper(scope_context,
							  node_context,
							  true,
							  0,
							  context_match_indexes,
							  input_vals,
							  context[context_match_indexes[0]].scope_history);
			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				if (this->input_scope_contexts[i_index].size() > 0) {
					if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
						action_node->hook_indexes.clear();
						action_node->hook_scope_contexts.clear();
						action_node->hook_node_contexts.clear();
						action_node->hook_is_fuzzy_match.clear();
						action_node->hook_strict_root_indexes.clear();
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.clear();
						branch_node->hook_scope_contexts.clear();
						branch_node->hook_node_contexts.clear();
						branch_node->hook_is_fuzzy_match.clear();
						branch_node->hook_strict_root_indexes.clear();
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
			if (branch_score > original_score) {
				history->is_branch = true;
			} else {
				history->is_branch = false;
			}
			#endif /* MDEBUG */

			if (history->is_branch) {
				curr_node = this->branch_next_node;

				for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
					if (this->experiment_types[e_index] == BRANCH_NODE_EXPERIMENT_TYPE_BRANCH) {
						bool is_selected = this->experiments[e_index]->activate(
							curr_node,
							problem,
							context,
							exit_depth,
							run_helper);
						if (is_selected) {
							return;
						}
					}
				}
			} else {
				curr_node = this->original_next_node;

				for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
					if (this->experiment_types[e_index] == BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL) {
						bool is_selected = this->experiments[e_index]->activate(
							curr_node,
							problem,
							context,
							exit_depth,
							run_helper);
						if (is_selected) {
							return;
						}
					}
				}
			}
		}
	} else {
		curr_node = this->original_next_node;
	}
}
