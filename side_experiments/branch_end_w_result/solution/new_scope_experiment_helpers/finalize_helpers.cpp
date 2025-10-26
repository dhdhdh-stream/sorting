#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void NewScopeExperiment::clean() {
	this->node_context->experiment = NULL;
}

void NewScopeExperiment::add(SolutionWrapper* wrapper) {
	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->parent = this->scope_context;
	new_scope_node->id = this->scope_context->node_counter;
	this->scope_context->node_counter++;
	this->scope_context->nodes[new_scope_node->id] = new_scope_node;
	new_scope_node->scope = this->new_scope;

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		new_ending_node = new ObsNode();
		new_ending_node->parent = this->scope_context;
		new_ending_node->id = this->scope_context->node_counter;
		this->scope_context->node_counter++;

		for (map<int, AbstractNode*>::iterator it = this->scope_context->nodes.begin();
				it != this->scope_context->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->next_node == NULL) {
					obs_node->next_node_id = new_ending_node->id;
					obs_node->next_node = new_ending_node;

					new_ending_node->ancestor_ids.push_back(obs_node->id);

					break;
				}
			}
		}

		this->scope_context->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;

			for (int a_index = 0; a_index < (int)start_node->next_node->ancestor_ids.size(); a_index++) {
				if (start_node->next_node->ancestor_ids[a_index] == start_node->id) {
					start_node->next_node->ancestor_ids.erase(
						start_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}

			start_node->next_node_id = new_scope_node->id;
			start_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(start_node->id);
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
				if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
					action_node->next_node->ancestor_ids.erase(
						action_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}

			action_node->next_node_id = new_scope_node->id;
			action_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(action_node->id);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
				if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
					scope_node->next_node->ancestor_ids.erase(
						scope_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}

			scope_node->next_node_id = new_scope_node->id;
			scope_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(scope_node->id);
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->branch_next_node->ancestor_ids.erase(
							branch_node->branch_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				branch_node->branch_next_node_id = new_scope_node->id;
				branch_node->branch_next_node = new_scope_node;
			} else {
				for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->original_next_node->ancestor_ids.erase(
							branch_node->original_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				branch_node->original_next_node_id = new_scope_node->id;
				branch_node->original_next_node = new_scope_node;
			}

			new_scope_node->ancestor_ids.push_back(branch_node->id);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			if (obs_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
					if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
						obs_node->next_node->ancestor_ids.erase(
							obs_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
			}

			obs_node->next_node_id = new_scope_node->id;
			obs_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(obs_node->id);
		}
		break;
	}

	new_scope_node->next_node_id = exit_node_id;
	new_scope_node->next_node = exit_node;

	exit_node->ancestor_ids.push_back(new_scope_node->id);
}

void NewScopeOverallExperiment::clean() {
	for (int e_index = 0; e_index < (int)this->successful_experiments.size(); e_index++) {
		this->successful_experiments[e_index]->clean();
	}
}

void NewScopeOverallExperiment::add(SolutionWrapper* wrapper) {
	cout << "NewScopeExperiment add" << endl;

	wrapper->solution->scopes.push_back(this->new_scope);
	this->new_scope->id = wrapper->solution->scopes.size()-1;

	clean_scope(this->new_scope,
				wrapper);

	recursive_add_child(this->scope_context,
						wrapper,
						this->new_scope);

	wrapper->solution->last_new_scope = this->new_scope;
	wrapper->solution->new_scope_iters = 0;

	for (int e_index = 0; e_index < (int)this->successful_experiments.size(); e_index++) {
		this->successful_experiments[e_index]->add(wrapper);
	}

	this->new_scope = NULL;
}
