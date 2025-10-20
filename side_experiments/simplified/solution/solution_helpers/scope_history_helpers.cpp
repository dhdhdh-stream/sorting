#include "solution_helpers.h"

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

void update_scores(ScopeHistory* scope_history,
				   double target_val,
				   int h_index) {
	Scope* scope = scope_history->scope;

	scope->existing_scope_histories.push_back(scope_history);
	scope->existing_target_val_histories.push_back(target_val);

	map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
	while (it != scope_history->node_histories.end()) {
		if (scope->nodes.find(it->first) == scope->nodes.end()) {
			delete it->second;
			it = scope_history->node_histories.erase(it);
		} else {
			AbstractNode* node = it->second->node;
			switch (node->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)node;
					if (h_index != start_node->last_updated_run_index) {
						start_node->sum_score += target_val;
						start_node->sum_hits++;

						start_node->last_updated_run_index = h_index;
					}
					start_node->sum_instances++;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node;
					if (h_index != action_node->last_updated_run_index) {
						action_node->sum_score += target_val;
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

					update_scores(scope_node_history->scope_history,
								  target_val,
								  h_index);

					if (h_index != scope_node->last_updated_run_index) {
						scope_node->sum_score += target_val;
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
							branch_node->branch_sum_score += target_val;
							branch_node->branch_sum_hits++;

							branch_node->branch_last_updated_run_index = h_index;
						}
						branch_node->branch_sum_instances++;
					} else {
						if (h_index != branch_node->original_last_updated_run_index) {
							branch_node->original_sum_score += target_val;
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
						obs_node->sum_score += target_val;
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

	while (scope_history->factor_initialized.size() < scope->factors.size()) {
		scope_history->factor_initialized.push_back(false);
		scope_history->factor_values.push_back(0.0);
	}
}

void new_scope_update_scores(ScopeHistory* scope_history,
							 double target_val,
							 int h_index,
							 Scope* scope_context) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
		map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
		while (it != scope_history->node_histories.end()) {
			if (scope->nodes.find(it->first) == scope->nodes.end()) {
				delete it->second;
				it = scope_history->node_histories.erase(it);
			} else {
				AbstractNode* node = it->second->node;
				switch (node->type) {
				case NODE_TYPE_START:
					{
						StartNode* start_node = (StartNode*)node;
						if (h_index != start_node->new_scope_last_updated_run_index) {
							start_node->new_scope_sum_score += target_val;
							start_node->new_scope_sum_hits++;

							start_node->new_scope_last_updated_run_index = h_index;
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)node;
						if (h_index != action_node->new_scope_last_updated_run_index) {
							action_node->new_scope_sum_score += target_val;
							action_node->new_scope_sum_hits++;

							action_node->new_scope_last_updated_run_index = h_index;
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)node;
						if (h_index != scope_node->new_scope_last_updated_run_index) {
							scope_node->new_scope_sum_score += target_val;
							scope_node->new_scope_sum_hits++;

							scope_node->new_scope_last_updated_run_index = h_index;
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)node;
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							if (h_index != branch_node->new_scope_branch_last_updated_run_index) {
								branch_node->new_scope_branch_sum_score += target_val;
								branch_node->new_scope_branch_sum_hits++;

								branch_node->new_scope_branch_last_updated_run_index = h_index;
							}
						} else {
							if (h_index != branch_node->new_scope_original_last_updated_run_index) {
								branch_node->new_scope_original_sum_score += target_val;
								branch_node->new_scope_original_sum_hits++;

								branch_node->new_scope_original_last_updated_run_index = h_index;
							}
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)node;
						if (h_index != obs_node->new_scope_last_updated_run_index) {
							obs_node->new_scope_sum_score += target_val;
							obs_node->new_scope_sum_hits++;

							obs_node->new_scope_last_updated_run_index = h_index;
						}
					}
					break;
				}

				it++;
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
						new_scope_update_scores(scope_node_history->scope_history,
												target_val,
												h_index,
												scope_context);
					}

					it++;
				}
			}
		}
	}
}
