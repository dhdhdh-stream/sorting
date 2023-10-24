#include "branch_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "scale.h"
#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchNode::view_activate(int& curr_node_id,
							   Problem& problem,
							   vector<ContextLayer>& context,
							   int& exit_depth,
							   int& exit_node_id,
							   RunHelper& run_helper) {
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

	cout << "branch node #" << this->id << endl;

	if (matches_context) {
		if (this->recursion_protection
				&& run_helper.recursion_protection_flags.find(this) != run_helper.recursion_protection_flags.end()) {
			// is_branch == false
			curr_node_id = this->original_next_node_id;
			cout << "recursion" << endl;
		} else {
			if (this->branch_is_pass_through) {
				// is_branch == true
				curr_node_id = this->branch_next_node_id;
				cout << "pass_through" << endl;

				if (this->recursion_protection) {
					context.back().added_recursion_protection_flags.push_back(this);
					run_helper.recursion_protection_flags.insert(this);
				}
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

				cout << "original_score: " << original_score << endl;
				cout << "branch_score: " << branch_score << endl;

				bool is_branch;
				if (branch_score > original_score) {
					is_branch = true;
					cout << "branch" << endl;

					if (this->recursion_protection) {
						context.back().added_recursion_protection_flags.push_back(this);
						run_helper.recursion_protection_flags.insert(this);
					}
				} else {
					is_branch = false;
					cout << "original" << endl;
				}

				double obs_snapshot;
				if (is_branch) {
					obs_snapshot = -1.0;
				} else {
					obs_snapshot = 1.0;
				}

				for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
					if (this->state_is_local[n_index]) {
						map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
						if (it == context.back().local_state_vals.end()) {
							it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
						}
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_snapshot,
												it->second);
						cout << "local state #" << this->state_indexes[n_index] << ": " << it->second.val << endl;
					} else {
						map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
						if (it != context.back().input_state_vals.end()) {
							StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
							state_network->activate(obs_snapshot,
													it->second);
							cout << "input state #" << this->state_indexes[n_index] << ": " << it->second.val << endl;
						}
					}
				}

				if (is_branch) {
					curr_node_id = this->branch_next_node_id;
				} else {
					curr_node_id = this->original_next_node_id;
				}
			}
		}
	} else {
		// is_branch == false
		curr_node_id = this->original_next_node_id;
		cout << "context mismatch" << endl;
	}

	cout << endl;
}
