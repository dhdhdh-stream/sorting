#include "solution_helpers.h"

using namespace std;

ScopeNode* reuse_existing(Problem* problem) {
	/**
	 * - even distribution
	 */
	uniform_int_distribution<int> scope_distribution(0, (int)solution->scopes.size() + problem->num_actions() - 1);
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
