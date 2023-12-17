#include "branch_node.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "retrain_branch_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(bool& is_branch,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (context[context.size()-this->branch_scope_context.size()+c_index].scope	== NULL			// OuterExperiment edge case
					|| this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope->id
					|| context[context.size()-this->branch_scope_context.size()+c_index].node == NULL	// explore edge case
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node->id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;
		} else {
			if (this->experiment != NULL) {
				bool is_selected = this->experiment->activate(is_branch,
															  problem,
															  context,
															  run_helper);

				if (is_selected) {
					return;
				}
			}

			double original_score = this->original_score_mod;
			double branch_score = this->branch_score_mod;

			for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
				if (this->decision_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;
						}
					}
				}
			}

			#if defined(MDEBUG) && MDEBUG
			bool decision_is_branch;
			if (run_helper.curr_run_seed%2 == 0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
			#else
			bool decision_is_branch;
			if (abs(branch_score - original_score) > DECISION_MIN_SCORE_IMPACT * this->decision_standard_deviation) {
				decision_is_branch = branch_score > original_score;
			} else {
				uniform_int_distribution<int> distribution(0, 1);
				decision_is_branch = distribution(generator);
			}
			#endif /* MDEBUG */

			if (decision_is_branch) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
	} else {
		is_branch = false;
	}
}
