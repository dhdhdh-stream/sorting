#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper) {
	/**
	 * - remove no longer accessible nodes
	 */
	while (true) {
		bool removed_node = false;

		set<int> next_node_ids;
		next_node_ids.insert(0);
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)it->second;
					next_node_ids.insert(start_node->next_node_id);
				}
				break;
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

				switch (it->second->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;

						for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
							if (action_node->next_node->ancestor_ids[a_index] == it->second->id) {
								action_node->next_node->ancestor_ids.erase(
									action_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)it->second;

						for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
							if (scope_node->next_node->ancestor_ids[a_index] == it->second->id) {
								scope_node->next_node->ancestor_ids.erase(
									scope_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)it->second;

						for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->original_next_node->ancestor_ids[a_index] == it->second->id) {
								branch_node->original_next_node->ancestor_ids.erase(
									branch_node->original_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}

						for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->branch_next_node->ancestor_ids[a_index] == it->second->id) {
								branch_node->branch_next_node->ancestor_ids.erase(
									branch_node->branch_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				case NODE_TYPE_BRANCH_END:
					{
						BranchEndNode* branch_end_node = (BranchEndNode*)it->second;

						for (int a_index = 0; a_index < (int)branch_end_node->next_node->ancestor_ids.size(); a_index++) {
							if (branch_end_node->next_node->ancestor_ids[a_index] == it->second->id) {
								branch_end_node->next_node->ancestor_ids.erase(
									branch_end_node->next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
					}
					break;
				}

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
