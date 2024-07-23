#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
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
		new_scope_node->index = solution->scopes[possible_index]->scope_node_index;
		solution->scopes[possible_index]->scope_node_index++;

		return new_scope_node;
	}

	return NULL;
}
