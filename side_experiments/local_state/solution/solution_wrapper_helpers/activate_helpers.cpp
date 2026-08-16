#include "solution_wrapper.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "globals.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::init(vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(this->solution->starting_scope->num_states);
	this->states.back().setConstant(0.0);

	this->run_num_actions = 0;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);

	this->solution->starting_scope->start_activate(obs,
												   this);
}

pair<bool,int> SolutionWrapper::step(vector<double> obs) {
	if (this->node_context.back() != NULL
			&& this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		action_node->step_callback(obs,
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
				scope_node->exit_step(obs,
									  this);
			}
		} else {
			this->node_context.back()->step(obs,
											action,
											is_next,
											this);
		}
	}

	return {is_done, action};
}

void SolutionWrapper::end() {
	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();

	this->states.clear();
}
