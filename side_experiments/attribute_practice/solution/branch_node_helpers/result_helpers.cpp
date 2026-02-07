#include "branch_node.h"

#include "abstract_experiment.h"
#include "constants.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::result_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	this->network->activate(obs);

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
	#else
	if (this->network->output->acti_vals[0] >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	if (is_branch) {
		wrapper->result_node_context.back() = this->branch_next_node;
	} else {
		wrapper->result_node_context.back() = this->original_next_node;
	}

	if (this->experiment != NULL) {
		this->experiment->result_check_activate(
			this,
			is_branch,
			wrapper);
	}
}
