#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 SolutionWrapper* wrapper) {
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
		return;
	}
	if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
		return;
	}

	if (this->is_ramp) {
		#if defined(MDEBUG) && MDEBUG
		uniform_int_distribution<int> ramp_distribution(-5, 20);
		#else
		// uniform_int_distribution<int> ramp_distribution(-50000, 200000);
		uniform_int_distribution<int> ramp_distribution(-100000, 400000);
		/**
		 * - make sure fully ramped up before update ends
		 */
		#endif /* MDEBUG */
		if (ramp_distribution(generator) >= wrapper->iters_since_update) {
			wrapper->node_context.back() = this->original_next_node;
			return;
		}
	}

	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	bool is_branch;
	this->original_network->activate(wrapper->state);
	this->branch_network->activate(wrapper->state);
	if (this->branch_network->output->acti_vals(0) >= this->original_network->output->acti_vals(0)) {
		is_branch = true;
	} else {
		is_branch = false;
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
		if (wrapper->run_type == RUN_TYPE_EXISTING) {
			this->consec_original = 0;
			this->consec_branch++;
		}

		if (wrapper->run_type == RUN_TYPE_EXISTING) {
			this->branch_network->activate(wrapper->partial_state);
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->branch_network);
			this->branch_network->save(score_network_history);
			wrapper->partial_network_histories.push_back(score_network_history);
		}

		wrapper->node_context.back() = this->branch_next_node;

		if (this->branch_experiment != NULL) {
			this->branch_experiment->experiment_check_activate(
				obs,
				wrapper);
		}
	} else {
		if (wrapper->run_type == RUN_TYPE_EXISTING) {
			this->consec_original++;
			this->consec_branch = 0;
		}

		if (wrapper->run_type == RUN_TYPE_EXISTING) {
			this->original_network->activate(wrapper->partial_state);
			ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->original_network);
			this->original_network->save(score_network_history);
			wrapper->partial_network_histories.push_back(score_network_history);
		}

		wrapper->node_context.back() = this->original_next_node;

		if (this->original_experiment != NULL) {
			this->original_experiment->experiment_check_activate(
				obs,
				wrapper);
		}
	}
}
