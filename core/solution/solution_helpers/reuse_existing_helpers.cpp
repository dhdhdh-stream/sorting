#include "solution_helpers.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

ScopeNode* reuse_existing() {
	uniform_int_distribution<int> flat_distribution(0, 9);
	if (flat_distribution(generator) == 0) {
		return NULL;
	}

	/**
	 * - even distribution
	 */
	uniform_int_distribution<int> scope_distribution(0, (int)solution->scopes.size() + problem_type->num_actions() - 1);
	int scope_index = scope_distribution(generator);
	if (scope_index > (int)solution->scopes.size()-1) {
		return NULL;
	}

	Scope* existing_scope = next(solution->scopes.begin(), scope_index)->second;
	if (existing_scope->nodes.size() == 1) {
		/**
		 * - starting edge case
		 */
		return NULL;
	}

	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->scope = existing_scope;

	return new_scope_node;
}
