#include "solution_helpers.h"

#include <iostream>

#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

double calc_reward_signal(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	double sum_vals = scope->score_average_val;
	for (int i_index = 0; i_index < (int)scope->score_inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   scope->score_inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - scope->score_input_averages[i_index]) / scope->score_input_standard_deviations[i_index];
			sum_vals += scope->score_weights[i_index] * normalized_val;
		}
	}

	return sum_vals;
}

void attach_explore_helper(ScopeHistory* scope_history,
						   double target_val) {
	Scope* scope = scope_history->scope;
	scope->explore_scope_histories.push_back(scope_history);
	scope->explore_target_val_histories.push_back(target_val);

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			attach_explore_helper(scope_node_history->scope_history,
								  target_val);
		}
	}
}

void update_reward_signals(SolutionWrapper* wrapper) {
	for (int h_index = 0; h_index < (int)wrapper->solution->explore_scope_histories.size(); h_index++) {
		attach_explore_helper(wrapper->solution->explore_scope_histories[h_index],
							  wrapper->solution->explore_target_val_histories[h_index]);
	}

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		train_score(wrapper->solution->scopes[s_index]);
	}

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		scope->explore_scope_histories.clear();
		scope->explore_target_val_histories.clear();
	}

	for (int h_index = 0; h_index < (int)wrapper->solution->explore_scope_histories.size(); h_index++) {
		delete wrapper->solution->explore_scope_histories[h_index];
	}
	wrapper->solution->explore_scope_histories.clear();
	wrapper->solution->explore_target_val_histories.clear();
}
