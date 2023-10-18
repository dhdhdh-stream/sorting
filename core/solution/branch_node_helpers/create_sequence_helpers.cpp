#include "branch_node.h"

#include "branch_stub_node.h"
#include "scale.h"
#include "scope_node.h"
#include "sequence.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchNode::create_sequence_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		Sequence* new_sequence,
		vector<map<pair<bool,int>, int>>& state_mappings,
		int& new_num_input_states,
		vector<AbstractNode*>& new_nodes,
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

	if (matches_context) {
		if (this->recursion_protection
				&& run_helper.recursion_protection_flags.find(this) != run_helper.recursion_protection_flags.end()) {
			is_branch = false;
		} else {
			if (this->branch_is_pass_through) {
				is_branch = true;

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

				if (branch_score > original_score) {
					is_branch = true;

					if (this->recursion_protection) {
						context.back().added_recursion_protection_flags.push_back(this);
						run_helper.recursion_protection_flags.insert(this);
					}
				} else {
					is_branch = false;
				}

				double obs_snapshot;
				if (is_branch) {
					obs_snapshot = -1.0;
				} else {
					obs_snapshot = 1.0;
				}

				BranchStubNode* new_node = new BranchStubNode();
				new_nodes.push_back(new_node);
				new_node->was_branch = is_branch;

				for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
					if (this->state_is_local[n_index]) {
						map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
						if (it == context.back().local_state_vals.end()) {
							it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
						}
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_snapshot,
												it->second);

						new_node->state_is_local.push_back(false);
						map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({true, this->state_indexes[n_index]});
						if (new_state_it != state_mappings.back().end()) {
							new_node->state_indexes.push_back(new_state_it->second);
						} else {
							int new_state_index;
							state_mappings.back()[{true, this->state_indexes[n_index]}] = new_num_input_states;
							new_state_index = new_num_input_states;
							new_num_input_states++;

							new_node->state_indexes.push_back(new_state_index);

							new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
							new_sequence->input_inner_indexes.push_back(new_state_index);
							new_sequence->input_scope_depths.push_back(-1);
							new_sequence->input_outer_is_local.push_back(-1);
							new_sequence->input_outer_indexes.push_back(-1);
							new_sequence->input_init_vals.push_back(0.0);
						}
						new_node->state_defs.push_back(this->state_defs[n_index]);
						new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
					} else {
						map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
						if (it != context.back().input_state_vals.end()) {
							StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
							state_network->activate(obs_snapshot,
													it->second);

							map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({false, this->state_indexes[n_index]});
							// new_state_it != state_mappings.back().end()
							new_node->state_is_local.push_back(false);
							new_node->state_indexes.push_back(new_state_it->second);
							new_node->state_defs.push_back(this->state_defs[n_index]);
							new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
						}
					}
				}

				// don't worry about score_state

				// don't increment curr_num_nodes for BranchNode
			}
		}
	} else {
		is_branch = false;
	}
}
