#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

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
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* info_branch_node = (InfoBranchNode*)it->second;
					next_node_ids.insert(info_branch_node->original_next_node_id);
					next_node_ids.insert(info_branch_node->branch_next_node_id);
				}
				break;
			case NODE_TYPE_BRANCH_END:
				{
					BranchEndNode* branch_end_node = (BranchEndNode*)it->second;
					next_node_ids.insert(branch_end_node->next_node_id);
				}
				break;
			}
		}

		map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		while (it != scope->nodes.end()) {
			set<int>::iterator needed_it = next_node_ids.find(it->first);
			if (needed_it == next_node_ids.end()) {
				removed_node = true;

				parent_solution->clean_node(scope->id,
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
}

void clean_info_scope(InfoScope* scope) {
	while (true) {
		bool removed_node = false;

		set<int> next_node_ids;
		next_node_ids.insert(0);
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			ActionNode* action_node = (ActionNode*)it->second;
			next_node_ids.insert(action_node->next_node_id);
		}

		map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		while (it != scope->nodes.end()) {
			set<int>::iterator needed_it = next_node_ids.find(it->first);
			if (needed_it == next_node_ids.end()) {
				removed_node = true;

				scope->clean_node(it->first);

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
}
