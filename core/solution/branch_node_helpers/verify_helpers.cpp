#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "action_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 vector<AbstractNodeHistory*>& node_histories) {
	bool matches_context = true;
	if (this->scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
					|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
				matches_context = false;
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
			if (this->input_max_depth > 0) {
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_scope_contexts[i_index].size() > 0) {
						if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
							action_node->hook_indexes.push_back(i_index);
							action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
							action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						} else {
							BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
							branch_node->hook_indexes.push_back(i_index);
							branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
							branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						}
					}
				}
				vector<Scope*> scope_context;
				vector<AbstractNode*> node_context;
				input_vals_helper(0,
								  this->input_max_depth,
								  scope_context,
								  node_context,
								  input_vals,
								  context[context.size() - this->scope_context.size()].scope_history);
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_scope_contexts[i_index].size() > 0) {
						if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
							action_node->hook_indexes.clear();
							action_node->hook_scope_contexts.clear();
							action_node->hook_node_contexts.clear();
						} else {
							BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
							branch_node->hook_indexes.clear();
							branch_node->hook_scope_contexts.clear();
							branch_node->hook_node_contexts.clear();
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

			if (this->verify_key == run_helper.verify_key) {
				// cout << "input_vals:" << endl;
				// for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
				// 	cout << i_index << ": " << input_vals[i_index] << endl;
				// }
				// cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
				// problem->print();

				// cout << "context scope" << endl;
				// for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
				// 	cout << c_index << ": " << context[c_index].scope->id << endl;
				// }
				// cout << "context node" << endl;
				// for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
				// 	cout << c_index << ": " << context[c_index].node->id << endl;
				// }

				if (this->verify_original_scores[0] != original_score
						|| this->verify_branch_scores[0] != branch_score) {
					cout << "this->verify_original_scores[0]: " << this->verify_original_scores[0] << endl;
					cout << "original_score: " << original_score << endl;

					cout << "this->verify_branch_scores[0]: " << this->verify_branch_scores[0] << endl;
					cout << "branch_score: " << branch_score << endl;

					throw invalid_argument("branch node verify fail");
				}

				this->verify_original_scores.erase(this->verify_original_scores.begin());
				this->verify_branch_scores.erase(this->verify_branch_scores.begin());
			}

			if (run_helper.curr_run_seed%2 == 0) {
				history->is_branch = true;
			} else {
				history->is_branch = false;
			}
			// if (this->verify_key == run_helper.verify_key) {
			// 	cout << "history->is_branch: " << history->is_branch << endl;
			// }
			run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

			if (history->is_branch) {
				curr_node = this->branch_next_node;
			} else {
				curr_node = this->original_next_node;
			}
		}
	} else {
		curr_node = this->original_next_node;
	}
}

#endif /* MDEBUG */