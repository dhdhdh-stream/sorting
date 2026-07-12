#include "solution_helpers.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_dependencies_helper(ScopeHistory* scope_history,
								vector<int>& curr_context,
								vector<int>& curr_index,
								int& count,
								vector<int>& dependency,
								vector<int>& index) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			curr_context.push_back(h_it->first);
			curr_index.push_back(h_it->second->index);
			gather_dependencies_helper(scope_node_history->scope_history,
									   curr_context,
									   curr_index,
									   count,
									   dependency,
									   index);
			curr_context.pop_back();
			curr_index.pop_back();
		}

		uniform_int_distribution<int> distribution(0, count);
		count++;
		if (distribution(generator) == 0) {
			curr_context.push_back(h_it->first);
			curr_index.push_back(h_it->second->index);
			dependency = curr_context;
			index = curr_index;
			curr_context.pop_back();
			curr_index.pop_back();
		}
	}
}

void fetch_dependency_helper(ScopeHistory* scope_history,
							 vector<int>& dependency,
							 int l_index,
							 bool& is_hit,
							 vector<double>& obs) {
	map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(dependency[l_index]);
	if (it == scope_history->node_histories.end()) {
		is_hit = false;
	} else {
		if (l_index == (int)dependency.size()-1) {
			is_hit = true;
			obs = it->second->obs;
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_dependency_helper(scope_node_history->scope_history,
									dependency,
									l_index+1,
									is_hit,
									obs);
		}
	}
}
