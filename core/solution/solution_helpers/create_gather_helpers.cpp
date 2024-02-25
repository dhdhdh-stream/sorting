#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"

using namespace std;

void create_gather_helper(bool on_path,
						  vector<Scope*>& scope_context,
						  vector<AbstractNode*>& node_context,
						  vector<vector<Scope*>>& possible_scope_contexts,
						  vector<vector<AbstractNode*>>& possible_node_contexts,
						  vector<bool>& possible_is_branch,
						  ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);

				node_context.back() = NULL;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			if (on_path && h_index == (int)scope_history->node_histories.size()-1) {
				create_gather_helper(true,
									 scope_context,
									 node_context,
									 possible_scope_contexts,
									 possible_node_contexts,
									 possible_is_branch,
									 scope_node_history->scope_history);
			} else {
				uniform_int_distribution<int> inner_distribution(0, 2);
				if (inner_distribution(generator) == 0) {
					create_gather_helper(false,
										 scope_context,
										 node_context,
										 possible_scope_contexts,
										 possible_node_contexts,
										 possible_is_branch,
										 scope_node_history->scope_history);
				} else {
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
					possible_is_branch.push_back(false);
				}
			}

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(branch_node_history->is_branch);

			node_context.back() = NULL;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_gather(vector<Scope*>& new_gather_scope_context,
				   vector<AbstractNode*>& new_gather_node_context,
				   bool& new_gather_is_branch,
				   int& new_gather_exit_depth,
				   AbstractNode*& new_gather_exit_node,
				   ScopeHistory* scope_history,
				   SeedExperimentFilter* seed_experiment_filter) {
	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;
	vector<bool> possible_is_branch;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_gather_helper(true,
						 scope_context,
						 node_context,
						 possible_scope_contexts,
						 possible_node_contexts,
						 possible_is_branch,
						 scope_history);

	uniform_int_distribution<int> start_distribution(0, (int)possible_scope_contexts.size()-1);
	int start_index = start_distribution(generator);

	vector<vector<Scope*>> possible_end_scope_contexts;
	vector<vector<AbstractNode*>> possible_end_node_contexts;
	for (int p_index = start_index+1; p_index < (int)possible_scope_contexts.size(); p_index++) {
		/**
		 * - every layer except last matches
		 */
		bool matches_context = true;
		if (possible_scope_contexts[p_index].size() > possible_scope_contexts[start_index].size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)possible_scope_contexts[p_index].size()-1; c_index++) {
				if (possible_scope_contexts[p_index][c_index] != possible_scope_contexts[start_index][c_index]
						|| possible_node_contexts[p_index][c_index] != possible_node_contexts[start_index][c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			possible_end_scope_contexts.push_back(possible_scope_contexts[p_index]);
			possible_end_node_contexts.push_back(possible_node_contexts[p_index]);
		}
	}
	/**
	 * - add scope ends
	 */
	if (possible_end_scope_contexts.size() > 1) {
		int e_index = 0;
		while (true) {
			if (possible_end_scope_contexts[e_index].size() < possible_end_scope_contexts[e_index+1].size()) {
				vector<Scope*> end_scope_context = possible_end_scope_contexts[e_index];
				vector<AbstractNode*> end_node_context = possible_end_node_contexts[e_index];
				end_node_context.back() = NULL;
				possible_end_scope_contexts.insert(possible_end_scope_contexts.begin() + e_index+1, end_scope_context);
				possible_end_node_contexts.insert(possible_end_node_contexts.begin() + e_index+1, end_node_context);
			}

			e_index++;
			if (e_index >= (int)possible_end_scope_contexts.size()-1) {
				break;
			}
		}
	}
	{
		bool on_path = true;
		if (possible_scope_contexts[start_index].size() >= seed_experiment_filter->scope_context.size()) {
			on_path = false;
		} else {
			for (int c_index = 0; c_index < (int)possible_scope_contexts[start_index].size()-1; c_index++) {
				if (possible_scope_contexts[start_index][c_index] != seed_experiment_filter->scope_context[c_index]
						|| possible_node_contexts[start_index][c_index] != seed_experiment_filter->node_context[c_index]) {
					on_path = false;
					break;
				}
			}
		}

		if (on_path) {
			vector<Scope*> scope_node_scope_context = seed_experiment_filter->scope_context;
			scope_node_scope_context.erase(scope_node_scope_context.begin() + possible_scope_contexts[start_index].size(),
				scope_node_scope_context.end());
			vector<AbstractNode*> scope_node_node_context = seed_experiment_filter->node_context;
			scope_node_node_context.erase(scope_node_node_context.begin() + possible_scope_contexts[start_index].size(),
				scope_node_node_context.end());
			possible_end_scope_contexts.push_back(scope_node_scope_context);
			possible_end_node_contexts.push_back(scope_node_node_context);
		} else {
			bool matches_rise = true;
			if (seed_experiment_filter->scope_context.size() > possible_scope_contexts[start_index].size()) {
				matches_rise = false;
			} else {
				for (int c_index = 0; c_index < (int)seed_experiment_filter->scope_context.size()-1; c_index++) {
					if (seed_experiment_filter->scope_context[c_index] != possible_scope_contexts[start_index][c_index]
							|| seed_experiment_filter->node_context[c_index] != possible_node_contexts[start_index][c_index]) {
						matches_rise = false;
						break;
					}
				}
			}

			if (matches_rise) {
				vector<Scope*> filter_scope_context = seed_experiment_filter->scope_context;
				vector<AbstractNode*> filter_node_context = seed_experiment_filter->node_context;
				filter_node_context.back() = seed_experiment_filter->branch_node;
				possible_end_scope_contexts.push_back(filter_scope_context);
				possible_end_node_contexts.push_back(filter_node_context);
			} else {
				/**
				 * - add final scope end
				 */
				if (possible_end_scope_contexts.size() == 0) {
					vector<Scope*> end_scope_context = possible_scope_contexts[start_index];
					vector<AbstractNode*> end_node_context = possible_node_contexts[start_index];
					end_node_context.back() = NULL;
					possible_end_scope_contexts.push_back(end_scope_context);
					possible_end_node_contexts.push_back(end_node_context);
				} else {
					vector<Scope*> end_scope_context = possible_end_scope_contexts.back();
					vector<AbstractNode*> end_node_context = possible_end_node_contexts.back();
					end_node_context.back() = NULL;
					possible_end_scope_contexts.push_back(end_scope_context);
					possible_end_node_contexts.push_back(end_node_context);
				}
			}
		}
	}

	geometric_distribution<int> end_distribution(0.2);
	int end_index = end_distribution(generator);
	if (end_index > (int)possible_end_scope_contexts.size()-1) {
		end_index = (int)possible_end_scope_contexts.size()-1;
	}

	new_gather_scope_context = possible_scope_contexts[start_index];
	new_gather_node_context = possible_node_contexts[start_index];
	new_gather_is_branch = possible_is_branch[start_index];
	new_gather_exit_depth = (int)possible_scope_contexts[start_index].size() - (int)possible_end_scope_contexts[end_index].size();
	new_gather_exit_node = possible_end_node_contexts[end_index].back();
}
