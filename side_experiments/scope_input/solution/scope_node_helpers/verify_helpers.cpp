#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void ScopeNode::verify_activate(AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;

	map<Scope*, vector<Input>>::iterator it = this->parent->child_scope_inputs.find(this->scope);
	inner_scope_history->input_history = vector<double>(this->scope->num_inputs);
	for (int i_index = 0; this->scope->num_inputs; i_index++) {
		fetch_input(run_helper,
					scope_history,
					it->second[i_index],
					inner_scope_history->input_history[i_index]);
	}

	this->scope->verify_activate(problem,
								 run_helper,
								 inner_scope_history);

	curr_node = this->next_node;
}

#endif /* MDEBUG */