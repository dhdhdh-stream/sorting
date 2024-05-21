#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

ScopeNode* create_existing() {
	uniform_int_distribution<int> possible_distribution(1, solution->scopes.size() + problem_type->num_possible_actions() - 1);
	int possible_index = possible_distribution(generator);
	if (possible_index < (int)solution->scopes.size()) {
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = solution->scopes[possible_index];

		return new_scope_node;
	}

	return NULL;
}

InfoScope* get_existing_info_scope() {
	uniform_int_distribution<int> non_null_distribution(0, 4);
	if (!non_null_distribution(generator) == 0) {
		return NULL;
	} else {
		vector<InfoScope*> possible_info_scopes;
		for (int i_index = 0; i_index < (int)solution->info_scopes.size(); i_index++) {
			if (solution->info_scopes[i_index]->state == INFO_SCOPE_STATE_NA) {
				possible_info_scopes.push_back(solution->info_scopes[i_index]);
			}
		}

		if (possible_info_scopes.size() == 0) {
			return NULL;
		}

		uniform_int_distribution<int> possible_distribution(0, possible_info_scopes.size()-1);
		return possible_info_scopes[possible_distribution(generator)];
	}
}

InfoScopeNode* create_existing_info_scope_node() {
	InfoScope* info_scope = get_existing_info_scope();
	if (info_scope == NULL) {
		return NULL;
	} else {
		InfoScopeNode* new_scope_node = new InfoScopeNode();
		new_scope_node->scope = info_scope;

		return new_scope_node;
	}
}
