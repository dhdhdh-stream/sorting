#include "branch_node.h"

#include "state.h"
#include "state_network.h"

using namespace std;

void BranchNode::flat_vals_back_activate(vector<int>& scope_context,
										 vector<int>& node_context,
										 int d_index,
										 int stride_size,
										 vector<double>& flat_vals,
										 BranchNodeHistory* history) {
	for (int h_index = 0; h_index < (int)this->test_hook_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != scope_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]
						|| this->test_hook_node_contexts[h_index][c_index] != node_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			flat_vals[d_index*stride_size + this->test_hook_indexes[h_index]] = history->obs_snapshot;
		}
	}
}

void BranchNode::rnn_vals_back_activate(vector<int>& scope_context,
										vector<int>& node_context,
										vector<int>& obs_indexes,
										vector<double>& obs_vals,
										BranchNodeHistory* history) {
	for (int h_index = 0; h_index < (int)this->test_hook_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != scope_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]
						|| this->test_hook_node_contexts[h_index][c_index] != node_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			obs_indexes.push_back(this->test_hook_indexes[h_index]);
			obs_vals.push_back(history->obs_snapshot);
		}
	}
}

void BranchNode::experiment_back_activate(vector<int>& scope_context,
										  vector<int>& node_context,
										  map<int, StateStatus>& experiment_state_vals,
										  BranchNodeHistory* history) {
	for (int n_index = 0; n_index < (int)this->experiment_hook_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_state_scope_contexts[n_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_state_scope_contexts[n_index][c_index] != scope_context[scope_context.size()-this->experiment_hook_state_scope_contexts[n_index].size()+c_index]
						|| this->experiment_hook_state_node_contexts[n_index][c_index] != node_context[scope_context.size()-this->experiment_hook_state_scope_contexts[n_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<int, StateStatus>::iterator it = experiment_state_vals.find(this->experiment_hook_state_indexes[n_index]);
			if (it == experiment_state_vals.end()) {
				it = experiment_state_vals.insert({this->experiment_hook_state_indexes[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_state_defs[n_index]->networks[this->experiment_hook_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}
}
