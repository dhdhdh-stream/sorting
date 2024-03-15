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

pair<bool,AbstractNode*> node_helper(AbstractNode* curr_node,
									 map<AbstractNode*, pair<bool,AbstractNode*>>& node_mappings,
									 map<AbstractNode*, AbstractNode*>& new_node_reverse_mappings,
									 Scope* new_scope) {
	map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it = node_mappings.find(curr_node);
	if (it != node_mappings.end()) {
		return it->second;
	} else {
		pair<bool,AbstractNode*> mapping;
		if (curr_node == NULL) {
			mapping = {false, NULL};
		} else {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = node_helper(
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

					pair<bool,AbstractNode*> next_mapping = node_helper(
						scope_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					vector<pair<bool,AbstractNode*>> catch_mappings;
					for (map<int, AbstractNode*>::iterator it = scope_node->catches.begin();
							it != scope_node->catches.end(); it++) {
						catch_mappings.push_back(node_helper(
							it->second,
							node_mappings,
							new_node_reverse_mappings,
							new_scope));
					}

					bool has_path = false;
					if (next_mapping.first) {
						has_path = true;
					} else {
						for (int m_index = 0; m_index < (int)catch_mappings.size(); m_index++) {
							if (catch_mappings[m_index].first) {
								has_path = true;
								break;
							}
						}
					}

					if (has_path) {
						ScopeNode* new_scope_node = new ScopeNode();

						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_node_reverse_mappings[new_scope_node] = scope_node;

						new_scope_node->scope = scope_node->scope;

						/**
						 * - simply go to end of scope if next_node has no path
						 */
						if (next_mapping.second == NULL) {
							new_scope_node->next_node_id = -1;
						} else {
							new_scope_node->next_node_id = next_mapping.second->id;
						}
						new_scope_node->next_node = next_mapping.second;

						int m_index = 0;
						for (map<int, AbstractNode*>::iterator it = scope_node->catches.begin();
								it != scope_node->catches.end(); it++) {
							if (catch_mappings[m_index].first) {
								if (catch_mappings[m_index].second == NULL) {
									new_scope_node->catch_ids[it->first] = -1;
								} else {
									new_scope_node->catch_ids[it->first] = catch_mappings[m_index].second->id;
								}
								new_scope_node->catches[it->first] = catch_mappings[m_index].second;
							}

							m_index++;
						}

						mapping = {true, new_scope_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					pair<bool,AbstractNode*> original_mapping = node_helper(
						branch_node->original_next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					pair<bool,AbstractNode*> branch_mapping = node_helper(
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

						new_branch_node->scope_context_ids = branch_node->scope_context_ids;
						new_branch_node->scope_context_ids.back() = -1;
						new_branch_node->scope_context = branch_node->scope_context;
						new_branch_node->scope_context.back() = new_scope;
						new_branch_node->node_context_ids = branch_node->node_context_ids;
						new_branch_node->node_context_ids.back() = new_branch_node->id;
						new_branch_node->node_context = branch_node->node_context;
						new_branch_node->node_context.back() = new_branch_node;
						new_branch_node->is_fuzzy_match = branch_node->is_fuzzy_match;

						new_branch_node->is_pass_through = branch_node->is_pass_through;

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

				break;
			case NODE_TYPE_EXIT:
				mapping = {false, NULL};

				break;
			}
		}

		node_mappings[curr_node] = mapping;
		return mapping;
	}
}

ScopeNode* create_subscope(Scope* parent_scope) {
	if (parent_scope->sample_run == NULL) {
		return NULL;
	}

	bool has_meaningful_actions = false;
	for (int n_index = 0; n_index < (int)parent_scope->sample_run->node_histories.size(); n_index++) {
		AbstractNodeHistory* node_history = parent_scope->sample_run->node_histories[n_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)node_history->node;
			if (action_node->action.move != ACTION_NOOP) {
				has_meaningful_actions = true;
				break;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			has_meaningful_actions = true;
			break;
		}
	}
	if (!has_meaningful_actions) {
		return NULL;
	}

	AbstractNode* start_node;
	AbstractNode* end_node;
	uniform_int_distribution<int> distribution(0, parent_scope->sample_run->node_histories.size()-1);
	while (true) {
		int start_index = distribution(generator);
		int end_index = distribution(generator);
		if (start_index < end_index
				&& parent_scope->sample_run->node_histories[end_index]->node->type != NODE_TYPE_BRANCH) {
			bool has_meaningful_actions = false;
			for (int n_index = start_index; n_index < end_index+1; n_index++) {
				AbstractNodeHistory* node_history = parent_scope->sample_run->node_histories[n_index];
				if (node_history->node->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)node_history->node;
					if (action_node->action.move != ACTION_NOOP) {
						has_meaningful_actions = true;
						break;
					}
				} else if (node_history->node->type == NODE_TYPE_SCOPE) {
					has_meaningful_actions = true;
					break;
				}
			}
			if (has_meaningful_actions) {
				start_node = parent_scope->sample_run->node_histories[start_index]->node;
				end_node = parent_scope->sample_run->node_histories[end_index]->node;

				break;
			}
		}
	}

	Scope* new_scope = new Scope();
	// don't set id/increment scope_counter until train
	new_scope->node_counter = 0;
	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->scope = new_scope;

	map<AbstractNode*, pair<bool,AbstractNode*>> node_mappings;
	map<AbstractNode*, AbstractNode*> new_node_reverse_mappings;

	if (end_node->type == NODE_TYPE_ACTION) {
		ActionNode* original_action_node = (ActionNode*)end_node;

		ActionNode* new_action_node = new ActionNode();

		new_action_node->parent = new_scope;
		new_action_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_action_node->id] = new_action_node;

		new_node_reverse_mappings[new_action_node] = original_action_node;

		new_action_node->action = original_action_node->action;

		new_action_node->next_node_id = -1;
		new_action_node->next_node = NULL;

		node_mappings[end_node] = {true, new_action_node};
	} else {
		ScopeNode* original_scope_node = (ScopeNode*)end_node;

		ScopeNode* new_scope_node = new ScopeNode();

		new_scope_node->parent = new_scope;
		new_scope_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_scope_node->id] = new_scope_node;

		new_node_reverse_mappings[new_scope_node] = original_scope_node;

		new_scope_node->scope = original_scope_node->scope;

		new_scope_node->next_node_id = -1;
		new_scope_node->next_node = NULL;

		for (map<int, AbstractNode*>::iterator it = original_scope_node->catches.begin();
				it != original_scope_node->catches.end(); it++) {
			new_scope_node->catch_ids[it->first] = -1;
			new_scope_node->catches[it->first] = NULL;
		}

		node_mappings[end_node] = {true, new_scope_node};
	}
	node_helper(start_node,
				node_mappings,
				new_node_reverse_mappings,
				new_scope);
	AbstractNode* new_start_node = node_mappings[start_node].second;

	if (new_start_node->type == NODE_TYPE_ACTION
			&& ((ActionNode*)new_start_node)->action.move == ACTION_NOOP) {
		new_scope->starting_node_id = new_start_node->id;
		new_scope->starting_node = new_start_node;
	} else {
		ActionNode* new_noop_action_node = new ActionNode();
		new_noop_action_node->parent = new_scope;
		new_noop_action_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_noop_action_node->action = Action(ACTION_NOOP);
		new_noop_action_node->next_node_id = new_start_node->id;
		new_noop_action_node->next_node = new_start_node;
		new_scope->nodes[new_noop_action_node->id] = new_noop_action_node;

		new_scope->starting_node_id = new_noop_action_node->id;
		new_scope->starting_node = new_noop_action_node;
	}

	for (map<AbstractNode*, AbstractNode*>::iterator node_it = new_node_reverse_mappings.begin();
			node_it != new_node_reverse_mappings.end(); node_it++) {
		if (node_it->first->type == NODE_TYPE_BRANCH) {
			BranchNode* new_branch_node = (BranchNode*)node_it->first;
			BranchNode* original_branch_node = (BranchNode*)node_it->second;

			new_branch_node->original_average_score = original_branch_node->original_average_score;
			new_branch_node->branch_average_score = original_branch_node->branch_average_score;

			new_branch_node->input_scope_context_ids = original_branch_node->input_scope_context_ids;
			new_branch_node->input_scope_contexts = original_branch_node->input_scope_contexts;
			new_branch_node->input_node_context_ids = original_branch_node->input_node_context_ids;
			new_branch_node->input_node_contexts = original_branch_node->input_node_contexts;
			new_branch_node->input_is_fuzzy_match = original_branch_node->input_is_fuzzy_match;
			new_branch_node->input_strict_root_indexes = original_branch_node->input_strict_root_indexes;
			uniform_int_distribution<int> fuzzy_keep_distribution(0, 1);
			for (int i_index = 0; i_index < (int)new_branch_node->input_scope_contexts.size(); i_index++) {
				if (new_branch_node->input_scope_contexts[i_index].size() > 0) {
					if (new_branch_node->input_is_fuzzy_match[i_index]) {
						vector<int> potential_indexes;
						for (int c_index = 0; c_index < (int)new_branch_node->input_scope_contexts[i_index].size(); c_index++) {
							if (new_branch_node->input_scope_contexts[i_index][c_index] == parent_scope) {
								potential_indexes.push_back(c_index);
							}
						}
						if (potential_indexes.size() > 0) {
							if (fuzzy_keep_distribution(generator) != 0) {
								uniform_int_distribution<int> layer_distribution(0, potential_indexes.size()-1);
								int replace_layer = layer_distribution(generator);
								map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it = 
									node_mappings.find(new_branch_node->input_node_contexts[i_index][replace_layer]);
								if (it == node_mappings.end()
										|| new_node_reverse_mappings[it->second.second] != it->first) {
									new_branch_node->input_scope_context_ids[i_index].clear();
									new_branch_node->input_scope_contexts[i_index].clear();
									new_branch_node->input_node_context_ids[i_index].clear();
									new_branch_node->input_node_contexts[i_index].clear();
								} else {
									new_branch_node->input_scope_context_ids[i_index][replace_layer] = -1;
									new_branch_node->input_scope_contexts[i_index][replace_layer] = new_scope;
									new_branch_node->input_node_context_ids[i_index][replace_layer] = it->second.second->id;
									new_branch_node->input_node_contexts[i_index][replace_layer] = it->second.second;
								}
							}
						}
					} else {
						if (new_branch_node->input_strict_root_indexes[i_index] == (int)new_branch_node->scope_context.size()-1) {
							map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it =
								node_mappings.find(new_branch_node->input_node_contexts[i_index][0]);
							if (it == node_mappings.end()
									|| new_node_reverse_mappings[it->second.second] != it->first) {
								new_branch_node->input_scope_context_ids[i_index].clear();
								new_branch_node->input_scope_contexts[i_index].clear();
								new_branch_node->input_node_context_ids[i_index].clear();
								new_branch_node->input_node_contexts[i_index].clear();
							} else {
								new_branch_node->input_scope_context_ids[i_index][0] = -1;
								new_branch_node->input_scope_contexts[i_index][0] = new_scope;
								new_branch_node->input_node_context_ids[i_index][0] = it->second.second->id;
								new_branch_node->input_node_contexts[i_index][0] = it->second.second;
							}
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

	return new_scope_node;
}
