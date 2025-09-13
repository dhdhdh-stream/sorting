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
		// TODO: clean experiment inputs
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
