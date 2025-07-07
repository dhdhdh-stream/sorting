#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_step(vector<double>& obs,
							 int& action,
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

			if (this->verify_key != NULL) {
				cout << i_index << ": " << val << endl;
				this->inputs[i_index].print();
				cout << "this->input_averages[i_index]: " << this->input_averages[i_index] << endl;
				cout << "this->input_standard_deviations[i_index]: " << this->input_standard_deviations[i_index] << endl;
			}
		}
	}

	if (this->verify_key != NULL) {
		cout << "this->id: " << this->id << endl;

		cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
		cout << "wrapper->curr_run_seed: " << wrapper->curr_run_seed << endl;
		wrapper->problem->print();

		if (this->verify_scores[0] != sum_vals) {
			cout << "this->verify_scores[0]: " << this->verify_scores[0] << endl;
			cout << "sum_vals: " << sum_vals << endl;

			cout << "seed: " << seed << endl;

			throw invalid_argument("branch node verify fail");
		}

		this->verify_scores.erase(this->verify_scores.begin());
	}

	bool is_branch;
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);

	history->is_branch = is_branch;

	for (int f_index = 0; f_index < (int)this->impacted_factors.size(); f_index++) {
		this->parent->invalidate_factor(wrapper->scope_histories.back(),
										this->impacted_factors[f_index]);
	}

	if (is_branch) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
		wrapper->node_context.back() = this->original_next_node;
	}
}

#endif /* MDEBUG */