#include "branch_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "scale.h"
#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchNode::activate(bool& is_branch,
						  vector<ContextLayer>& context) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope_id
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;
		} else {
			double original_score = this->original_score_mod;
			double branch_score = this->branch_score_mod;

			for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
				if (this->decision_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().local_state_vals.end()) {
						StateNetwork* last_network = it->second.last_network;
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
						StateNetwork* last_network = it->second.last_network;
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

			if (branch_score > original_score) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
	} else {
		is_branch = false;
	}
}
