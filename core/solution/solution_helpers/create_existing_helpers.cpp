#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

ScopeNode* create_existing(Scope* experiment_scope) {
	// vector<Scope*> possible_scopes;
	// for (map<int, Scope*>::iterator it = solution->scopes.begin();
	// 		it != solution->scopes.end(); it++) {
	// 	if (it->second->id != 0) {
	// 		possible_scopes.push_back(it->second);
	// 	}
	// }
	vector<Scope*> possible_scopes;
	if (experiment_scope->id == 2) {
		possible_scopes = vector<Scope*>{solution->scopes[0]};
	} else if (experiment_scope->id == 1) {
		// possible_scopes = vector<Scope*>{solution->scopes[0], solution->scopes[2]};
		possible_scopes = vector<Scope*>{solution->scopes[0]};
	} else if (experiment_scope->id == 3) {
		possible_scopes = vector<Scope*>{solution->scopes[0], solution->scopes[2]};
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
