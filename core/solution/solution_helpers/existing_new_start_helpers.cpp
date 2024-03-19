#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

ScopeNode* existing_new_start(Scope* parent_scope,
							  RunHelper& run_helper) {
	if (parent_scope->nodes.size() == 1) {
		/**
		 * - starting edge case
		 */
		return NULL;
	}

	uniform_int_distribution<int> distribution(0, parent_scope->nodes.size()-1);
	AbstractNode* new_starting_node = next(parent_scope->nodes.begin(), distribution(generator))->second;

	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->starting_node_id = new_starting_node->id;
	new_scope_node->starting_node = new_starting_node;
	new_scope_node->scope = parent_scope;

	return new_scope_node;
}
