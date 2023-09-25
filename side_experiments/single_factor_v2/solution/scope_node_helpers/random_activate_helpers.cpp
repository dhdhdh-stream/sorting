#include "scope_node.h"

using namespace std;

void ScopeNode::random_activate(vector<int>& scope_context,
								vector<int>& node_context,
								int& inner_exit_depth,
								int& inner_exit_node_id,
								int& num_nodes,
								vector<AbstractNodeHistory*>& node_histories) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	node_histories.push_back(history);

	num_nodes++;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;

	uniform_int_distribution<int> distribution(0, 1);
	if (!inner_scope->is_loop
			&& distribution(generator) == 0) {
		node_context.back() = this->id;

		scope_context.push_back(this->inner_scope_id);
		node_context.push_back(-1);

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
	/**
	 * - don't worry about missing early exits
	 *   - can change distribution, but won't lead to missing nodes
	 *     - as early exits will always be on path
	 */
}

void ScopeNode::halfway_random_activate(vector<int>& starting_node_ids,
										vector<int>& scope_context,
										vector<int>& node_context,
										int& inner_exit_depth,
										int& inner_exit_node_id,
										int& num_nodes,
										vector<AbstractNodeHistory*>& node_histories) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	node_histories.push_back(history);

	history->is_halfway = true;

	// don't increment num_nodes

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;

	// inner_scope->is_loop == false
	uniform_int_distribution<int> distribution(0, 1);
	if (distribution(generator) == 0) {
		node_context.back() = this->id;

		scope_context.push_back(this->inner_scope_id);
		node_context.push_back(-1);

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
}
