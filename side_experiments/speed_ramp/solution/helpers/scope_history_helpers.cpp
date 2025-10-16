#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& val,
						bool& is_on) {
	if (input.factor_index != -1
			&& l_index == (int)input.scope_context.size()-1) {
		while (scope_history->factor_initialized.size() < scope_history->scope->factors.size()) {
			scope_history->factor_initialized.push_back(false);
			scope_history->factor_values.push_back(0.0);
		}

		if (!scope_history->factor_initialized[input.factor_index]) {
			double value = scope_history->scope->factors[input.factor_index]->back_activate(scope_history);
			scope_history->factor_initialized[input.factor_index] = true;
			scope_history->factor_values[input.factor_index] = value;
		}
		val = scope_history->factor_values[input.factor_index];
		is_on = true;
	} else {
		map<int, AbstractNodeHistory*>::iterator it = scope_history
			->node_histories.find(input.node_context[l_index]);
		if (it != scope_history->node_histories.end()) {
			if (l_index == (int)input.scope_context.size()-1) {
				switch (it->second->node->type) {
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							val = 1.0;
							is_on = true;
						} else {
							val = -1.0;
							is_on = true;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
						val = obs_node_history->obs_history[input.obs_index];
						is_on = true;
					}
					break;
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				fetch_input_helper(scope_node_history->scope_history,
								   input,
								   l_index+1,
								   val,
								   is_on);
			}
		} else {
			val = 0.0;
			is_on = false;
		}
	}
}

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& val,
						bool& is_on,
						int& num_actions_snapshot) {
	if (input.factor_index != -1
			&& l_index == (int)input.scope_context.size()-1) {
		while (scope_history->factor_initialized.size() < scope_history->scope->factors.size()) {
			scope_history->factor_initialized.push_back(false);
			scope_history->factor_values.push_back(0.0);
		}

		if (!scope_history->factor_initialized[input.factor_index]) {
			double value = scope_history->scope->factors[input.factor_index]->back_activate(scope_history);
			scope_history->factor_initialized[input.factor_index] = true;
			scope_history->factor_values[input.factor_index] = value;
		}
		val = scope_history->factor_values[input.factor_index];
		is_on = true;
		num_actions_snapshot = scope_history->num_actions_snapshot;
	} else {
		map<int, AbstractNodeHistory*>::iterator it = scope_history
			->node_histories.find(input.node_context[l_index]);
		if (it != scope_history->node_histories.end()) {
			if (l_index == (int)input.scope_context.size()-1) {
				switch (it->second->node->type) {
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							val = 1.0;
						} else {
							val = -1.0;
						}
						is_on = true;
						num_actions_snapshot = branch_node_history->num_actions_snapshot;
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
						val = obs_node_history->obs_history[input.obs_index];
						is_on = true;
						num_actions_snapshot = obs_node_history->num_actions_snapshot;
					}
					break;
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				fetch_input_helper(scope_node_history->scope_history,
								   input,
								   l_index+1,
								   val,
								   is_on,
								   num_actions_snapshot);
			}
		} else {
			val = 0.0;
			is_on = false;
		}
	}
}

void fetch_histories_helper(ScopeHistory* scope_history,
							vector<ScopeHistory*>& scope_context_histories,
							double true_diff,
							AbstractNode* node_context,
							bool is_branch,
							vector<ScopeHistory*>& scope_histories,
							vector<double>& target_val_histories) {
	scope_context_histories.push_back(scope_history);

	Scope* scope = scope_history->scope;

	if (scope == node_context->parent) {
		bool has_match = false;

		map<int, AbstractNodeHistory*>::iterator match_it = scope_history->node_histories.find(node_context->id);
		if (match_it != scope_history->node_histories.end()) {
			if (node_context->type == NODE_TYPE_BRANCH) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)match_it->second;
				if (branch_node_history->is_branch == is_branch) {
					has_match = true;
				}
			} else {
				has_match = true;
			}
		}

		if (has_match) {
			ScopeHistory* cleaned_scope_history = new ScopeHistory(
				scope_history,
				match_it->second->index,
				match_it->second->num_actions_snapshot);
			scope_histories.push_back(cleaned_scope_history);

			double sum_vals = true_diff;
			int sum_count = 1;
			for (int l_index = 0; l_index < (int)scope_context_histories.size(); l_index++) {
				if (scope_context_histories[l_index]->signal_initialized) {
					sum_vals += scope_context_histories[l_index]->signal_val;
					sum_count++;
				}
			}
			double average_val = sum_vals / (double)sum_count;
			target_val_histories.push_back(average_val);
		}
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == node_context->parent) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			while (it != scope_history->node_histories.end()) {
				if (scope->nodes.find(it->first) == scope->nodes.end()) {
					delete it->second;
					it = scope_history->node_histories.erase(it);
				} else {
					AbstractNode* node = it->second->node;
					if (node->type == NODE_TYPE_SCOPE) {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
						fetch_histories_helper(scope_node_history->scope_history,
											   scope_context_histories,
											   true_diff,
											   node_context,
											   is_branch,
											   scope_histories,
											   target_val_histories);
					}

					it++;
				}
			}
		}
	}

	scope_context_histories.pop_back();
}
