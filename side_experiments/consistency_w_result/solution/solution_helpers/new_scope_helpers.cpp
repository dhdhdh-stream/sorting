/**
 * - don't worry about replacing original with new scope
 *   - as new scope may be placed within original
 * 
 * TODO:
 * - when trying new scope, temporarily replace
 */

#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

const int NEW_SCOPE_MIN_NUM_NODES = 3;
const int CREATE_NEW_SCOPE_NUM_TRIES = 50;

void ancestor_helper(AbstractNode* curr_node,
					 set<AbstractNode*>& ancestors) {
	for (int a_index = 0; a_index < (int)curr_node->ancestor_ids.size(); a_index++) {
		AbstractNode* next_node = curr_node->parent->nodes[curr_node->ancestor_ids[a_index]];
		set<AbstractNode*>::iterator it = ancestors.find(next_node);
		if (it == ancestors.end()) {
			ancestors.insert(next_node);

			ancestor_helper(next_node,
							ancestors);
		}
	}
}

void children_helper(AbstractNode* curr_node,
					 set<AbstractNode*>& children) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)curr_node;
			if (action_node->next_node != NULL) {
				set<AbstractNode*>::iterator it = children.find(action_node->next_node);
				if (it == children.end()) {
					children.insert(action_node->next_node);

					children_helper(action_node->next_node,
									children);
				}
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)curr_node;
			if (scope_node->next_node != NULL) {
				set<AbstractNode*>::iterator it = children.find(scope_node->next_node);
				if (it == children.end()) {
					children.insert(scope_node->next_node);

					children_helper(scope_node->next_node,
									children);
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)curr_node;
			if (branch_node->original_next_node != NULL) {
				set<AbstractNode*>::iterator original_it = children.find(branch_node->original_next_node);
				if (original_it == children.end()) {
					children.insert(branch_node->original_next_node);

					children_helper(branch_node->original_next_node,
									children);
				}
			}
			if (branch_node->branch_next_node != NULL) {
				set<AbstractNode*>::iterator branch_it = children.find(branch_node->branch_next_node);
				if (branch_it == children.end()) {
					children.insert(branch_node->branch_next_node);

					children_helper(branch_node->branch_next_node,
									children);
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)curr_node;
			if (branch_end_node->next_node != NULL) {
				set<AbstractNode*>::iterator it = children.find(branch_end_node->next_node);
				if (it == children.end()) {
					children.insert(branch_end_node->next_node);

					children_helper(branch_end_node->next_node,
									children);
				}
			}
		}
		break;
	}
}

