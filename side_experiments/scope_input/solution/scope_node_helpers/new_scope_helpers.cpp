#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void ScopeNode::new_scope_activate(AbstractNode*& curr_node,
								   Problem* problem,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history,
								   vector<Input>& new_scope_inputs) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;

	inner_scope_history->input_history = vector<double>(this->scope->num_inputs);
	for (int i_index = 0; this->scope->num_inputs; i_index++) {
		fetch_input(run_helper,
					scope_history,
					new_scope_inputs[i_index],
					inner_scope_history->input_history[i_index]);
	}

	this->scope->activate(problem,
						  run_helper,
						  inner_scope_history);

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

#if defined(MDEBUG) && MDEBUG
void ScopeNode::new_scope_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		vector<Input>& new_scope_inputs) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;

	inner_scope_history->input_history = vector<double>(this->scope->num_inputs);
	for (int i_index = 0; this->scope->num_inputs; i_index++) {
		fetch_input(run_helper,
					scope_history,
					new_scope_inputs[i_index],
					inner_scope_history->input_history[i_index]);
	}

	this->scope->new_scope_capture_verify_activate(problem,
												   run_helper,
												   inner_scope_history);

	curr_node = this->next_node;

	/**
	 * - still need to check experiments
	 */
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
#endif /* MDEBUG */
