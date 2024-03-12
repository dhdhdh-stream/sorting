/**
 * - simply exclude branches
 */

#include "solution_helpers.h"

#include "action_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

ScopeNode* create_scope(Scope* parent_scope,
						RunHelper& run_helper) {
	// determine start and end
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

	parent_scope->random_activate(scope_context,
								  node_context,
								  exit_depth,
								  exit_node,
								  random_curr_depth,
								  random_throw_id,
								  random_exceeded_limit,
								  possible_scope_contexts,
								  possible_node_contexts);

	if (random_exceeded_limit) {
		return NULL;
	}

	bool has_meaningful_actions = false;
	for (int n_index = 0; n_index < (int)possible_scope_contexts.size()-1; n_index++) {
		if (possible_node_contexts[n_index].back() != NULL) {
			if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)possible_node_contexts[n_index].back();
				if (action_node->action.move != ACTION_NOOP) {
					has_meaningful_actions = true;
					break;
				}
			} else if (possible_node_contexts[n_index].back()->type == NODE_TYPE_SCOPE) {
				if (possible_scope_contexts[n_index].size() == possible_scope_contexts[n_index+1].size()) {
					has_meaningful_actions = true;
					break;
				}
			}
		}
	}
	if (!has_meaningful_actions) {
		return NULL;
	}

	int start_index;
	int end_index;
	uniform_int_distribution<int> distribution(0, possible_scope_contexts.size()-1);
	while (true) {
		start_index = distribution(generator);
		end_index = distribution(generator);
		if (start_index <= end_index
				&& possible_node_contexts[start_index].back() != NULL
				&& possible_node_contexts[start_index].back()->type != NODE_TYPE_EXIT) {
			bool empty_path = true;
			for (int n_index = start_index; n_index < end_index+1; n_index++) {
				if (possible_node_contexts[n_index].back() != NULL) {
					if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)possible_node_contexts[n_index].back();
						if (action_node->action.move != ACTION_NOOP) {
							empty_path = false;
							break;
						}
					} else if (possible_node_contexts[n_index].back()->type == NODE_TYPE_SCOPE) {
						if (possible_scope_contexts[n_index].size() == possible_scope_contexts[n_index+1].size()) {
							// same depth means that entire scope node included
							empty_path = false;
							break;
						}
					}
				}
			}
			if (!empty_path) {
				break;
			}
		}
	}

	Scope* new_scope = new Scope();
	// don't set id/increment scope_counter until train
	new_scope->node_counter = 0;
	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->scope = new_scope;

	vector<AbstractNode*> new_nodes;

	ActionNode* new_noop_action_node = new ActionNode();
	new_noop_action_node->action = Action(ACTION_NOOP);
	new_nodes.push_back(new_noop_action_node);

	for (int n_index = start_index; n_index < end_index+1; n_index++) {
		bool on_path = false;

		bool match_start = true;
		if (possible_scope_contexts[n_index].size() > possible_scope_contexts[start_index].size()) {
			match_start = false;
		} else {
			for (int c_index = 0; c_index < (int)possible_scope_contexts[n_index].size()-1; c_index++) {
				if (possible_scope_contexts[start_index][c_index] != possible_scope_contexts[n_index][c_index]
						|| possible_node_contexts[start_index][c_index] != possible_node_contexts[n_index][c_index]) {
					match_start = false;
					break;
				}
			}
		}
		if (match_start) {
			on_path = true;
		} else {
			bool match_end = true;
			if (possible_scope_contexts[n_index].size() > possible_scope_contexts[end_index].size()) {
				match_end = false;
			} else {
				for (int c_index = 0; c_index < (int)possible_scope_contexts[n_index].size()-1; c_index++) {
					if (possible_scope_contexts[end_index][c_index] != possible_scope_contexts[n_index][c_index]
							|| possible_node_contexts[end_index][c_index] != possible_node_contexts[n_index][c_index]) {
						match_end = false;
						break;
					}
				}
			}
			if (match_end) {
				on_path = true;
			}
		}

		if (on_path) {
			if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* original_action_node = (ActionNode*)possible_node_contexts[n_index].back();
				if (original_action_node->action.move != ACTION_NOOP) {
					ActionNode* new_action_node = new ActionNode();

					new_action_node->action = original_action_node->action;

					new_nodes.push_back(new_action_node);
				}
			} else if (possible_node_contexts[n_index].back()->type == NODE_TYPE_SCOPE) {
				ScopeNode* original_scope_node = (ScopeNode*)possible_node_contexts[n_index].back();

				ScopeNode* new_scope_node = new ScopeNode();

				new_scope_node->scope = original_scope_node->scope;

				new_nodes.push_back(new_scope_node);
			}
		}
	}

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		new_nodes[n_index]->parent = new_scope;
		new_nodes[n_index]->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_nodes[n_index]->id] = new_nodes[n_index];

		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			next_node_id = n_index+1;
			next_node = new_nodes[n_index+1];
		}

		if (new_nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)new_nodes[n_index];
			action_node->next_node_id = next_node_id;
			action_node->next_node = next_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)new_nodes[n_index];
			scope_node->next_node_id = next_node_id;
			scope_node->next_node = next_node;
		}
	}

	new_scope->starting_node_id = 0;
	new_scope->starting_node = new_nodes[0];

	return new_scope_node;
}
