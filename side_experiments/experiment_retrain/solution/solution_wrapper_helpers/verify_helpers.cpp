#include "solution_wrapper.h"

#include <iostream>

#include "action_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::verify_init(vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->starting_run_seed = this->verify_starting_run_seeds[0];
	this->verify_starting_run_seeds.erase(this->verify_starting_run_seeds.begin());
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	this->state = vector<double>(this->solution->num_states, 0.0);

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);

	this->solution->starting_scope->start_activate(obs,
												   this);
}

pair<bool,int> SolutionWrapper::verify_step(vector<double> obs) {
	if (this->node_context.back() != NULL
			&& this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		action_node->verify_step_callback(obs,
										  this);
	}

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
				scope_node->verify_exit_step(obs,
											 this);
			}
		} else {
			this->node_context.back()->verify_step(obs,
												  action,
												  is_next,
												  this);
		}
	}

	return {is_done, action};
}

void SolutionWrapper::verify_end() {
	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
}
