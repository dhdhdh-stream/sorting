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

void update_counts(ScopeHistory* scope_history,
				   int h_index) {
	Scope* scope = scope_history->scope;

	if (h_index != scope->last_updated_run_index) {
		scope->sum_hits++;
		scope->last_updated_run_index = h_index;
	}
	scope->sum_instances++;

	map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
	while (it != scope_history->node_histories.end()) {
		if (scope->nodes.find(it->first) == scope->nodes.end()) {
			delete it->second;
			it = scope_history->node_histories.erase(it);
		} else {
			AbstractNode* node = it->second->node;
			switch (node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node;
					if (h_index != action_node->last_updated_run_index) {
						action_node->sum_hits++;
						action_node->last_updated_run_index = h_index;
					}
					action_node->sum_instances++;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)node;
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

					update_counts(scope_node_history->scope_history,
								  h_index);

					if (h_index != scope_node->last_updated_run_index) {
						scope_node->sum_hits++;
						scope_node->last_updated_run_index = h_index;
					}
					scope_node->sum_instances++;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node;
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						if (h_index != branch_node->branch_last_updated_run_index) {
							branch_node->branch_sum_hits++;
							branch_node->branch_last_updated_run_index = h_index;
						}
						branch_node->branch_sum_instances++;
					} else {
						if (h_index != branch_node->original_last_updated_run_index) {
							branch_node->original_sum_hits++;
							branch_node->original_last_updated_run_index = h_index;
						}
						branch_node->original_sum_instances++;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;
					if (h_index != obs_node->last_updated_run_index) {
						obs_node->sum_hits++;
						obs_node->last_updated_run_index = h_index;
					}
					obs_node->sum_instances++;
				}
				break;
			}

			it++;
		}
	}
}

bool hit_helper(ScopeHistory* scope_history,
				Scope* scope_context,
				AbstractNode* node_context,
				bool is_branch) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
		map<int, AbstractNodeHistory*>::iterator it = scope_history
			->node_histories.find(node_context->id);
		if (it == scope_history->node_histories.end()) {
			return false;
		} else {
			if (node_context->type == NODE_TYPE_BRANCH) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				if (branch_node_history->is_branch == is_branch) {
					return true;
				} else {
					return false;
				}
			} else {
				return true;
			}
		}
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == scope_context) {
				is_child = true;
				break;
			}
		}

		if (!is_child) {
			return false;
		} else {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				AbstractNode* node = it->second->node;
				if (node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					bool inner_result = hit_helper(scope_node_history->scope_history,
												   scope_context,
												   node_context,
												   is_branch);
					if (inner_result) {
						return true;
					}
				}
			}

			return false;
		}
	}
}

void fetch_histories_helper(ScopeHistory* scope_history,
							double target_val,
							Scope* scope_context,
							AbstractNode* node_context,
							bool is_branch,
							vector<ScopeHistory*>& scope_histories,
							vector<double>& target_val_histories) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
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
			ScopeHistory* cleaned_scope_history = new ScopeHistory(scope_history, match_it->second->index);
			scope_histories.push_back(cleaned_scope_history);
			target_val_histories.push_back(target_val);
		}
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == scope_context) {
				is_child = true;
				break;
			}
		}

		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				AbstractNode* node = it->second->node;
				if (node->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)node;
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

					double inner_target_val;
					if (scope_node->signals.size() > 0) {
						if (!scope_node_history->signal_initialized) {
							scope_node_history->signal_val = calc_signal(scope_node,
																		 scope_history);
						}
						inner_target_val = scope_node_history->signal_val;
					} else {
						inner_target_val = target_val;
					}

					fetch_histories_helper(scope_node_history->scope_history,
										   inner_target_val,
										   scope_context,
										   node_context,
										   is_branch,
										   scope_histories,
										   target_val_histories);
				}
			}
		}
	}
}
