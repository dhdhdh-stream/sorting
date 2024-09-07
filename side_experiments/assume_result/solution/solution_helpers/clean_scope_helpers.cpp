#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void clean_scope(Scope* scope) {
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
			case NODE_TYPE_RETURN:
				{
					ReturnNode* return_node = (ReturnNode*)it->second;
					next_node_ids.insert(return_node->passed_next_node_id);
					next_node_ids.insert(return_node->skipped_next_node_id);
				}
				break;
			}
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

void clean_branch_node(Scope* scope) {
	while (true) {
		bool removed_node = false;

		map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		while (it != scope->nodes.end()) {
			if (it->second->type == NODE_TYPE_BRANCH) {
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->original_next_node == branch_node->branch_next_node) {
					for (map<int, AbstractNode*>::iterator inner_it = scope->nodes.begin();
							inner_it != scope->nodes.end(); inner_it++) {
						switch (inner_it->second->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* node = (ActionNode*)inner_it->second;
								if (node->next_node == branch_node) {
									node->next_node_id = branch_node->original_next_node_id;
									node->next_node = branch_node->original_next_node;
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* node = (ScopeNode*)inner_it->second;
								if (node->next_node == branch_node) {
									node->next_node_id = branch_node->original_next_node_id;
									node->next_node = branch_node->original_next_node;
								}
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* node = (BranchNode*)inner_it->second;
								if (node->original_next_node == branch_node) {
									node->original_next_node_id = branch_node->original_next_node_id;
									node->original_next_node = branch_node->original_next_node;
								}
								if (node->branch_next_node == branch_node) {
									node->branch_next_node_id = branch_node->original_next_node_id;
									node->branch_next_node = branch_node->original_next_node;
								}
							}
							break;
						case NODE_TYPE_RETURN:
							{
								ReturnNode* node = (ReturnNode*)inner_it->second;
								if (node->passed_next_node == branch_node) {
									node->passed_next_node_id = branch_node->original_next_node_id;
									node->passed_next_node = branch_node->original_next_node;
								}
								if (node->skipped_next_node == branch_node) {
									node->skipped_next_node_id = branch_node->original_next_node_id;
									node->skipped_next_node = branch_node->original_next_node;
								}
							}
							break;
						}
					}

					removed_node = true;

					scope->clean_node(it->first);

					delete it->second;
					it = scope->nodes.erase(it);

					continue;
				}
			}

			it++;
		}

		if (!removed_node) {
			break;
		}
	}
}

void clean_scope_node_helper(Scope* scope,
							 AbstractNode* original_node,
							 AbstractNode* new_node) {
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)it->second;
				if (node->next_node == original_node) {
					if (new_node == NULL) {
						node->next_node_id = -1;
						node->next_node = NULL;
					} else {
						node->next_node_id = new_node->id;
						node->next_node = new_node;
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)it->second;
				if (node->next_node == original_node) {
					if (new_node == NULL) {
						node->next_node_id = -1;
						node->next_node = NULL;
					} else {
						node->next_node_id = new_node->id;
						node->next_node = new_node;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* node = (BranchNode*)it->second;
				if (node->original_next_node == original_node) {
					if (new_node == NULL) {
						node->original_next_node_id = -1;
						node->original_next_node = NULL;
					} else {
						node->original_next_node_id = new_node->id;
						node->original_next_node = new_node;
					}
				}
				if (node->branch_next_node == original_node) {
					if (new_node == NULL) {
						node->branch_next_node_id = -1;
						node->branch_next_node = NULL;
					} else {
						node->branch_next_node_id = new_node->id;
						node->branch_next_node = new_node;
					}
				}
			}
			break;
		case NODE_TYPE_RETURN:
			{
				ReturnNode* node = (ReturnNode*)it->second;
				
				if (node->previous_location == original_node) {
					if (new_node == NULL) {
						node->previous_location_id = -1;
						node->previous_location = NULL;
					} else {
						node->previous_location_id = new_node->id;
						node->previous_location = new_node;
					}
				}

				if (node->passed_next_node == original_node) {
					if (new_node == NULL) {
						node->passed_next_node_id = -1;
						node->passed_next_node = NULL;
					} else {
						node->passed_next_node_id = new_node->id;
						node->passed_next_node = new_node;
					}
				}
				if (node->skipped_next_node == original_node) {
					if (new_node == NULL) {
						node->skipped_next_node_id = -1;
						node->skipped_next_node = NULL;
					} else {
						node->skipped_next_node_id = new_node->id;
						node->skipped_next_node = new_node;
					}
				}
			}
			break;
		}
	}
}

