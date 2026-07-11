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
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		this->init_networks[n_index]->activate(wrapper->state,
											   obs);
		if (!wrapper->should_explore) {
			InitNetworkHistory* init_network_history = new InitNetworkHistory(this->init_networks[n_index]);
			this->init_networks[n_index]->save(init_network_history);
			wrapper->network_histories.push_back(init_network_history);
		}
	}

	// uniform_int_distribution<int> on_distribution(0, this->ramp_num_gears);
	uniform_int_distribution<int> on_distribution(0, this->ramp_num_gears-1);
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
	} else if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
	} else if (this->ramp < this->ramp_num_gears
			&& this->ramp < on_distribution(generator)) {
		wrapper->node_context.back() = this->original_next_node;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		BranchNodeHistory* history = new BranchNodeHistory(this);
		history->index = (int)scope_history->node_histories.size();
		scope_history->node_histories[this->id] = history;

		if (this->dependencies.size() > 0) {
			history->state = wrapper->state;
			history->obs = obs;
		}

		bool is_branch;
		this->original_network->activate(wrapper->state);
		this->branch_network->activate(wrapper->state);
		if (this->branch_network->output->acti_vals(0) >= this->original_network->output->acti_vals(0)) {
			is_branch = true;

			if (!wrapper->should_explore) {
				this->consec_original = 0;
				this->consec_branch++;
			}
		} else {
			is_branch = false;

			if (!wrapper->should_explore) {
				this->consec_original++;
				this->consec_branch = 0;
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
			if (!wrapper->should_explore) {
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->branch_network);
				this->branch_network->save(score_network_history);
				wrapper->network_histories.push_back(score_network_history);
			}

			wrapper->node_context.back() = this->branch_next_node;

			if (this->branch_experiment != NULL) {
				this->branch_experiment->experiment_check_activate(
					obs,
					wrapper);
			}
		} else {
			if (!wrapper->should_explore) {
				ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->original_network);
				this->original_network->save(score_network_history);
				wrapper->network_histories.push_back(score_network_history);
			}

			wrapper->node_context.back() = this->original_next_node;

			if (this->original_experiment != NULL) {
				this->original_experiment->experiment_check_activate(
					obs,
					wrapper);
			}
		}
	}
}
