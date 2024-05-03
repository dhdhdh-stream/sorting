#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

ScopeNode* create_existing() {
	uniform_int_distribution<int> possible_distribution(0, solution->scopes.size() + problem_type->num_possible_actions() - 1);
	int possible_index = possible_distribution(generator);
	if (solution->state == SOLUTION_STATE_TRAVERSE) {
		if (possible_index < (int)solution->scopes.size()) {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->scope = solution->scopes[possible_index];

			return new_scope_node;
		}
	} else {
		if (possible_index < (int)solution->scopes.size()-1) {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->scope = solution->scopes[possible_index];

			return new_scope_node;
		}
	}

	return NULL;
}

InfoScopeNode* create_existing_info() {
	uniform_int_distribution<int>possible_distribution(0, solution->info_scopes.size()-1);
	int possible_index = possible_distribution(generator);

	InfoScopeNode* new_scope_node = new InfoScopeNode();
	new_scope_node->scope = solution->info_scopes[possible_index];

	return new_scope_node;
}
