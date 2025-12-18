#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);

	inner_scope_history->pre_obs = wrapper->problem->get_observations();
}

void ScopeNode::experiment_exit_step(SolutionWrapper* wrapper) {
	wrapper->scope_histories.back()->post_obs = wrapper->problem->get_observations();

	if (wrapper->has_explore) {
		wrapper->scope_histories.back()->has_explore = true;
		if (wrapper->scope_histories.back()->scope->signal_experiment != NULL) {
			for (int i_index = 0; i_index < (int)wrapper->experiment_history->post_scope_histories.size(); i_index++) {
				wrapper->experiment_history->post_scope_histories[i_index].push_back(wrapper->scope_histories.back());
			}
		}
	}

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	}
}
