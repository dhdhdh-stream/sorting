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

using namespace std;

void gather_possible_helper(ScopeHistory* scope_history,
							vector<Scope*>& scope_context,
							vector<int>& node_context,
							int& node_count,
							Input& new_input,
							SolutionWrapper* wrapper) {
	Scope* scope = scope_history->scope;

	uniform_int_distribution<int> obs_distribution(0, wrapper->num_obs-1);
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				scope_context.push_back(scope);
				node_context.push_back(it->first);

				gather_possible_helper(scope_node_history->scope_history,
									   scope_context,
									   node_context,
									   node_count,
									   new_input,
									   wrapper);

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					new_input.scope_context = scope_context;
					new_input.node_context = node_context;
					new_input.factor_index = -1;
					new_input.obs_index = -1;

					scope_context.pop_back();
					node_context.pop_back();
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;

				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					new_input.scope_context = scope_context;
					new_input.node_context = node_context;
					new_input.factor_index = -1;
					new_input.obs_index = obs_distribution(generator);

					scope_context.pop_back();
					node_context.pop_back();
				}

				for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
					if (obs_node->factors[f_index]->inputs.size() > 0) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							scope_context.push_back(scope);
							node_context.push_back(it->first);

							new_input.scope_context = scope_context;
							new_input.node_context = node_context;
							new_input.factor_index = f_index;
							new_input.obs_index = -1;

							scope_context.pop_back();
							node_context.pop_back();
						}
					}
				}
			}
			break;
		}
	}
}

void gather_possible_factor_helper(ScopeHistory* scope_history,
								   pair<int,int>& new_factor) {
	int factor_count = 0;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_OBS) {
			ObsNode* obs_node = (ObsNode*)it->second->node;
			for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
				if (obs_node->factors[f_index]->inputs.size() > 0) {
					uniform_int_distribution<int> select_distribution(0, factor_count);
					factor_count++;
					if (select_distribution(generator) == 0) {
						new_factor = {obs_node->id, f_index};
					}
				}
			}
		}
	}
}

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.node_context[l_index]);
	if (it != scope_history->node_histories.end()) {
		if (l_index == (int)input.scope_context.size()-1) {
			switch (it->second->node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						obs = 1.0;
						is_on = true;
					} else {
						obs = -1.0;
						is_on = true;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
					if (input.factor_index == -1) {
						obs = obs_node_history->obs_history[input.obs_index];
						is_on = true;
					} else {
						if (!obs_node_history->factor_initialized[input.factor_index]) {
							ObsNode* obs_node = (ObsNode*)it->second->node;
							double value = obs_node->factors[input.factor_index]->back_activate(scope_history);
							obs_node_history->factor_values[input.factor_index] = value;
							obs_node_history->factor_initialized[input.factor_index] = true;
						}
						obs = obs_node_history->factor_values[input.factor_index];
						is_on = true;
					}
				}
				break;
			}
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_input_helper(scope_node_history->scope_history,
							   input,
							   l_index+1,
							   obs,
							   is_on);
		}
	} else {
		obs = 0.0;
		is_on = false;
	}
}

void update_scores(ScopeHistory* scope_history,
				   double target_val,
				   SolutionWrapper* wrapper) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (wrapper->run_index != action_node->last_updated_run_index) {
					action_node->sum_score += target_val;
					action_node->sum_count++;

					action_node->last_updated_run_index = wrapper->run_index;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				update_scores(scope_node_history->scope_history,
							  target_val,
							  wrapper);

				if (wrapper->run_index != scope_node->last_updated_run_index) {
					scope_node->sum_score += target_val;
					scope_node->sum_count++;

					scope_node->last_updated_run_index = wrapper->run_index;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				if (branch_node_history->is_branch) {
					if (wrapper->run_index != branch_node->branch_last_updated_run_index) {
						branch_node->branch_sum_score += target_val;
						branch_node->branch_sum_count++;

						branch_node->branch_last_updated_run_index = wrapper->run_index;
					}
				} else {
					if (wrapper->run_index != branch_node->original_last_updated_run_index) {
						branch_node->original_sum_score += target_val;
						branch_node->original_sum_count++;

						branch_node->original_last_updated_run_index = wrapper->run_index;
					}
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (wrapper->run_index != obs_node->last_updated_run_index) {
					obs_node->sum_score += target_val;
					obs_node->sum_count++;

					obs_node->last_updated_run_index = wrapper->run_index;
				}
			}
			break;
		}
	}
}
