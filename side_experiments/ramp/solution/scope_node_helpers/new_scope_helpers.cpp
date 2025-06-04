#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::new_scope_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	this->scope->experiment_activate(problem,
									 run_helper,
									 inner_scope_history);
	delete inner_scope_history;

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(
			this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
