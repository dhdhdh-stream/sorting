#include "solution_helpers.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

ScopeNode* existing_new_start(Scope* parent_scope,
							  RunHelper& run_helper) {
	if (parent_scope->nodes.size() == 1) {
		/**
		 * - starting edge case
		 */
		return NULL;
	}

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context{parent_scope};
	vector<AbstractNode*> node_context{NULL};

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	int random_curr_depth = run_helper.curr_depth;
	int random_throw_id = -1;
	bool random_exceeded_limit = false;

	/**
	 * TODO: add custom, cleaner code for existing_new_start()
	 */
	parent_scope->random_activate(parent_scope->default_starting_node,
								  scope_context,
								  node_context,
								  exit_depth,
								  exit_node,
								  random_curr_depth,
								  random_throw_id,
								  random_exceeded_limit,
								  possible_scope_contexts,
								  possible_node_contexts);

	vector<AbstractNode*> possible_starting_nodes;
	/**
	 * - don't include last
	 */
	for (int p_index = 0; p_index < (int)possible_scope_contexts.size()-1; p_index++) {
		if (possible_scope_contexts[p_index].size() == 1) {
			possible_starting_nodes.push_back(possible_node_contexts[p_index][0]);
		}
	}

	uniform_int_distribution<int> distribution(0, possible_starting_nodes.size()-1);
	AbstractNode* new_starting_node = possible_starting_nodes[distribution(generator)];

	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->starting_node_id = new_starting_node->id;
	new_scope_node->starting_node = new_starting_node;
	new_scope_node->scope = parent_scope;

	return new_scope_node;
}
