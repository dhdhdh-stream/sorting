#include "branch_node.h"

#include "globals.h"

using namespace std;

void BranchNode::random_activate(AbstractNode*& curr_node,
								 vector<Scope*>& scope_context,
								 vector<AbstractNode*>& node_context) {
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
			curr_node = this->branch_next_node;

			// don't include
		} else {
			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				curr_node = this->branch_next_node;
			} else {
				curr_node = this->original_next_node;
			}
		}
	} else {
		curr_node = this->original_next_node;

		// don't include
	}
}

void BranchNode::random_exit_activate(AbstractNode*& curr_node,
									  vector<Scope*>& scope_context,
									  vector<AbstractNode*>& node_context,
									  int curr_depth,
									  vector<pair<int,AbstractNode*>>& possible_exits) {
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
			curr_node = this->branch_next_node;

			// don't include
		} else {
			possible_exits.push_back({curr_depth, this});

			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				curr_node = this->branch_next_node;
			} else {
				curr_node = this->original_next_node;
			}
		}
	} else {
		curr_node = this->original_next_node;

		// don't include
	}
}

void BranchNode::inner_random_exit_activate(AbstractNode*& curr_node,
											vector<Scope*>& scope_context,
											vector<AbstractNode*>& node_context) {
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
			curr_node = this->branch_next_node;
		} else {
			uniform_int_distribution<int> distribution(0, 1);
			if (distribution(generator) == 0) {
				curr_node = this->branch_next_node;
			} else {
				curr_node = this->original_next_node;
			}
		}
	} else {
		curr_node = this->original_next_node;
	}
}
