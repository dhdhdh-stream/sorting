#include "helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"

using namespace std;

void gather_possible_exits(vector<pair<int,AbstractNode*>>& possible_exits,
						   vector<ContextLayer>& context,
						   vector<int>& scope_context,
						   vector<int>& node_context) {
	for (int l_index = 0; l_index < (int)scope_context.size(); l_index++) {
		Scope* scope = context[context.size() - scope_context.size() + l_index].scope;
		scope->random_exit_activate(node_context[l_index],
									scope_context,
									node_context,
									scope_context.size()-1 - l_index,
									possible_exits);
	}
}
