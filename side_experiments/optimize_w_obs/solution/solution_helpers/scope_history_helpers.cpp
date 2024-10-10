#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_helper(ScopeHistory* scope_history,
							vector<int>& scope_context,
							vector<int>& node_context,
							int& node_count,
							pair<pair<vector<int>,vector<int>>,int>& new_input) {
	Scope* scope = scope_history->scope;

	uniform_int_distribution<int> obs_distribution(0, problem_type->num_obs()-1);
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = scope->nodes[it->first];
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					scope_context.push_back(scope->id);
					node_context.push_back(it->first);

					new_input = {{scope_context, node_context}, obs_distribution(generator)};

					scope_context.pop_back();
					node_context.pop_back();
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				scope_context.push_back(scope->id);
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
		}
	}
}

void fetch_input_helper(ScopeHistory* scope_history,
						pair<pair<vector<int>,vector<int>>,int>& input,
						int l_index,
						double& obs) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history
		->node_histories.find(input.first.second[l_index]);
	if (it != scope_history->node_histories.end()) {
		if (l_index == (int)input.first.first.size()-1) {
			if (input.second == -1) {
				obs = 1.0;
			} else {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				obs = action_node_history->obs_history[input.second];
			}
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_input_helper(scope_node_history->scope_history,
							   input,
							   l_index+1,
							   obs);
		}
	}
}
