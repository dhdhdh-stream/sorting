#include "solution_helpers.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_dependencies_helper(ScopeHistory* scope_history,
								vector<int>& curr_context,
								int& count,
								vector<int>& dependency) {
	{
		uniform_int_distribution<int> distribution(0, count);
		count++;
		if (distribution(generator) == 0) {
			curr_context.push_back(-1);
			dependency = curr_context;
			curr_context.pop_back();
		}
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			curr_context.push_back(h_it->first);
			gather_dependencies_helper(scope_node_history->scope_history,
									   curr_context,
									   count,
									   dependency);
			curr_context.pop_back();
		}

		uniform_int_distribution<int> distribution(0, count);
		count++;
		if (distribution(generator) == 0) {
			curr_context.push_back(h_it->first);
			dependency = curr_context;
			curr_context.pop_back();
		}
	}
}

void set_dependency_helper(Scope* scope,
						   vector<int>& dependency,
						   int l_index,
						   AbstractExperiment* experiment) {
	if (l_index == (int)dependency.size()-1) {
		if (dependency[l_index] == -1) {
			scope->dependencies.push_back(experiment);
		} else {
			scope->nodes[dependency[l_index]]->dependencies.push_back(experiment);
		}
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		set_dependency_helper(scope_node->scope,
							  dependency,
							  l_index+1,
							  experiment);
	}
}

void clear_dependency_helper(Scope* scope,
							 vector<int>& dependency,
							 int l_index,
							 AbstractExperiment* experiment) {
	if (l_index == (int)dependency.size()-1) {
		if (dependency[l_index] == -1) {
			for (int d_index = 0; d_index < (int)scope->dependencies.size(); d_index++) {
				if (scope->dependencies[d_index] == experiment) {
					scope->dependencies.erase(scope->dependencies.begin() + d_index);
					break;
				}
			}
		} else {
			AbstractNode* node = scope->nodes[dependency[l_index]];
			for (int d_index = 0; d_index < (int)node->dependencies.size(); d_index++) {
				if (node->dependencies[d_index] == experiment) {
					node->dependencies.erase(node->dependencies.begin() + d_index);
					break;
				}
			}
		}
	} else {
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[dependency[l_index]];
		clear_dependency_helper(scope_node->scope,
								dependency,
								l_index+1,
								experiment);
	}
}

void fetch_dependency_helper(ScopeHistory* scope_history,
							 vector<int>& dependency,
							 int l_index,
							 bool& is_hit,
							 vector<double>& state,
							 vector<double>& obs) {
	if (dependency[l_index] == -1) {
		is_hit = true;
		state = scope_history->state;
		obs = scope_history->obs;
	} else {
		map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(dependency[l_index]);
		if (it == scope_history->node_histories.end()) {
			is_hit = false;
		} else {
			if (l_index == (int)dependency.size()-1) {
				is_hit = true;
				state = it->second->state;
				obs = it->second->obs;
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				fetch_dependency_helper(scope_node_history->scope_history,
										dependency,
										l_index+1,
										is_hit,
										state,
										obs);
			}
		}
	}
}
