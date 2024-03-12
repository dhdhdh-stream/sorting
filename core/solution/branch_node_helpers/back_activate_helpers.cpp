#include "branch_node.h"

using namespace std;

void BranchNode::back_activate(vector<Scope*>& scope_context,
							   vector<AbstractNode*>& node_context,
							   int path_depth,
							   vector<int>& context_match_indexes,
							   vector<double>& input_vals,
							   BranchNodeHistory* history) {
	for (int h_index = 0; h_index < (int)this->hook_indexes.size(); h_index++) {
		bool matches_context = false;
		if (this->hook_is_fuzzy_match[h_index]) {
			int c_index = (int)this->hook_scope_contexts[h_index].size()-2;
			int l_index = (int)scope_context.size()-2;
			while (true) {
				if (c_index < 0) {
					matches_context = true;
					break;
				}

				if (l_index < 0) {
					break;
				}

				if (this->hook_scope_contexts[h_index][c_index] == scope_context[l_index]
						&& this->hook_node_contexts[h_index][c_index] == node_context[l_index]) {
					c_index--;
				}
				l_index--;
			}
		} else {
			int start_index = context_match_indexes[this->hook_strict_root_indexes[h_index]]
				- context_match_indexes[0];
			if (path_depth >= start_index) {
				if (scope_context.size() - start_index == this->hook_scope_contexts[h_index].size()) {
					matches_context = true;
					for (int c_index = 0; c_index < (int)this->hook_scope_contexts[h_index].size()-1; c_index++) {
						if (this->hook_scope_contexts[h_index][c_index] != scope_context[start_index + c_index]
								|| this->hook_node_contexts[h_index][c_index] != node_context[start_index + c_index]) {
							matches_context = false;
							break;
						}
					}
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
