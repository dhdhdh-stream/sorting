#include "solution_helpers.h"

#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void attach_explore_helper(ScopeHistory* scope_history,
						   double target_val) {
	Scope* scope = scope_history->scope;
	scope->explore_scope_histories.push_back(scope_history);
	scope->explore_target_val_histories.push_back(target_val);

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			attach_explore_helper(scope_node_history->scope_history,
								  target_val);
		}
	}
}

void update_reward_signals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		scope->explore_scope_histories.clear();
		scope->explore_target_val_histories.clear();
	}

	for (int h_index = 0; h_index < (int)wrapper->solution->explore_scope_histories.size(); h_index++) {
		attach_explore_helper(wrapper->solution->explore_scope_histories[h_index],
							  wrapper->solution->explore_target_val_histories[h_index]);
	}


}