void clean_scope_node(Solution* parent_solution,
					  Scope* to_remove) {
	for (int s_index = 0; s_index < (int)parent_solution->scopes.size(); s_index++) {
		map<int, AbstractNode*>::iterator it = parent_solution->scopes[s_index]->nodes.begin();
		while (it != parent_solution->scopes[s_index]->nodes.end()) {
			if (it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->scope == to_remove) {
					clean_scope_node_helper(parent_solution->scopes[s_index],
											scope_node,
											scope_node->next_node);

					parent_solution->scopes[s_index]->clean_node(it->first);

					delete it->second;
					it = parent_solution->scopes[s_index]->nodes.erase(it);

					continue;
				}
			}

			it++;
		}
	}
}

void clean_scope_node(Solution* parent_solution,
					  Scope* to_remove,
					  Action to_replace) {
	for (int s_index = 0; s_index < (int)parent_solution->scopes.size(); s_index++) {
		map<int, AbstractNode*>::iterator it = parent_solution->scopes[s_index]->nodes.begin();
		while (it != parent_solution->scopes[s_index]->nodes.end()) {
			if (it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->scope == to_remove) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = parent_solution->scopes[s_index];
					new_action_node->id = parent_solution->scopes[s_index]->node_counter;
					parent_solution->scopes[s_index]->node_counter++;
					new_action_node->action = to_replace;
					new_action_node->next_node_id = scope_node->next_node_id;
					new_action_node->next_node = scope_node->next_node;
					parent_solution->scopes[s_index]->nodes[new_action_node->id] = new_action_node;

					clean_scope_node_helper(parent_solution->scopes[s_index],
											scope_node,
											new_action_node);

					parent_solution->scopes[s_index]->clean_node(it->first);

					delete it->second;
					it = parent_solution->scopes[s_index]->nodes.erase(it);

					continue;
				}
			}

			it++;
		}
	}
}

void clean_scope_node(Solution* parent_solution,
					  Scope* to_remove,
					  Scope* to_replace) {
	for (int s_index = 0; s_index < (int)parent_solution->scopes.size(); s_index++) {
		map<int, AbstractNode*>::iterator it = parent_solution->scopes[s_index]->nodes.begin();
		while (it != parent_solution->scopes[s_index]->nodes.end()) {
			if (it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->scope == to_remove) {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = parent_solution->scopes[s_index];
					new_scope_node->id = parent_solution->scopes[s_index]->node_counter;
					parent_solution->scopes[s_index]->node_counter++;
					new_scope_node->scope = to_replace;
					new_scope_node->index = to_replace->scope_node_index;
					to_replace->scope_node_index++;
					new_scope_node->next_node_id = scope_node->next_node_id;
					new_scope_node->next_node = scope_node->next_node;
					parent_solution->scopes[s_index]->nodes[new_scope_node->id] = new_scope_node;

					clean_scope_node_helper(parent_solution->scopes[s_index],
											scope_node,
											new_scope_node);

					parent_solution->scopes[s_index]->clean_node(it->first);

					delete it->second;
					it = parent_solution->scopes[s_index]->nodes.erase(it);

					continue;
				}
			}

			it++;
		}
	}
}
