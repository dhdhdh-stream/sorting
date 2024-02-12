#include "branch_node.h"

#include "globals.h"

using namespace std;

void BranchNode::random_activate(bool& is_branch,
								 vector<Scope*>& scope_context,
								 vector<AbstractNode*>& node_context,
								 vector<vector<Scope*>>& possible_scope_contexts,
								 vector<vector<AbstractNode*>>& possible_node_contexts) {
	bool matches_context = true;
	if (this->scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			if (this->scope_context[c_index] != scope_context[scope_context.size()-this->scope_context.size()+c_index]
					|| this->node_context[c_index] != node_context[scope_context.size()-this->scope_context.size()+c_index]) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->is_pass_through) {
			is_branch = true;

			// don't include
		} else {
			node_context.back() = this;

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

void BranchNode::random_exit_activate(bool& is_branch,
									  vector<Scope*>& scope_context,
									  vector<AbstractNode*>& node_context,
									  int curr_depth,
									  vector<pair<int,AbstractNode*>>& possible_exits) {
	bool matches_context = true;
	if ((int)this->scope_context.size() > curr_depth+1) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			if (this->scope_context[c_index] != scope_context[curr_depth+1-this->scope_context.size()+c_index]
					|| this->node_context[c_index] != node_context[curr_depth+1-this->scope_context.size()+c_index]) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->is_pass_through) {
			is_branch = true;

			// don't include
		} else {
			possible_exits.push_back({curr_depth, this});

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
