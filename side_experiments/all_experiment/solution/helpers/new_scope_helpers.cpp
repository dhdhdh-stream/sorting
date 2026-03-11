#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int NEW_SCOPE_MIN_NODES = 20;

const int NEW_SCOPE_MIN_NUM_NODES = 3;
const int CREATE_NEW_SCOPE_NUM_TRIES = 50;

const double OUTER_PATH_MIN_RATIO = 0.2;
/**
 * - since outer discarded afterwards, simply try to pull as much as possible
 */

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
			set<AbstractNode*>::iterator it = children.find(action_node->next_node);
			if (it == children.end()) {
				children.insert(action_node->next_node);

				children_helper(action_node->next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(scope_node->next_node);
			if (it == children.end()) {
				children.insert(scope_node->next_node);

				children_helper(scope_node->next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)curr_node;
			set<AbstractNode*>::iterator original_it = children.find(branch_node->original_next_node);
			if (original_it == children.end()) {
				children.insert(branch_node->original_next_node);

				children_helper(branch_node->original_next_node,
								children);
			}
			set<AbstractNode*>::iterator branch_it = children.find(branch_node->branch_next_node);
			if (branch_it == children.end()) {
				children.insert(branch_node->branch_next_node);

				children_helper(branch_node->branch_next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)curr_node;
			if (obs_node->next_node != NULL) {
				set<AbstractNode*>::iterator it = children.find(obs_node->next_node);
				if (it == children.end()) {
					children.insert(obs_node->next_node);

					children_helper(obs_node->next_node,
									children);
				}
			}
		}
		break;
	}
}

void create_new_scope(AbstractNode* potential_start_node,
					  AbstractNode* potential_end_node,
					  Scope*& new_scope) {
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
		new_scope = new Scope();
		new_scope->is_outer = false;
		new_scope->id = -1;

		new_scope->node_counter = 0;

		StartNode* starting_node = new StartNode();
		starting_node->parent = new_scope;
		starting_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[starting_node->id] = starting_node;

		ObsNode* new_ending_node = new ObsNode();
		new_ending_node->parent = new_scope;
		new_ending_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_ending_node->id] = new_ending_node;
		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

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
			case NODE_TYPE_OBS:
				{
					ObsNode* original_obs_node = (ObsNode*)(*node_it);

					ObsNode* new_obs_node = new ObsNode();
					new_obs_node->parent = new_scope;
					new_obs_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_obs_node->id] = new_obs_node;

					node_mappings[original_obs_node] = new_obs_node;
				}
				break;
			}
		}
		for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
				node_it != potential_included_nodes.end(); node_it++) {
			if ((*node_it)->type == NODE_TYPE_BRANCH) {
				BranchNode* original_branch_node = (BranchNode*)(*node_it);

				map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings.find(original_branch_node->original_next_node);
				map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings.find(original_branch_node->branch_next_node);
				if (original_it != node_mappings.end()
						&& branch_it != node_mappings.end()) {
					BranchNode* new_branch_node = new BranchNode();
					new_branch_node->parent = new_scope;
					new_branch_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_branch_node->id] = new_branch_node;

					node_mappings[original_branch_node] = new_branch_node;
				} else if (original_it != node_mappings.end()) {
					node_mappings[original_branch_node] = original_it->second;
				} else if (branch_it != node_mappings.end()) {
					node_mappings[original_branch_node] = branch_it->second;
				}
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
						new_action_node->next_node_id = new_ending_node->id;
						new_action_node->next_node = new_ending_node;

						new_ending_node->ancestor_ids.push_back(new_action_node->id);
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
						new_scope_node->next_node_id = new_ending_node->id;
						new_scope_node->next_node = new_ending_node;

						new_ending_node->ancestor_ids.push_back(new_scope_node->id);
					} else {
						new_scope_node->next_node_id = it->second->id;
						new_scope_node->next_node = it->second;

						it->second->ancestor_ids.push_back(new_scope_node->id);
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				if (node_it->first->type == node_it->second->type) {
					BranchNode* original_branch_node = (BranchNode*)node_it->first;
					BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

					for (int n_index = 0; n_index < (int)original_branch_node->networks.size(); n_index++) {
						new_branch_node->networks.push_back(new Network(original_branch_node->networks[n_index]));
					}

					map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
						.find(original_branch_node->original_next_node);
					if (original_it == node_mappings.end()) {
						new_branch_node->original_next_node_id = new_ending_node->id;
						new_branch_node->original_next_node = new_ending_node;

						new_ending_node->ancestor_ids.push_back(new_branch_node->id);
					} else {
						new_branch_node->original_next_node_id = original_it->second->id;
						new_branch_node->original_next_node = original_it->second;

						original_it->second->ancestor_ids.push_back(new_branch_node->id);
					}
					map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
						.find(original_branch_node->branch_next_node);
					if (branch_it == node_mappings.end()) {
						new_branch_node->branch_next_node_id = new_ending_node->id;
						new_branch_node->branch_next_node = new_ending_node;

						new_ending_node->ancestor_ids.push_back(new_branch_node->id);
					} else {
						new_branch_node->branch_next_node_id = branch_it->second->id;
						new_branch_node->branch_next_node = branch_it->second;

						branch_it->second->ancestor_ids.push_back(new_branch_node->id);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* original_obs_node = (ObsNode*)node_it->first;
					ObsNode* new_obs_node = (ObsNode*)node_mappings[original_obs_node];

					map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
						.find(original_obs_node->next_node);
					if (it == node_mappings.end()) {
						new_obs_node->next_node_id = new_ending_node->id;
						new_obs_node->next_node = new_ending_node;

						new_ending_node->ancestor_ids.push_back(new_obs_node->id);
					} else {
						new_obs_node->next_node_id = it->second->id;
						new_obs_node->next_node = it->second;

						it->second->ancestor_ids.push_back(new_obs_node->id);
					}
				}
				break;
			}
		}

		Scope* parent_scope = potential_start_node->parent;
		new_scope->child_scopes = parent_scope->child_scopes;

		clean_scope(new_scope);
	} else {
		new_scope = NULL;
	}
}

