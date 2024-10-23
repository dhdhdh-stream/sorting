#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "condition_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void replace_condition_node(ConditionNode* to_remove) {
	for (map<int, AbstractNode*>::iterator it = to_remove->parent->nodes.begin();
			it != to_remove->parent->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				if (action_node->next_node == to_remove) {
					action_node->next_node_id = to_remove->original_next_node_id;
					action_node->next_node = to_remove->original_next_node;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->next_node == to_remove) {
					scope_node->next_node_id = to_remove->original_next_node_id;
					scope_node->next_node = to_remove->original_next_node;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->original_next_node == to_remove) {
					branch_node->original_next_node_id = to_remove->original_next_node_id;
					branch_node->original_next_node = to_remove->original_next_node;
				}
				if (branch_node->branch_next_node == to_remove) {
					branch_node->branch_next_node_id = to_remove->original_next_node_id;
					branch_node->branch_next_node = to_remove->original_next_node;
				}
			}
			break;
		case NODE_TYPE_CONDITION:
			{
				ConditionNode* condition_node = (ConditionNode*)it->second;
				if (condition_node->original_next_node == to_remove) {
					condition_node->original_next_node_id = to_remove->original_next_node_id;
					condition_node->original_next_node = to_remove->original_next_node;
				}
				if (condition_node->branch_next_node == to_remove) {
					condition_node->branch_next_node_id = to_remove->original_next_node_id;
					condition_node->branch_next_node = to_remove->original_next_node;
				}
			}
			break;
		}
	}
}

void clean_condition_nodes(Solution* parent_solution) {
	for (int s_index = 0; s_index < (int)parent_solution->scopes.size(); s_index++) {
		map<int, AbstractNode*>::iterator it = parent_solution->scopes[s_index]->nodes.begin();
		while (it != parent_solution->scopes[s_index]->nodes.end()) {
			bool can_remove = false;
			if (it->second->type == NODE_TYPE_CONDITION) {
				ConditionNode* condition_node = (ConditionNode*)it->second;
				for (int c_index = 0; c_index < (int)condition_node->conditions.size(); c_index++) {
					for (int l_index = 0; l_index < (int)condition_node->conditions[c_index].first.first.size(); l_index++) {
						Scope* scope = parent_solution->scopes[condition_node->conditions[c_index].first.first[l_index]];
						map<int, AbstractNode*>::iterator find_it = scope->nodes.find(
							condition_node->conditions[c_index].first.second[l_index]);
						if (find_it == scope->nodes.end()) {
							can_remove = true;
							break;
						}
					}
				}
			}

			if (can_remove) {
				ConditionNode* condition_node = (ConditionNode*)it->second;

				replace_condition_node(condition_node);

				delete condition_node;
				it = parent_solution->scopes[s_index]->nodes.erase(it);
			} else {
				it++;
			}
		}
	}
}

void clean_scope(Scope* scope,
				 Solution* parent_solution) {
	while (true) {
		bool removed_node = false;

		set<int> next_node_ids;
		next_node_ids.insert(0);
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)it->second;
					next_node_ids.insert(action_node->next_node_id);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					next_node_ids.insert(scope_node->next_node_id);
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;
					next_node_ids.insert(branch_node->original_next_node_id);
					next_node_ids.insert(branch_node->branch_next_node_id);
				}
				break;
			case NODE_TYPE_CONDITION:
				{
					ConditionNode* condition_node = (ConditionNode*)it->second;
					next_node_ids.insert(condition_node->original_next_node_id);
					next_node_ids.insert(condition_node->branch_next_node_id);
				}
				break;
			}
		}

		map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		while (it != scope->nodes.end()) {
			set<int>::iterator needed_it = next_node_ids.find(it->first);
			if (needed_it == next_node_ids.end()) {
				removed_node = true;

				parent_solution->clean_inputs(scope->id,
											  it->first);

				delete it->second;
				it = scope->nodes.erase(it);
			} else {
				it++;
			}
		}

		if (!removed_node) {
			break;
		}
	}

	clean_condition_nodes(parent_solution);
}
