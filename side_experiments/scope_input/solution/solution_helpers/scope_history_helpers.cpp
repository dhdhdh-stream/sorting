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

using namespace std;

void gather_possible_helper(ScopeHistory* scope_history,
							vector<Scope*>& scope_context,
							vector<int>& node_context,
							int& node_count,
							Input& new_input) {
	Scope* scope = scope_history->scope;

	if (scope_context.size() == 0) {
		for (int i_index = 0; i_index < (int)scope->num_inputs; i_index++) {
			uniform_int_distribution<int> select_distribution(0, node_count);
			node_count++;
			if (select_distribution(generator) == 0) {
				new_input = Input();
				new_input.type = INPUT_TYPE_INPUT;
				new_input.factor_index = -1;
				new_input.obs_index = -1;
				new_input.input_index = i_index;
			}
		}
	}

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

					new_input = Input();
					new_input.type = INPUT_TYPE_OBS;
					new_input.scope_context = scope_context;
					new_input.node_context = node_context;
					new_input.factor_index = -1;
					new_input.obs_index = -1;
					new_input.input_index = -1;

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

					new_input = Input();
					new_input.type = INPUT_TYPE_OBS;
					new_input.scope_context = scope_context;
					new_input.node_context = node_context;
					new_input.factor_index = -1;
					new_input.obs_index = obs_distribution(generator);
					new_input.input_index = -1;

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

							new_input = Input();
							new_input.type = INPUT_TYPE_OBS;
							new_input.scope_context = scope_context;
							new_input.node_context = node_context;
							new_input.factor_index = f_index;
							new_input.obs_index = -1;
							new_input.input_index = -1;

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
								   Input& new_factor) {
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
						new_factor = Input();
						new_factor.type = INPUT_TYPE_OBS;
						new_factor.scope_context = {scope_history->scope};
						new_factor.node_context = {it->first};
						new_factor.factor_index = f_index;
						new_factor.obs_index = -1;
						new_factor.input_index = -1;
					}
				}
			}
		}
	}
}

void fetch_input_helper(RunHelper& run_helper,
						ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& val) {
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
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
					if (input.factor_index == -1) {
						val = obs_node_history->obs_history[input.obs_index];
					} else {
						if (!obs_node_history->factor_initialized[input.factor_index]) {
							ObsNode* obs_node = (ObsNode*)it->second->node;
							double value = obs_node->factors[input.factor_index]->back_activate(
								run_helper,
								scope_history);
							obs_node_history->factor_values[input.factor_index] = value;
							obs_node_history->factor_initialized[input.factor_index] = true;
						}
						val = obs_node_history->factor_values[input.factor_index];
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
							   val);
		}
	}
}

void fetch_input(RunHelper& run_helper,
				 ScopeHistory* scope_history,
				 Input& input,
				 double& val) {
	switch (input.type) {
	case INPUT_TYPE_REMOVED:
		val = 0.0;
		break;
	case INPUT_TYPE_OBS:
		fetch_input_helper(run_helper,
						   scope_history,
						   input,
						   0,
						   val);
		break;
	case INPUT_TYPE_INPUT:
		val = scope_history->input_history[input.input_index];
		break;
	}
}
