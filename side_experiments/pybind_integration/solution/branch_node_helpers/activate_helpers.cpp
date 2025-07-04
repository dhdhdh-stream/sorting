#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::step(vector<double>& obs,
					  string& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
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
	if (sum_vals >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	history->is_branch = is_branch;

	if (is_branch) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		wrapper->node_context.back() = this->original_next_node;
	}
}
