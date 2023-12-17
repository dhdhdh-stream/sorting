#include "branch_node.h"

#include <iostream>

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchNode::view_activate(bool& is_branch,
							   vector<ContextLayer>& context) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope->id
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node->id) {
				matches_context = false;
				break;
			}
		}
	}

	cout << "branch node #" << this->id << endl;

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;
			cout << "pass_through" << endl;
		} else {
			double original_score = this->original_score_mod;
			double branch_score = this->branch_score_mod;

			for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
				cout << "s_index: " << s_index << endl;
				if (this->decision_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;
							cout << "local normalized: " << normalized << endl;
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;
							cout << "local it->second.val: " << it->second.val << endl;
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
							cout << "input normalized: " << normalized << endl;
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;
							cout << "input it->second.val: " << it->second.val << endl;
						}
					}
				}
			}

			cout << "original_score: " << original_score << endl;
			cout << "branch_score: " << branch_score << endl;

			bool decision_is_branch;
			if (abs(branch_score - original_score) > DECISION_MIN_SCORE_IMPACT * this->decision_standard_deviation) {
				decision_is_branch = branch_score > original_score;
			} else {
				uniform_int_distribution<int> distribution(0, 1);
				decision_is_branch = distribution(generator);
			}

			if (decision_is_branch) {
				is_branch = true;
				cout << "branch" << endl;
			} else {
				is_branch = false;
				cout << "original" << endl;
			}
		}
	} else {
		is_branch = false;
		cout << "context mismatch" << endl;
	}

	cout << endl;
}
