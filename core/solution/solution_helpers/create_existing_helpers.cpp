#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int MIN_NUM_IMPROVEMENTS = 20;

ScopeNode* create_existing() {
	vector<Scope*> possible_scopes;
	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		if (solution->scopes[s_index]->num_improvements >= MIN_NUM_IMPROVEMENTS) {
			possible_scopes.push_back(solution->scopes[s_index]);
		}
	}

	uniform_int_distribution<int> possible_distribution(0, possible_scopes.size() + problem_type->num_possible_actions() - 1);
	int possible_index = possible_distribution(generator);
	if (possible_index < (int)possible_scopes.size()) {
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = possible_scopes[possible_index];

		return new_scope_node;
	} else {
		return NULL;
	}
}
