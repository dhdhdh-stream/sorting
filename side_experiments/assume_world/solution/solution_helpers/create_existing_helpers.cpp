#include "solution_helpers.h"

#include <iostream>

using namespace std;

ScopeNode* create_existing() {
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];

	uniform_int_distribution<int> possible_distribution(1, solution->scopes.size() + problem_type->num_possible_actions() - 1);
	int possible_index = possible_distribution(generator);
	if (possible_index < (int)solution->scopes.size()) {
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = solution->scopes[possible_index];

		return new_scope_node;
	}

	return NULL;
}
