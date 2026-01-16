#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void fetch_histories_helper(ScopeHistory* scope_history,
							ObsNode* node_context,
							vector<vector<double>>& obs_histories) {
	Scope* scope = scope_history->scope;

	if (scope == node_context->parent) {
		map<int, AbstractNodeHistory*>::iterator match_it = scope_history->node_histories.find(node_context->id);
		if (match_it != scope_history->node_histories.end()) {
			ObsNodeHistory* obs_node_history = (ObsNodeHistory*)match_it->second;
			obs_histories.push_back(obs_node_history->obs_history);
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
											   node_context,
											   obs_histories);
					}

					it++;
				}
			}
		}
	}
}
