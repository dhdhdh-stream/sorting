#include "solution_helpers.h"

#include "action_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void fetch_dependency_helper(ScopeHistory* scope_history,
							 vector<int>& dependencies,
							 int l_index,
							 bool& is_hit,
							 vector<double>& state,
							 vector<double>& obs) {
	if (dependencies[l_index] == -1) {
		is_hit = true;
		state = scope_history->state;
		obs = scope_history->obs;
	} else {
		map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(dependencies[l_index]);
		if (it == scope_history->node_histories.end()) {
			is_hit = false;
		} else {
			if (l_index == (int)dependencies.size()-1) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				is_hit = true;
				state = action_node_history->state;
				obs = action_node_history->obs;
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				fetch_dependency_helper(scope_node_history->scope_history,
										dependencies,
										l_index+1,
										is_hit,
										state,
										obs);
			}
		}
	}
}
