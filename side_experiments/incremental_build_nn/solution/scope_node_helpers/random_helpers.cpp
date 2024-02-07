#include "scope_node.h"

using namespace std;

void ScopeNode::random_activate(vector<Scope*>& scope_context,
								vector<AbstractNode*>& node_context,
								int& inner_exit_depth,
								AbstractNode*& inner_exit_node,
								vector<vector<Scope*>>& possible_scope_contexts,
								vector<vector<AbstractNode*>>& possible_node_contexts) {
	node_context.back() = this;

	possible_scope_contexts.push_back(scope_context);
	possible_node_contexts.push_back(node_context);

	uniform_int_distribution<int> distribution(0, 2);
	if (distribution(generator) != 0) {
		scope_context.push_back(this->scope);
		node_context.push_back(NULL);

		this->scope->random_activate(scope_context,
									 node_context,
									 inner_exit_depth,
									 inner_exit_node,
									 possible_scope_contexts,
									 possible_node_contexts);

		scope_context.pop_back();
		node_context.pop_back();
	}

	node_context.back() = NULL;
}
