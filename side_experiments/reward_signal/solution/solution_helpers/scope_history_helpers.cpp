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
	scope_history->scope->existing_scope_histories.push_back(scope_history);
	scope_history->scope->existing_target_val_histories.push_back(target_val);

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (wrapper->run_index != action_node->last_updated_run_index) {
					action_node->sum_score += target_val;
					action_node->sum_hits++;

					action_node->last_updated_run_index = wrapper->run_index;
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
							  wrapper);

				if (wrapper->run_index != scope_node->last_updated_run_index) {
					scope_node->sum_score += target_val;
					scope_node->sum_hits++;

					scope_node->last_updated_run_index = wrapper->run_index;
				}
				scope_node->sum_instances++;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				if (branch_node_history->is_branch) {
					if (wrapper->run_index != branch_node->branch_last_updated_run_index) {
						branch_node->branch_sum_score += target_val;
						branch_node->branch_sum_hits++;

						branch_node->branch_last_updated_run_index = wrapper->run_index;
					}
					branch_node->branch_sum_instances++;
				} else {
					if (wrapper->run_index != branch_node->original_last_updated_run_index) {
						branch_node->original_sum_score += target_val;
						branch_node->original_sum_hits++;

						branch_node->original_last_updated_run_index = wrapper->run_index;
					}
					branch_node->original_sum_instances++;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;

				obs_node->obs_val_histories.push_back(obs_node_history->obs_history);

				if (wrapper->run_index != obs_node->last_updated_run_index) {
					obs_node->sum_score += target_val;
					obs_node->sum_hits++;

					obs_node->last_updated_run_index = wrapper->run_index;
				}
				obs_node->sum_instances++;
			}
			break;
		}
	}
}

void analyze_input(Input& input,
				   vector<ScopeHistory*>& scope_histories,
				   InputData& input_data) {
	vector<double> vals;
	int num_is_on = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_histories[h_index],
						   input,
						   0,
						   val,
						   is_on);
		if (is_on) {
			vals.push_back(val);
			num_is_on++;
		}
	}

	input_data.hit_percent = (double)num_is_on / (double)scope_histories.size();
	if (input_data.hit_percent >= MIN_CONSIDER_HIT_PERCENT) {
		double sum_vals = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_vals += vals[v_index];
		}
		input_data.average = sum_vals / (double)vals.size();

		double sum_variance = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_variance += (input_data.average - vals[v_index]) * (input_data.average - vals[v_index]);
		}
		input_data.standard_deviation = sqrt(sum_variance / (double)vals.size());
	}
}
