#include "solution_helpers.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

pair<bool,AbstractNode*> end_node_helper(vector<Scope*>& scope_context,
										 vector<AbstractNode*>& node_context,
										 int curr_depth,
										 AbstractNode* curr_node,
										 map<AbstractNode*, pair<bool,AbstractNode*>>& node_mappings,
										 map<AbstractNode*, AbstractNode*>& new_node_reverse_mappings,
										 Scope* new_scope) {
	map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it = node_mappings.find(curr_node);
	if (it != node_mappings.end()) {
		return it->second;
	} else {
		pair<bool,AbstractNode*> mapping;
		if (curr_node == NULL) {
			/**
			 * - if curr_node is NULL and not already added, then it's off path
			 */
			mapping = {false, NULL};
		} else {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = end_node_helper(
						scope_context,
						node_context,
						curr_depth,
						action_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ActionNode* new_action_node = new ActionNode();

						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_node_reverse_mappings[new_action_node] = action_node;

						new_action_node->action = action_node->action;

						if (next_mapping.second == NULL) {
							new_action_node->next_node_id = -1;
						} else {
							new_action_node->next_node_id = next_mapping.second->id;
						}
						new_action_node->next_node = next_mapping.second;

						mapping = {true, new_action_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = end_node_helper(
						scope_context,
						node_context,
						curr_depth,
						scope_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ScopeNode* new_scope_node = new ScopeNode();

						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_node_reverse_mappings[new_scope_node] = scope_node;

						new_scope_node->scope = scope_node->scope;

						if (next_mapping.second == NULL) {
							new_scope_node->next_node_id = -1;
						} else {
							new_scope_node->next_node_id = next_mapping.second->id;
						}
						new_scope_node->next_node = next_mapping.second;

						mapping = {true, new_scope_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					bool matches_context = true;
					if ((int)branch_node->scope_context.size() > 1+curr_depth) {
						matches_context = false;
					} else {
						for (int c_index = 0; c_index < (int)branch_node->scope_context.size()-1; c_index++) {
							if (branch_node->scope_context[c_index] != scope_context[1+curr_depth - branch_node->scope_context.size() + c_index]
									|| branch_node->node_context[c_index] != node_context[1+curr_depth - branch_node->scope_context.size() + c_index]) {
								matches_context = false;
								break;
							}
						}
					}

					if (matches_context) {
						if (branch_node->is_pass_through) {
							pair<bool,AbstractNode*> branch_mapping = end_node_helper(
								scope_context,
								node_context,
								curr_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							mapping = branch_mapping;
						} else {
							pair<bool,AbstractNode*> original_mapping = end_node_helper(
								scope_context,
								node_context,
								curr_depth,
								branch_node->original_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							pair<bool,AbstractNode*> branch_mapping = end_node_helper(
								scope_context,
								node_context,
								curr_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							if (original_mapping.first && branch_mapping.first) {
								BranchNode* new_branch_node = new BranchNode();

								new_branch_node->parent = new_scope;
								new_branch_node->id = new_scope->node_counter;
								new_scope->node_counter++;
								new_scope->nodes[new_branch_node->id] = new_branch_node;

								new_node_reverse_mappings[new_branch_node] = branch_node;

								new_branch_node->scope_context_ids = vector<int>{-1};
								new_branch_node->scope_context = vector<Scope*>{new_scope};
								new_branch_node->node_context_ids = vector<int>{new_branch_node->id};
								new_branch_node->node_context = vector<AbstractNode*>{new_branch_node};

								new_branch_node->is_pass_through = false;

								if (original_mapping.second == NULL) {
									new_branch_node->original_next_node_id = -1;
								} else {
									new_branch_node->original_next_node_id = original_mapping.second->id;
								}
								new_branch_node->original_next_node = original_mapping.second;
								if (branch_mapping.second == NULL) {
									new_branch_node->branch_next_node_id = -1;
								} else {
									new_branch_node->branch_next_node_id = branch_mapping.second->id;
								}
								new_branch_node->branch_next_node = branch_mapping.second;

								mapping = {true, new_branch_node};
							} else if (original_mapping.first) {
								mapping = original_mapping;
							} else if (branch_mapping.first) {
								mapping = branch_mapping;
							} else {
								mapping = {false, NULL};
							}
						}
					} else {
						pair<bool,AbstractNode*> original_mapping = end_node_helper(
							scope_context,
							node_context,
							curr_depth,
							branch_node->original_next_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);

						mapping = original_mapping;
					}
				}

				break;
			case NODE_TYPE_EXIT:
				/**
				 * - if not already added, then off path
				 */
				mapping = {false, NULL};

				break;
			}
		}

		node_mappings[curr_node] = mapping;
		return mapping;
	}
}

pair<bool,AbstractNode*> start_node_helper(vector<Scope*>& scope_context,
										   vector<AbstractNode*>& node_context,
										   int curr_depth,
										   int starting_depth,
										   AbstractNode* curr_node,
										   vector<map<AbstractNode*, pair<bool,AbstractNode*>>>& node_mappings,
										   vector<map<AbstractNode*, AbstractNode*>>& new_node_reverse_mappings,
										   Scope* new_scope) {
	map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it = node_mappings[curr_depth].find(curr_node);
	if (it != node_mappings[curr_depth].end()) {
		return it->second;
	} else {
		pair<bool,AbstractNode*> mapping;
		if (curr_node == NULL) {
			if (curr_depth == starting_depth) {
				mapping = {false, NULL};
			} else {
				ScopeNode* outer_scope_node = (ScopeNode*)node_context[curr_depth-1];
				pair<bool,AbstractNode*> next_mapping = start_node_helper(
					scope_context,
					node_context,
					curr_depth-1,
					starting_depth,
					outer_scope_node->next_node,
					node_mappings,
					new_node_reverse_mappings,
					new_scope);
				mapping = next_mapping;
			}
		} else {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = start_node_helper(
						scope_context,
						node_context,
						curr_depth,
						starting_depth,
						action_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ActionNode* new_action_node = new ActionNode();

						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_node_reverse_mappings[curr_depth][new_action_node] = action_node;

						new_action_node->action = action_node->action;

						if (next_mapping.second == NULL) {
							new_action_node->next_node_id = -1;
						} else {
							new_action_node->next_node_id = next_mapping.second->id;
						}
						new_action_node->next_node = next_mapping.second;

						mapping = {true, new_action_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = start_node_helper(
						scope_context,
						node_context,
						curr_depth,
						starting_depth,
						scope_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ScopeNode* new_scope_node = new ScopeNode();

						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_node_reverse_mappings[curr_depth][new_scope_node] = scope_node;

						new_scope_node->scope = scope_node->scope;

						if (next_mapping.second == NULL) {
							new_scope_node->next_node_id = -1;
						} else {
							new_scope_node->next_node_id = next_mapping.second->id;
						}
						new_scope_node->next_node = next_mapping.second;

						mapping = {true, new_scope_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					bool matches_context = true;
					if ((int)branch_node->scope_context.size() > 1+curr_depth) {
						matches_context = false;
					} else {
						for (int c_index = 0; c_index < (int)branch_node->scope_context.size()-1; c_index++) {
							if (branch_node->scope_context[c_index] != scope_context[1+curr_depth - branch_node->scope_context.size() + c_index]
									|| branch_node->node_context[c_index] != node_context[1+curr_depth - branch_node->scope_context.size() + c_index]) {
								matches_context = false;
								break;
							}
						}
					}

					if (matches_context) {
						if (branch_node->is_pass_through) {
							pair<bool,AbstractNode*> branch_mapping = start_node_helper(
								scope_context,
								node_context,
								curr_depth,
								starting_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							mapping = branch_mapping;
						} else {
							pair<bool,AbstractNode*> original_mapping = start_node_helper(
								scope_context,
								node_context,
								curr_depth,
								starting_depth,
								branch_node->original_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							pair<bool,AbstractNode*> branch_mapping = start_node_helper(
								scope_context,
								node_context,
								curr_depth,
								starting_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							if (original_mapping.first && branch_mapping.first) {
								BranchNode* new_branch_node = new BranchNode();

								new_branch_node->parent = new_scope;
								new_branch_node->id = new_scope->node_counter;
								new_scope->node_counter++;
								new_scope->nodes[new_branch_node->id] = new_branch_node;

								new_node_reverse_mappings[curr_depth][new_branch_node] = branch_node;

								new_branch_node->scope_context_ids = vector<int>{-1};
								new_branch_node->scope_context = vector<Scope*>{new_scope};
								new_branch_node->node_context_ids = vector<int>{new_branch_node->id};
								new_branch_node->node_context = vector<AbstractNode*>{new_branch_node};

								new_branch_node->is_pass_through = false;

								if (original_mapping.second == NULL) {
									new_branch_node->original_next_node_id = -1;
								} else {
									new_branch_node->original_next_node_id = original_mapping.second->id;
								}
								new_branch_node->original_next_node = original_mapping.second;
								if (branch_mapping.second == NULL) {
									new_branch_node->branch_next_node_id = -1;
								} else {
									new_branch_node->branch_next_node_id = branch_mapping.second->id;
								}
								new_branch_node->branch_next_node = branch_mapping.second;

								mapping = {true, new_branch_node};
							} else if (original_mapping.first) {
								mapping = original_mapping;
							} else if (branch_mapping.first) {
								mapping = branch_mapping;
							} else {
								mapping = {false, NULL};
							}
						}
					} else {
						pair<bool,AbstractNode*> original_mapping = start_node_helper(
							scope_context,
							node_context,
							curr_depth,
							starting_depth,
							branch_node->original_next_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);

						mapping = original_mapping;
					}
				}

				break;
			case NODE_TYPE_EXIT:
				{
					ExitNode* exit_node = (ExitNode*)curr_node;

					if (exit_node->exit_depth > curr_depth - starting_depth) {
						mapping = {false, NULL};
					} else {
						/**
						 * - if reachable, then context must match, and exit must be valid
						 */
						pair<bool,AbstractNode*> exit_mapping = start_node_helper(
							scope_context,
							node_context,
							curr_depth - exit_node->exit_depth,
							starting_depth,
							exit_node->next_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);
						mapping = exit_mapping;
					}
				}

				break;
			}
		}

		node_mappings[curr_depth][curr_node] = mapping;
		return mapping;
	}
}

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
	bool random_exceeded_limit = false;

	parent_scope->random_activate(scope_context,
								  node_context,
								  exit_depth,
								  exit_node,
								  random_curr_depth,
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

	vector<Scope*> start_scope_context;
	vector<AbstractNode*> start_node_context;
	vector<Scope*> end_scope_context;
	vector<AbstractNode*> end_node_context;
	uniform_int_distribution<int> distribution(0, possible_scope_contexts.size()-1);
	while (true) {
		int start_index = distribution(generator);
		int end_index = distribution(generator);
		if (start_index < end_index
				&& possible_node_contexts[start_index].back() != NULL
				&& possible_node_contexts[start_index].back()->type != NODE_TYPE_EXIT) {
			bool empty_path = true;
			for (int n_index = start_index; n_index < end_index; n_index++) {
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
				start_scope_context = possible_scope_contexts[start_index];
				start_node_context = possible_node_contexts[start_index];
				end_scope_context = possible_scope_contexts[end_index];
				end_node_context = possible_node_contexts[end_index];

				break;
			}
		}
	}

	Scope* new_scope = new Scope();
	// don't set id/increment scope_counter until train
	new_scope->node_counter = 0;
	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->scope = new_scope;

	int starting_depth = 0;
	while (true) {
		if (starting_depth >= (int)start_scope_context.size()	// start can be empty
				|| starting_depth + 1 >= (int)end_scope_context.size()) {
			break;
		}

		if (start_scope_context[starting_depth] == end_scope_context[starting_depth]
				&& start_node_context[starting_depth] == end_node_context[starting_depth]) {
			starting_depth++;
		} else {
			break;
		}
	}

	AbstractNode* new_starting_node;
	vector<map<AbstractNode*, pair<bool,AbstractNode*>>> end_node_mappings(end_scope_context.size());
	vector<map<AbstractNode*, AbstractNode*>> end_new_node_reverse_mappings(end_scope_context.size());
	if ((int)start_scope_context.size() == starting_depth) {
		// start empty edge case
		{
			end_node_mappings.back()[end_node_context.back()] = {true, NULL};
			if (end_scope_context.back()->starting_node != end_node_context.back()) {
				end_node_helper(end_scope_context,
								end_node_context,
								(int)end_scope_context.size()-1,
								end_scope_context.back()->starting_node,
								end_node_mappings.back(),
								end_new_node_reverse_mappings.back(),
								new_scope);

				new_starting_node = end_node_mappings.back()[end_scope_context.back()->starting_node].second;
			}
		}
		for (int l_index = (int)end_scope_context.size()-2; l_index >= starting_depth; l_index--) {
			AbstractNode* next_layer_start = end_node_mappings[l_index + 1]
				[end_scope_context[l_index + 1]->starting_node].second;
			end_node_mappings[l_index][end_node_context[l_index]] = {true, next_layer_start};
			if (end_scope_context[l_index]->starting_node != end_node_context[l_index]) {
				end_node_helper(end_scope_context,
								end_node_context,
								l_index,
								end_scope_context[l_index]->starting_node,
								end_node_mappings[l_index],
								end_new_node_reverse_mappings[l_index],
								new_scope);

				new_starting_node = end_node_mappings[l_index][end_scope_context[l_index]->starting_node].second;
			}
		}
	} else if ((int)end_scope_context.size() > starting_depth + 1) {
		{
			end_node_mappings.back()[end_node_context.back()] = {true, NULL};
			if (end_scope_context.back()->starting_node != end_node_context.back()) {
				end_node_helper(end_scope_context,
								end_node_context,
								(int)end_scope_context.size()-1,
								end_scope_context.back()->starting_node,
								end_node_mappings.back(),
								end_new_node_reverse_mappings.back(),
								new_scope);

				new_starting_node = end_node_mappings.back()[end_scope_context.back()->starting_node].second;
			}
		}
		for (int l_index = (int)end_scope_context.size()-2; l_index > starting_depth; l_index--) {
			AbstractNode* next_layer_start = end_node_mappings[l_index + 1]
				[end_scope_context[l_index + 1]->starting_node].second;
			end_node_mappings[l_index][end_node_context[l_index]] = {true, next_layer_start};
			if (end_scope_context[l_index]->starting_node != end_node_context[l_index]) {
				end_node_helper(end_scope_context,
								end_node_context,
								l_index,
								end_scope_context[l_index]->starting_node,
								end_node_mappings[l_index],
								end_new_node_reverse_mappings[l_index],
								new_scope);

				new_starting_node = end_node_mappings[l_index][end_scope_context[l_index]->starting_node].second;
			}
		}
	}

	vector<map<AbstractNode*, pair<bool,AbstractNode*>>> start_node_mappings(start_scope_context.size());
	vector<map<AbstractNode*, AbstractNode*>> start_new_node_reverse_mappings(start_scope_context.size());
	if ((int)start_scope_context.size() > starting_depth) {
		AbstractNode* end_node;
		if ((int)end_node_mappings.size() > starting_depth + 1) {
			end_node = end_node_mappings[starting_depth + 1]
				[end_scope_context[starting_depth + 1]->starting_node].second;
		} else {
			end_node = NULL;
		}
		start_node_mappings[starting_depth][end_node_context[starting_depth]] = {true, end_node};
		start_node_helper(start_scope_context,
						  start_node_context,
						  (int)start_scope_context.size()-1,
						  starting_depth,
						  start_node_context.back(),
						  start_node_mappings,
						  start_new_node_reverse_mappings,
						  new_scope);

		new_starting_node = start_node_mappings.back()[start_node_context.back()].second;
	}

	/**
	 * - edge case where inner scope exits to a node that's no longer reachable outside
	 */
	if (new_starting_node == NULL) {
		/**
		 * - may have added nodes that need to be deleted
		 */
		delete new_scope;
		delete new_scope_node;

		return NULL;
	}

	if (new_starting_node->type == NODE_TYPE_ACTION
			&& ((ActionNode*)new_starting_node)->action.move == ACTION_NOOP) {
		new_scope->starting_node_id = new_starting_node->id;
		new_scope->starting_node = new_starting_node;
	} else {
		ActionNode* new_noop_action_node = new ActionNode();
		new_noop_action_node->parent = new_scope;
		new_noop_action_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_noop_action_node->action = Action(ACTION_NOOP);
		new_noop_action_node->next_node_id = new_starting_node->id;
		new_noop_action_node->next_node = new_starting_node;
		new_scope->nodes[new_noop_action_node->id] = new_noop_action_node;

		new_scope->starting_node_id = new_noop_action_node->id;
		new_scope->starting_node = new_noop_action_node;
	}

	for (int l_index = (int)start_new_node_reverse_mappings.size()-1; l_index >= starting_depth; l_index--) {
		for (map<AbstractNode*, AbstractNode*>::iterator node_it = start_new_node_reverse_mappings[l_index].begin();
				node_it != start_new_node_reverse_mappings[l_index].end(); node_it++) {
			if (node_it->first->type == NODE_TYPE_BRANCH) {
				BranchNode* new_branch_node = (BranchNode*)node_it->first;
				BranchNode* original_branch_node = (BranchNode*)node_it->second;

				new_branch_node->original_average_score = original_branch_node->original_average_score;
				new_branch_node->branch_average_score = original_branch_node->branch_average_score;

				int context_starting_depth = l_index+1 - (int)original_branch_node->scope_context.size();
				for (int i_index = 0; i_index < (int)original_branch_node->input_scope_contexts.size(); i_index++) {
					if (original_branch_node->input_scope_contexts[i_index].size() == 0) {
						new_branch_node->input_scope_context_ids.push_back(vector<int>());
						new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
						new_branch_node->input_node_context_ids.push_back(vector<int>());
						new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
					} else {
						int matching_depth = 0;
						while (true) {
							if (context_starting_depth + matching_depth + 1 >= (int)start_scope_context.size()
									|| matching_depth + 1 >= (int)original_branch_node->input_scope_contexts[i_index].size()) {
								break;
							}

							if (start_scope_context[context_starting_depth + matching_depth] == original_branch_node->input_scope_contexts[i_index][matching_depth]
									&& start_node_context[context_starting_depth + matching_depth] == original_branch_node->input_node_contexts[i_index][matching_depth]) {
								matching_depth++;
							} else {
								break;
							}
						}

						map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it =
							start_node_mappings[context_starting_depth + matching_depth]
								.find(original_branch_node->input_node_contexts[i_index][matching_depth]);
						if (it == start_node_mappings[context_starting_depth + matching_depth].end()
								|| !it->second.first
								|| start_new_node_reverse_mappings[context_starting_depth + matching_depth][it->second.second] != it->first) {
							new_branch_node->input_scope_context_ids.push_back(vector<int>());
							new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
							new_branch_node->input_node_context_ids.push_back(vector<int>());
							new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
						} else {
							vector<int> new_input_scope_context_ids;
							vector<Scope*> new_input_scope_contexts;
							vector<int> new_input_node_context_ids;
							vector<AbstractNode*> new_input_node_contexts;

							/**
							 * - set top context on explore success
							 */
							new_input_scope_context_ids.push_back(-1);
							new_input_scope_contexts.push_back(new_scope);
							new_input_node_context_ids.push_back(it->second.second->id);
							new_input_node_contexts.push_back(it->second.second);

							new_input_scope_context_ids.insert(new_input_scope_context_ids.end(),
								original_branch_node->input_scope_context_ids[i_index].begin() + matching_depth+1,
								original_branch_node->input_scope_context_ids[i_index].end());
							new_input_scope_contexts.insert(new_input_scope_contexts.end(),
								original_branch_node->input_scope_contexts[i_index].begin() + matching_depth+1,
								original_branch_node->input_scope_contexts[i_index].end());
							new_input_node_context_ids.insert(new_input_node_context_ids.end(),
								original_branch_node->input_node_context_ids[i_index].begin() + matching_depth+1,
								original_branch_node->input_node_context_ids[i_index].end());
							new_input_node_contexts.insert(new_input_node_contexts.end(),
								original_branch_node->input_node_contexts[i_index].begin() + matching_depth+1,
								original_branch_node->input_node_contexts[i_index].end());

							new_branch_node->input_scope_context_ids.push_back(new_input_scope_context_ids);
							new_branch_node->input_scope_contexts.push_back(new_input_scope_contexts);
							new_branch_node->input_node_context_ids.push_back(new_input_node_context_ids);
							new_branch_node->input_node_contexts.push_back(new_input_node_contexts);
						}
					}
				}

				new_branch_node->linear_original_input_indexes = original_branch_node->linear_original_input_indexes;
				new_branch_node->linear_original_weights = original_branch_node->linear_original_weights;
				new_branch_node->linear_branch_input_indexes = original_branch_node->linear_branch_input_indexes;
				new_branch_node->linear_branch_weights = original_branch_node->linear_branch_weights;

				new_branch_node->original_network_input_indexes = original_branch_node->original_network_input_indexes;
				if (original_branch_node->original_network == NULL) {
					new_branch_node->original_network = NULL;
				} else {
					new_branch_node->original_network = new Network(original_branch_node->original_network);
				}
				new_branch_node->branch_network_input_indexes = original_branch_node->branch_network_input_indexes;
				if (original_branch_node->branch_network == NULL) {
					new_branch_node->branch_network = NULL;
				} else {
					new_branch_node->branch_network = new Network(original_branch_node->branch_network);
				}
			}
		}
	}

	for (int l_index = starting_depth; l_index < (int)end_new_node_reverse_mappings.size(); l_index++) {
		for (map<AbstractNode*, AbstractNode*>::iterator node_it = end_new_node_reverse_mappings[l_index].begin();
				node_it != end_new_node_reverse_mappings[l_index].end(); node_it++) {
			if (node_it->second->type == NODE_TYPE_BRANCH) {
				BranchNode* new_branch_node = (BranchNode*)node_it->first;
				BranchNode* original_branch_node = (BranchNode*)node_it->second;

				new_branch_node->original_average_score = original_branch_node->original_average_score;
				new_branch_node->branch_average_score = original_branch_node->branch_average_score;

				int context_starting_depth = l_index+1 - (int)original_branch_node->scope_context.size();
				for (int i_index = 0; i_index < (int)original_branch_node->input_scope_contexts.size(); i_index++) {
					if (original_branch_node->input_scope_contexts[i_index].size() == 0) {
						new_branch_node->input_scope_context_ids.push_back(vector<int>());
						new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
						new_branch_node->input_node_context_ids.push_back(vector<int>());
						new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
					} else {
						int start_matching_depth = 0;
						while (true) {
							if (context_starting_depth + start_matching_depth + 1 >= (int)start_scope_context.size()
									|| start_matching_depth + 1 >= (int)original_branch_node->input_scope_contexts[i_index].size()) {
								break;
							}

							if (start_scope_context[context_starting_depth + start_matching_depth] == original_branch_node->input_scope_contexts[i_index][start_matching_depth]
									&& start_node_context[context_starting_depth + start_matching_depth] == original_branch_node->input_node_contexts[i_index][start_matching_depth]) {
								start_matching_depth++;
							} else {
								break;
							}
						}

						int end_matching_depth = 0;
						while (true) {
							if (context_starting_depth + end_matching_depth + 1 >= (int)end_scope_context.size()
									|| end_matching_depth + 1 >= (int)original_branch_node->input_scope_contexts[i_index].size()) {
								break;
							}

							if (end_scope_context[context_starting_depth + end_matching_depth] == original_branch_node->input_scope_contexts[i_index][end_matching_depth]
									&& end_node_context[context_starting_depth + end_matching_depth] == original_branch_node->input_node_contexts[i_index][end_matching_depth]) {
								end_matching_depth++;
							} else {
								break;
							}
						}

						/**
						 * - if context_starting_depth >= starting_depth, then not connected to start
						 *   - only check end
						 */
						if (end_matching_depth > start_matching_depth
								|| context_starting_depth >= starting_depth) {
							map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it =
								end_node_mappings[context_starting_depth + end_matching_depth]
									.find(original_branch_node->input_node_contexts[i_index][end_matching_depth]);
							if (it == end_node_mappings[context_starting_depth + end_matching_depth].end()
									|| !it->second.first
									|| end_new_node_reverse_mappings[context_starting_depth + end_matching_depth][it->second.second] != it->first) {
								new_branch_node->input_scope_context_ids.push_back(vector<int>());
								new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
								new_branch_node->input_node_context_ids.push_back(vector<int>());
								new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
							} else {
								vector<int> new_input_scope_context_ids;
								vector<Scope*> new_input_scope_contexts;
								vector<int> new_input_node_context_ids;
								vector<AbstractNode*> new_input_node_contexts;

								/**
								 * - set top context on explore success
								 */
								new_input_scope_context_ids.push_back(-1);
								new_input_scope_contexts.push_back(new_scope);
								new_input_node_context_ids.push_back(it->second.second->id);
								new_input_node_contexts.push_back(it->second.second);

								new_input_scope_context_ids.insert(new_input_scope_context_ids.end(),
									original_branch_node->input_scope_context_ids[i_index].begin() + end_matching_depth+1,
									original_branch_node->input_scope_context_ids[i_index].end());
								new_input_scope_contexts.insert(new_input_scope_contexts.end(),
									original_branch_node->input_scope_contexts[i_index].begin() + end_matching_depth+1,
									original_branch_node->input_scope_contexts[i_index].end());
								new_input_node_context_ids.insert(new_input_node_context_ids.end(),
									original_branch_node->input_node_context_ids[i_index].begin() + end_matching_depth+1,
									original_branch_node->input_node_context_ids[i_index].end());
								new_input_node_contexts.insert(new_input_node_contexts.end(),
									original_branch_node->input_node_contexts[i_index].begin() + end_matching_depth+1,
									original_branch_node->input_node_contexts[i_index].end());

								new_branch_node->input_scope_context_ids.push_back(new_input_scope_context_ids);
								new_branch_node->input_scope_contexts.push_back(new_input_scope_contexts);
								new_branch_node->input_node_context_ids.push_back(new_input_node_context_ids);
								new_branch_node->input_node_contexts.push_back(new_input_node_contexts);
							}
						} else {
							map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it =
								start_node_mappings[context_starting_depth + start_matching_depth]
									.find(original_branch_node->input_node_contexts[i_index][start_matching_depth]);
							if (it == start_node_mappings[context_starting_depth + start_matching_depth].end()
									|| !it->second.first
									|| start_new_node_reverse_mappings[context_starting_depth + start_matching_depth][it->second.second] != it->first) {
								new_branch_node->input_scope_context_ids.push_back(vector<int>());
								new_branch_node->input_scope_contexts.push_back(vector<Scope*>());
								new_branch_node->input_node_context_ids.push_back(vector<int>());
								new_branch_node->input_node_contexts.push_back(vector<AbstractNode*>());
							} else {
								vector<int> new_input_scope_context_ids;
								vector<Scope*> new_input_scope_contexts;
								vector<int> new_input_node_context_ids;
								vector<AbstractNode*> new_input_node_contexts;

								/**
								 * - set top context on explore success
								 */
								new_input_scope_context_ids.push_back(-1);
								new_input_scope_contexts.push_back(new_scope);
								new_input_node_context_ids.push_back(it->second.second->id);
								new_input_node_contexts.push_back(it->second.second);

								new_input_scope_context_ids.insert(new_input_scope_context_ids.end(),
									original_branch_node->input_scope_context_ids[i_index].begin() + start_matching_depth+1,
									original_branch_node->input_scope_context_ids[i_index].end());
								new_input_scope_contexts.insert(new_input_scope_contexts.end(),
									original_branch_node->input_scope_contexts[i_index].begin() + start_matching_depth+1,
									original_branch_node->input_scope_contexts[i_index].end());
								new_input_node_context_ids.insert(new_input_node_context_ids.end(),
									original_branch_node->input_node_context_ids[i_index].begin() + start_matching_depth+1,
									original_branch_node->input_node_context_ids[i_index].end());
								new_input_node_contexts.insert(new_input_node_contexts.end(),
									original_branch_node->input_node_contexts[i_index].begin() + start_matching_depth+1,
									original_branch_node->input_node_contexts[i_index].end());

								new_branch_node->input_scope_context_ids.push_back(new_input_scope_context_ids);
								new_branch_node->input_scope_contexts.push_back(new_input_scope_contexts);
								new_branch_node->input_node_context_ids.push_back(new_input_node_context_ids);
								new_branch_node->input_node_contexts.push_back(new_input_node_contexts);
							}
						}
					}
				}

				new_branch_node->linear_original_input_indexes = original_branch_node->linear_original_input_indexes;
				new_branch_node->linear_original_weights = original_branch_node->linear_original_weights;
				new_branch_node->linear_branch_input_indexes = original_branch_node->linear_branch_input_indexes;
				new_branch_node->linear_branch_weights = original_branch_node->linear_branch_weights;

				new_branch_node->original_network_input_indexes = original_branch_node->original_network_input_indexes;
				if (original_branch_node->original_network == NULL) {
					new_branch_node->original_network = NULL;
				} else {
					new_branch_node->original_network = new Network(original_branch_node->original_network);
				}
				new_branch_node->branch_network_input_indexes = original_branch_node->branch_network_input_indexes;
				if (original_branch_node->branch_network == NULL) {
					new_branch_node->branch_network = NULL;
				} else {
					new_branch_node->branch_network = new Network(original_branch_node->branch_network);
				}
			}
		}
	}

	return new_scope_node;
}
