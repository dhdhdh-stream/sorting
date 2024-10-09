#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

ScopeNode* create_existing() {
	/**
	 * - always give raw actions a large weight
	 *   - existing scopes often learned to avoid certain patterns
	 *     - which can prevent innovation
	 */
	uniform_int_distribution<int> default_distribution(0, 1);
	if (default_distribution(generator) != 0) {
		uniform_int_distribution<int> possible_distribution(1, solution->scopes.size() + problem_type->num_possible_actions() - 1);
		int possible_index = possible_distribution(generator);
		if (possible_index < (int)solution->scopes.size()) {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->scope = solution->scopes[possible_index];

			return new_scope_node;
		}
	}

	return NULL;
}
