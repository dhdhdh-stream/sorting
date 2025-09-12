#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "helpers.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	double sum_vals = this->average_val;
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   this->inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - this->input_averages[i_index]) / this->input_standard_deviations[i_index];
			sum_vals += this->weights[i_index] * normalized_val;
		}
	}

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
	#else
	if (sum_vals >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	history->is_branch = is_branch;

	for (int f_index = 0; f_index < (int)this->impacted_factors.size(); f_index++) {
		this->parent->invalidate_factor(wrapper->scope_histories.back(),
										this->impacted_factors[f_index]);
	}

	history->num_actions_snapshot = wrapper->num_actions;

	if (is_branch) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		wrapper->node_context.back() = this->original_next_node;
	}

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			is_branch,
			wrapper);
	}
}
