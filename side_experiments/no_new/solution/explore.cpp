#include "explore.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

Explore::Explore(Scope* scope_context,
				 AbstractNode* node_context,
				 bool is_branch,
				 AbstractNode* exit_next_node) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	bool exit_is_next;
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			if (this->exit_next_node == noop_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			if (this->exit_next_node == action_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			if (this->exit_next_node == scope_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	default:
	// case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				if (this->exit_next_node == branch_node->branch_next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			} else {
				if (this->exit_next_node == branch_node->original_next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
		}
		break;
	}

	int new_num_steps;
	geometric_distribution<int> geo_distribution(0.3);
	/**
	 * - num_steps less than exit length on average to reduce solution size
	 */
	if (exit_is_next) {
		new_num_steps = 1 + geo_distribution(generator);
	} else {
		new_num_steps = geo_distribution(generator);
	}

	vector<int> possible_child_indexes;
	for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
		if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
			possible_child_indexes.push_back(c_index);
		}
	}
	uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		bool is_scope = false;
		if (possible_child_indexes.size() > 0) {
			if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
				uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
				if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
					is_scope = true;
				}
			} else {
				uniform_int_distribution<int> scope_distribution(0, 1);
				if (scope_distribution(generator) == 0) {
					is_scope = true;
				}
			}
		}
		if (is_scope) {
			this->step_types.push_back(STEP_TYPE_SCOPE);
			int child_index = possible_child_indexes[child_index_distribution(generator)];
			this->indexes.push_back(child_index);
		} else {
			this->step_types.push_back(STEP_TYPE_ACTION);
			this->indexes.push_back(-1);
		}
	}

	double average_instances_per_hit;
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			average_instances_per_hit = noop_node->average_instances_per_hit;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			average_instances_per_hit = action_node->average_instances_per_hit;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			average_instances_per_hit = scope_node->average_instances_per_hit;
		}
		break;
	default:
	// case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				average_instances_per_hit = branch_node->branch_average_instances_per_hit;
			} else {
				average_instances_per_hit = branch_node->original_average_instances_per_hit;
			}
		}
		break;
	}
	uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
	this->num_instances_until_target = until_distribution(generator);
}

Explore::~Explore() {
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			noop_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			action_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			scope_node->experiment = NULL;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				branch_node->branch_experiment = NULL;
			} else {
				branch_node->original_experiment = NULL;
			}
		}
		break;
	}
}

ExploreHistory::ExploreHistory(Explore* explore) {
	this->experiment = explore;

	this->has_explore = false;
}

ExploreState::ExploreState(Explore* explore) {
	this->experiment = explore;
}
