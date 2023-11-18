#include "branch_node.h"

#include "globals.h"
#include "scope.h"

using namespace std;

void BranchNode::random_activate(bool& is_branch,
								 vector<Scope*>& scope_context,
								 vector<AbstractNode*>& node_context,
								 vector<AbstractNode*>& possible_nodes,
								 vector<vector<Scope*>>& possible_scope_contexts,
								 vector<vector<AbstractNode*>>& possible_node_contexts) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != scope_context[scope_context.size()-this->branch_scope_context.size()+c_index]->id
					|| this->branch_node_context[c_index] != node_context[scope_context.size()-this->branch_scope_context.size()+c_index]->id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;

			// don't include
		} else {
			node_context.back() = this;

			possible_nodes.push_back(this);
			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);

			node_context.back() = NULL;

			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
	} else {
		is_branch = false;

		// don't include
	}
}
