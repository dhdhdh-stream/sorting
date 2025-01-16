#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void update_scores(ScopeHistory* scope_history,
				   double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		it->second->node->num_measure++;
		it->second->node->sum_score += target_val;

		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

			update_scores(scope_node_history->scope_history,
						  target_val);
		}
	}
}

void gather_possible_helper(ScopeHistory* scope_history,
							vector<Scope*>& scope_context,
							vector<int>& node_context,
							int& node_count,
							pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>& new_input) {
	Scope* scope = scope_history->scope;

	uniform_int_distribution<int> obs_distribution(0, problem_type->num_obs()-1);
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = scope->nodes[it->first];
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
									   new_input);

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

					new_input = {{scope_context, node_context}, {-1, -1}};

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

					new_input = {{scope_context, node_context}, {-1, obs_distribution(generator)}};

					scope_context.pop_back();
					node_context.pop_back();
				}

				for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
					if (obs_node->factors[f_index]->inputs.size() > 1) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							scope_context.push_back(scope);
							node_context.push_back(it->first);

							new_input = {{scope_context, node_context}, {f_index, -1}};

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

void gather_factors(RunHelper& run_helper,
					ScopeHistory* scope_history,
					map<pair<int,int>, double>& factors) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_OBS) {
			ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
			ObsNode* obs_node = (ObsNode*)it->second->node;

			for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
				if (obs_node->factors[f_index]->inputs.size() > 1) {
					if (!obs_node_history->factor_initialized[f_index]) {
						double value = obs_node->factors[f_index]->back_activate(
							run_helper,
							scope_history);
						obs_node_history->factor_values[f_index] = value;
						obs_node_history->factor_initialized[f_index] = true;
					}
					factors[{obs_node->id, f_index}] = obs_node_history->factor_values[f_index];
				}
			}
		}
	}
}

void fetch_factor_helper(RunHelper& run_helper,
						 ScopeHistory* scope_history,
						 pair<int,int> factor,
						 double& val) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(factor.first);
	if (it != scope_history->node_histories.end()) {
		ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
		if (!obs_node_history->factor_initialized[factor.second]) {
			ObsNode* obs_node = (ObsNode*)obs_node_history->node;
			double value = obs_node->factors[factor.second]->back_activate(
				run_helper,
				scope_history);
			obs_node_history->factor_values[factor.second] = value;
			obs_node_history->factor_initialized[factor.second] = true;
		}
		val = obs_node_history->factor_values[factor.second];
	} else {
		val = 0.0;
	}
}

void fetch_input_helper(RunHelper& run_helper,
						ScopeHistory* scope_history,
						pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>& input,
						int l_index,
						double& obs) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.first.second[l_index]);
	if (it != scope_history->node_histories.end()) {
		if (l_index == (int)input.first.first.size()-1) {
			switch (it->second->node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						obs = 1.0;
					} else {
						obs = -1.0;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
					if (input.second.first == -1) {
						obs = obs_node_history->obs_history[input.second.second];
					} else {
						if (!obs_node_history->factor_initialized[input.second.first]) {
							ObsNode* obs_node = (ObsNode*)it->second->node;
							double value = obs_node->factors[input.second.first]->back_activate(
								run_helper,
								scope_history);
							obs_node_history->factor_values[input.second.first] = value;
							obs_node_history->factor_initialized[input.second.first] = true;
						}
						obs = obs_node_history->factor_values[input.second.first];
					}
				}
				break;
			}
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_input_helper(run_helper,
							   scope_node_history->scope_history,
							   input,
							   l_index+1,
							   obs);
		}
	}
}
