#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "problem.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
		}
	}

	// // temp
	// if (wrapper->starting_run_seed == 131) {
	// 	cout << "this->id: " << this->id << endl;
	// 	cout << "wrapper->state:";
	// 	for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
	// 		cout << " " << wrapper->state[s_index];
	// 	}
	// 	cout << endl;
	// 	cout << "obs:";
	// 	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
	// 		cout << " " << obs[o_index];
	// 	}
	// 	cout << endl;
	// }

	if (this->verify_states.size() > 0) {
		if (this->verify_states[0] != wrapper->state) {
			#if defined(MDEBUG) && MDEBUG
			cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
			#endif /* MDEBUG */
			wrapper->problem->print();
			cout << "this->verify_states[0]:";
			for (int s_index = 0; s_index < (int)this->verify_states[0].size(); s_index++) {
				cout << " " << this->verify_states[0][s_index];
			}
			cout << endl;
			cout << "wrapper->state:";
			for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
				cout << " " << wrapper->state[s_index];
			}
			cout << endl;
			throw invalid_argument("this->verify_states[0] != wrapper->state");
		}
		this->verify_states.erase(this->verify_states.begin());
	}

	// uniform_int_distribution<int> on_distribution(0, this->ramp_num_gears);
	uniform_int_distribution<int> on_distribution(0, this->ramp_num_gears-1);
	if (this->consec_original >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->original_next_node;
	} else if (this->consec_branch >= CONSEC_DEPRECATE_LIMIT) {
		wrapper->node_context.back() = this->branch_next_node;
	} else {
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
			wrapper->node_context.back() = this->branch_next_node;
		} else {
			wrapper->node_context.back() = this->original_next_node;
		}
	}
}
