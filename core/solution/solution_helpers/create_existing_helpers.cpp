#include "solution_helpers.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

ScopeNode* create_existing(Scope* parent_scope,
						   RunHelper& run_helper) {
	if (parent_scope->nodes.size() == 1) {
		/**
		 * - starting edge case
		 */
		return NULL;
	}

	uniform_int_distribution<int> new_distribution(0, 3);
	if (parent_scope->subscopes.size() > 0
			&& !new_distribution(generator) == 0) {
		uniform_int_distribution<int> subscope_distribution(0, parent_scope->subscopes.size()-1);
		pair<int, set<int>> subscope = *next(parent_scope->subscopes.begin(), subscope_distribution(generator));

		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = parent_scope;

		new_scope_node->starting_node_id = subscope.first;
		new_scope_node->starting_node = parent_scope->nodes[subscope.first];

		new_scope_node->exit_node_ids = subscope.second;
		for (set<int>::iterator it = subscope.second.begin();
				it != subscope.second.end(); it++) {
			new_scope_node->exit_nodes.insert(parent_scope->nodes[*it]);
		}

		return new_scope_node;
	} else {
		vector<AbstractNode*> possible_nodes;

		vector<Scope*> scope_context{parent_scope};
		vector<AbstractNode*> node_context{NULL};

		// unused
		int exit_depth = -1;
		AbstractNode* exit_node = NULL;

		int random_curr_depth = run_helper.curr_depth;
		int random_throw_id = -1;
		bool random_exceeded_limit = false;

		parent_scope->random_existing_activate(parent_scope->default_starting_node,
											   scope_context,
											   node_context,
											   exit_depth,
											   exit_node,
											   random_curr_depth,
											   random_throw_id,
											   random_exceeded_limit,
											   possible_nodes);

		if (random_exceeded_limit) {
			return NULL;
		}

		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->scope = parent_scope;

		uniform_int_distribution<int> starting_distribution(0, possible_nodes.size()-1);
		int starting_index = starting_distribution(generator);
		AbstractNode* new_starting_node = possible_nodes[starting_index];

		new_scope_node->starting_node_id = new_starting_node->id;
		new_scope_node->starting_node = new_starting_node;

		geometric_distribution<int> num_exits_distribution(0.33);
		int num_exits = num_exits_distribution(generator);
		if (num_exits > 0) {
			if (starting_index != (int)possible_nodes.size()-1) {
				uniform_int_distribution<int> exit_distribution(starting_index+1, possible_nodes.size()-1);
				int exit_index = exit_distribution(generator);
				new_scope_node->exit_node_ids.insert(possible_nodes[exit_index]->id);
				new_scope_node->exit_nodes.insert(possible_nodes[exit_index]);
			}
		}

		for (int e_index = 1; e_index < num_exits; e_index++) {
			vector<AbstractNode*> possible_nodes;

			vector<Scope*> scope_context{parent_scope};
			vector<AbstractNode*> node_context{NULL};

			// unused
			int exit_depth = -1;
			AbstractNode* exit_node = NULL;

			int random_curr_depth = run_helper.curr_depth;
			int random_throw_id = -1;
			bool random_exceeded_limit = false;

			parent_scope->random_existing_activate(new_starting_node,
												   scope_context,
												   node_context,
												   exit_depth,
												   exit_node,
												   random_curr_depth,
												   random_throw_id,
												   random_exceeded_limit,
												   possible_nodes);

			if (!random_exceeded_limit
					&& possible_nodes.size() > 1) {
				uniform_int_distribution<int> exit_distribution(1, possible_nodes.size()-1);
				int exit_index = exit_distribution(generator);
				new_scope_node->exit_node_ids.insert(possible_nodes[exit_index]->id);
				new_scope_node->exit_nodes.insert(possible_nodes[exit_index]);
			}
		}

		return new_scope_node;
	}
}
