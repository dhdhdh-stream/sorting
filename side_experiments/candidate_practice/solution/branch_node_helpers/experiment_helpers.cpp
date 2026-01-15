#include "branch_node.h"

#include "abstract_experiment.h"
#include "constants.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"
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

	bool is_branch;
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		this->networks[n_index]->activate(obs);
		double output = this->networks[n_index]->output->acti_vals[0];
		if (output >= 0.0) {
			#if defined(MDEBUG) && MDEBUG
			if (output >= this->above_min[n_index] || rand()%2 == 0) {
			#else
			if (output >= this->above_min[n_index]) {
			#endif /* MDEBUG */
				is_branch = true;
				break;
			}
		} else {
			#if defined(MDEBUG) && MDEBUG
			if (output <= this->below_max[n_index] || rand()%2 == 0) {
			#else
			if (output <= this->below_max[n_index]) {
			#endif /* MDEBUG */
				is_branch = false;
				break;
			}
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
	#endif /* MDEBUG */

	history->is_branch = is_branch;

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

void BranchNode::compare_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->compare_scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	bool is_branch;
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		this->networks[n_index]->activate(obs);
		double output = this->networks[n_index]->output->acti_vals[0];
		if (output >= 0.0) {
			#if defined(MDEBUG) && MDEBUG
			if (output >= this->above_min[n_index] || rand()%2 == 0) {
			#else
			if (output >= this->above_min[n_index]) {
			#endif /* MDEBUG */
				is_branch = true;
				break;
			}
		} else {
			#if defined(MDEBUG) && MDEBUG
			if (output <= this->below_max[n_index] || rand()%2 == 0) {
			#else
			if (output <= this->below_max[n_index]) {
			#endif /* MDEBUG */
				is_branch = false;
				break;
			}
		}
	}

	history->is_branch = is_branch;

	if (is_branch) {
		wrapper->compare_node_context.back() = this->branch_next_node;
	} else {
		wrapper->compare_node_context.back() = this->original_next_node;
	}

	if (this->experiment != NULL) {
		this->experiment->compare_check_activate(
			this,
			is_branch,
			wrapper);
	}
}
