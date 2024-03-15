#include "branch_node.h"

using namespace std;

void BranchNode::back_activate(vector<Scope*>& scope_context,
							   vector<AbstractNode*>& node_context,
							   vector<double>& input_vals,
							   BranchNodeHistory* history) {
	for (int h_index = 0; h_index < (int)this->hook_indexes.size(); h_index++) {
		bool matches_context = true;
		/**
		 * - exact match to not match recursive
		 */
		if (this->hook_scope_contexts[h_index].size() != scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->hook_scope_contexts[h_index][c_index] != scope_context[c_index]
						|| this->hook_node_contexts[h_index][c_index] != node_context[c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			if (history->is_branch) {
				input_vals[this->hook_indexes[h_index]] = 1.0;
			} else {
				input_vals[this->hook_indexes[h_index]] = -1.0;
			}
		}
	}
}