Scope* create_new_scope(Scope* scope_context) {
	if (scope_context->nodes.size() < NEW_SCOPE_MIN_NODES) {
		return NULL;
	}

	vector<AbstractNode*> possible_starting_nodes;
	for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
			it != scope_context->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_ACTION:
		case NODE_TYPE_SCOPE:
		case NODE_TYPE_BRANCH:
			possible_starting_nodes.push_back(it->second);
			break;
		}
	}
	uniform_int_distribution<int> starting_node_distribution(0, possible_starting_nodes.size()-1);

	for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
		AbstractNode* potential_start_node = possible_starting_nodes[starting_node_distribution(generator)];

		vector<AbstractNode*> possible_exits;
		scope_context->random_new_scope_end_activate(
			potential_start_node,
			possible_exits);
		uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
		/**
		 * - don't include possible_exits end
		 */
		AbstractNode* potential_end_node = possible_exits[exit_distribution(generator)];

		set<AbstractNode*> children;
		children_helper(potential_start_node,
						children);

		set<AbstractNode*> ancestors;
		ancestor_helper(potential_end_node,
						ancestors);

		set<AbstractNode*> potential_included_nodes;
		for (set<AbstractNode*>::iterator it = children.begin(); it != children.end(); it++) {
			if (ancestors.find(*it) != ancestors.end()) {
				potential_included_nodes.insert(*it);
			}
		}
		potential_included_nodes.insert(potential_start_node);
		potential_included_nodes.insert(potential_end_node);

		int num_meaningful_nodes = 0;
		for (set<AbstractNode*>::iterator it = potential_included_nodes.begin();
				it != potential_included_nodes.end(); it++) {
			switch ((*it)->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				num_meaningful_nodes++;
				break;
			}
		}
		if (num_meaningful_nodes >= NEW_SCOPE_MIN_NUM_NODES) {
			Scope* new_scope = new Scope();
			new_scope->id = -1;

			new_scope->node_counter = 0;

			StartNode* starting_node = new StartNode();
			starting_node->parent = new_scope;
			starting_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[starting_node->id] = starting_node;

			map<AbstractNode*, AbstractNode*> node_mappings;
			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				switch ((*node_it)->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)(*node_it);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_action_node->action = original_action_node->action;

						node_mappings[original_action_node] = new_action_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)(*node_it);

						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_scope_node->scope = original_scope_node->scope;

						node_mappings[original_scope_node] = new_scope_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* original_branch_node = (BranchNode*)(*node_it);

						BranchNode* new_branch_node = new BranchNode();
						new_branch_node->parent = new_scope;
						new_branch_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_branch_node->id] = new_branch_node;

						node_mappings[original_branch_node] = new_branch_node;
					}
					break;
				case NODE_TYPE_BRANCH_END:
					{
						BranchEndNode* original_branch_end_node = (BranchEndNode*)(*node_it);

						BranchEndNode* new_branch_end_node = new BranchEndNode();
						new_branch_end_node->parent = new_scope;
						new_branch_end_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_branch_end_node->id] = new_branch_end_node;

						node_mappings[original_branch_end_node] = new_branch_end_node;
					}
					break;
				}
			}

			starting_node->next_node_id = node_mappings[potential_start_node]->id;
			starting_node->next_node = node_mappings[potential_start_node];

			starting_node->next_node->ancestor_ids.push_back(starting_node->id);

			for (map<AbstractNode*, AbstractNode*>::iterator node_it = node_mappings.begin();
					node_it != node_mappings.end(); node_it++) {
				switch (node_it->first->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)node_it->first;
						ActionNode* new_action_node = (ActionNode*)node_mappings[original_action_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_action_node->next_node);
						if (it == node_mappings.end()) {
							new_action_node->next_node_id = -1;
							new_action_node->next_node = NULL;
						} else {
							new_action_node->next_node_id = it->second->id;
							new_action_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_action_node->id);
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)node_it->first;
						ScopeNode* new_scope_node = (ScopeNode*)node_mappings[original_scope_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_scope_node->next_node);
						if (it == node_mappings.end()) {
							new_scope_node->next_node_id = -1;
							new_scope_node->next_node = NULL;
						} else {
							new_scope_node->next_node_id = it->second->id;
							new_scope_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_scope_node->id);
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* original_branch_node = (BranchNode*)node_it->first;
						BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

						new_branch_node->val_network = new Network(original_branch_node->val_network);

						map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
							.find(original_branch_node->original_next_node);
						if (original_it == node_mappings.end()) {
							new_branch_node->original_next_node_id = -1;
							new_branch_node->original_next_node = NULL;
						} else {
							new_branch_node->original_next_node_id = original_it->second->id;
							new_branch_node->original_next_node = original_it->second;

							original_it->second->ancestor_ids.push_back(new_branch_node->id);
						}
						map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
							.find(original_branch_node->branch_next_node);
						if (branch_it == node_mappings.end()) {
							new_branch_node->branch_next_node_id = -1;
							new_branch_node->branch_next_node = NULL;
						} else {
							new_branch_node->branch_next_node_id = branch_it->second->id;
							new_branch_node->branch_next_node = branch_it->second;

							branch_it->second->ancestor_ids.push_back(new_branch_node->id);
						}
					}
					break;
				case NODE_TYPE_BRANCH_END:
					{
						BranchEndNode* original_branch_end_node = (BranchEndNode*)node_it->first;
						BranchEndNode* new_branch_end_node = (BranchEndNode*)node_mappings[original_branch_end_node];

						BranchNode* branch_start_node = (BranchNode*)node_mappings[original_branch_end_node->branch_start_node];
						new_branch_end_node->branch_start_node_id = branch_start_node->id;
						new_branch_end_node->branch_start_node = branch_start_node;

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_branch_end_node->next_node);
						if (it == node_mappings.end()) {
							new_branch_end_node->next_node_id = -1;
							new_branch_end_node->next_node = NULL;
						} else {
							new_branch_end_node->next_node_id = it->second->id;
							new_branch_end_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_branch_end_node->id);
						}
					}
					break;
				}
			}

			new_scope->child_scopes = scope_context->child_scopes;

			new_scope->existing_pre_obs.push_back(vector<vector<double>>());
			new_scope->existing_post_obs.push_back(vector<vector<double>>());

			new_scope->explore_pre_obs.push_back(vector<vector<double>>());
			new_scope->explore_post_obs.push_back(vector<vector<double>>());

			return new_scope;
		}
	}

	return NULL;
}

void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope) {
	curr_parent->child_scopes.push_back(new_scope);

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		bool is_needed = false;
		bool is_added = false;
		for (int c_index = 0; c_index < (int)wrapper->solution->scopes[s_index]->child_scopes.size(); c_index++) {
			if (wrapper->solution->scopes[s_index]->child_scopes[c_index] == curr_parent) {
				is_needed = true;
			}

			if (wrapper->solution->scopes[s_index]->child_scopes[c_index] == new_scope) {
				is_added = true;
			}
		}

		if (is_needed && !is_added) {
			recursive_add_child(wrapper->solution->scopes[s_index],
								wrapper,
								new_scope);
		}
	}
}