void create_new_scope(Scope* scope_context,
					  SolutionWrapper* wrapper,
					  Scope*& new_scope,
					  Scope*& parent_scope) {
	parent_scope = scope_context;

	if (parent_scope->nodes.size() < NEW_SCOPE_MIN_NODES) {
		new_scope = NULL;
		return;
	}

	uniform_int_distribution<int> node_distribution(1, parent_scope->nodes.size()-1);
	for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
		AbstractNode* potential_start_node = next(parent_scope->nodes.begin(), node_distribution(generator))->second;
		AbstractNode* potential_end_node = next(parent_scope->nodes.begin(), node_distribution(generator))->second;

		Scope* potential_new_scope = NULL;
		create_new_scope(potential_start_node,
						 potential_end_node,
						 potential_new_scope);

		if (potential_new_scope != NULL) {
			new_scope = potential_new_scope;
			return;
		}
	}

	new_scope = NULL;
	return;
}

void outer_create_new_scope(Scope* scope_context,
							SolutionWrapper* wrapper,
							Scope*& new_scope,
							Scope*& parent_scope) {
	while (true) {
		uniform_int_distribution<int> outer_distribution(0, wrapper->solution->outer_root_scope_ids.size()-1);
		int scope_id = wrapper->solution->outer_root_scope_ids[outer_distribution(generator)];
		parent_scope = wrapper->solution->outer_scopes[scope_id];

		if (parent_scope->nodes.size() < NEW_SCOPE_MIN_NODES) {
			new_scope = NULL;
			continue;
		}

		for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
			vector<AbstractNode*> path;
			parent_scope->random_activate(path);

			int min_length = OUTER_PATH_MIN_RATIO * (double)path.size();
			uniform_int_distribution<int> length_distribution(min_length, path.size());
			int length = length_distribution(generator);
			uniform_int_distribution<int> start_distribution(0, path.size() - length);
			int start_index = start_distribution(generator);

			AbstractNode* potential_start_node = path[start_index];
			AbstractNode* potential_end_node = path[start_index + length-1];

			Scope* potential_new_scope = NULL;
			create_new_scope(potential_start_node,
							 potential_end_node,
							 potential_new_scope);

			if (potential_new_scope != NULL) {
				new_scope = potential_new_scope;
				return;
			}
		}
	}
}
