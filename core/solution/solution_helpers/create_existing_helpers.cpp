#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "info_scope.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

ScopeNode* create_existing(Scope* parent_scope) {
	uniform_int_distribution<int> possible_distribution(1, solution->scopes.size() + problem_type->num_possible_actions() - 1);
	int possible_index = possible_distribution(generator);
	if (possible_index < (int)solution->scopes.size()) {
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = solution->scopes[possible_index];

		return new_scope_node;
	}

	return NULL;
}

InfoScope* get_existing_info_scope(Scope* parent_scope) {
	uniform_int_distribution<int> non_null_distribution(0, 4);
	if (solution->info_scopes.size() == 0
			|| non_null_distribution(generator) != 0) {
		return NULL;
	} else {
		uniform_int_distribution<int> info_scope_distribution(0, (int)solution->info_scopes.size()-1);
		return solution->info_scopes[info_scope_distribution(generator)];
	}
}
