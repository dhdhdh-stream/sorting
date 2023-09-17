#include "scope_node.h"

using namespace std;

void ScopeNode::random_activate(vector<int>& scope_context,
								vector<int>& node_context,
								int& inner_exit_depth,
								int& inner_exit_node_id,
								int& num_nodes,
								ScopeNodeHistory* history) {
	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	if (inner_scope->is_loop) {
		return;
	}

	node_context.back() = this->id;

	scope_context.push_back(this->inner_scope_id);
	node_context.push_back(-1);

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	inner_scope->random_activate(starting_node_ids_copy,
								 scope_context,
								 node_context,
								 inner_exit_depth,
								 inner_exit_node_id,
								 num_nodes,
								 inner_scope_history);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = -1;
}

void ScopeNode::halfway_random_activate(vector<int>& starting_node_ids,
										vector<int>& scope_context,
										vector<int>& node_context,
										int& inner_exit_depth,
										int& inner_exit_node_id,
										int& num_nodes,
										ScopeNodeHistory* history) {
	history->is_halfway = true;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	// inner_scope->is_loop == false

	node_context.back() = this->id;

	scope_context.push_back(this->inner_scope_id);
	node_context.push_back(-1);

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;

	inner_scope->random_activate(starting_node_ids,
								 scope_context,
								 node_context,
								 inner_exit_depth,
								 inner_exit_node_id,
								 num_nodes,
								 inner_scope_history);

	scope_context.pop_back();
	node_context.pop_back();

	node_context.back() = -1;
}
