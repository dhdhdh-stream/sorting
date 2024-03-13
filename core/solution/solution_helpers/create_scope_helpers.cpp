/**
 * - simply exclude branches
 */

#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void create_scope_helper(vector<Scope*>& scope_context,
						 vector<AbstractNode*>& node_context,
						 vector<vector<Scope*>>& possible_scope_contexts,
						 vector<vector<AbstractNode*>>& possible_node_contexts,
						 ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		switch (node_history->node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (action_node->action.move != ACTION_NOOP) {
					node_context.back() = action_node;

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = NULL;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				create_scope_helper(scope_context,
									node_context,
									possible_scope_contexts,
									possible_node_contexts,
									scope_node_history->scope_history);

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			}
			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

ScopeNode* create_scope(Scope* parent_scope,
						RunHelper& run_helper) {
	if (parent_scope->sample_run == NULL) {
		return NULL;
	}

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_scope_helper(scope_context,
						node_context,
						possible_scope_contexts,
						possible_node_contexts,
						parent_scope->sample_run);

	int num_actions = 0;
	for (int n_index = 0; n_index < (int)possible_scope_contexts.size(); n_index++) {
		if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
			num_actions++;
			if (num_actions >= 2) {
				break;
			}
		}
	}
	if (num_actions < 2) {
		return NULL;
	}

	int start_index;
	int end_index;
	uniform_int_distribution<int> distribution(0, possible_scope_contexts.size()-1);
	while (true) {
		start_index = distribution(generator);
		end_index = distribution(generator);
		if (start_index <= end_index) {
			int num_actions = 0;
			for (int n_index = start_index; n_index < end_index+1; n_index++) {
				if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
					num_actions++;
					if (num_actions >= 2) {
						break;
					}
				}
			}
			if (num_actions >= 2) {
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
			if (n_index != start_index
					&& possible_scope_contexts[n_index].back() == possible_scope_contexts[start_index][possible_scope_contexts[n_index].size()-1]) {
				/**
				 * - remove duplicate scope nodes
				 */
				on_path = false;
			} else {
				on_path = true;
			}
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

				ActionNode* new_action_node = new ActionNode();

				new_action_node->action = original_action_node->action;

				new_nodes.push_back(new_action_node);
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
