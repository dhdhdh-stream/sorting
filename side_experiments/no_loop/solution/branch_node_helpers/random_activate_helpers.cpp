#include "branch_node.h"

using namespace std;

void BranchNode::random_activate(vector<int>& scope_context,
								 vector<int>& node_context,
								 bool& is_branch,
								 int& num_nodes,
								 vector<AbstractNodeHistory*>& node_histories) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != scope_context[scope_context.size()-this->branch_scope_context.size()+c_index]
					|| this->branch_node_context[c_index] != node_context[scope_context.size()-this->branch_scope_context.size()+c_index]) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;

			// don't increment num_nodes;
		} else {
			BranchNodeHistory* history = new BranchNodeHistory(this);
			node_histories.push_back(history);

			num_nodes++;

			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
	} else {
		is_branch = false;

		// don't increment num_nodes;
	}
}
