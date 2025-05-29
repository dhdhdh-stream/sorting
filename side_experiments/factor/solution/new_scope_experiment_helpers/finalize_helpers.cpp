#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void NewScopeExperiment::clean() {
	this->scope_context->new_scope_experiment = NULL;

	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
		this->successful_location_starts[s_index]->experiment = NULL;
	}
}

void NewScopeExperiment::add() {
	this->new_scope->id = solution->scopes.size();
	solution->scopes.push_back(this->new_scope);

	#if defined(MDEBUG) && MDEBUG
	for (map<int, AbstractNode*>::iterator it = this->new_scope->nodes.begin();
			it != this->new_scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->verify_key = this;
			}
			break;
		}
	}

	solution->verify_problems = this->verify_problems;
	this->verify_problems.clear();
	solution->verify_seeds = this->verify_seeds;
	#endif /* MDEBUG */

	clean_scope(this->new_scope);

	ObsNode* new_local_ending_node = NULL;

	this->new_scope->child_scopes = this->scope_context->child_scopes;
	this->scope_context->child_scopes.push_back(this->new_scope);

	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
		ScopeNode* new_scope_node = this->successful_scope_nodes[s_index];
		new_scope_node->parent = this->scope_context;
		this->scope_context->nodes[new_scope_node->id] = new_scope_node;

		if (new_scope_node->next_node == NULL) {
			if (new_local_ending_node == NULL) {
				new_local_ending_node = new ObsNode();
				new_local_ending_node->parent = this->scope_context;
				new_local_ending_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				for (map<int, AbstractNode*>::iterator it = this->scope_context->nodes.begin();
						it != this->scope_context->nodes.end(); it++) {
					if (it->second->type == NODE_TYPE_OBS) {
						ObsNode* obs_node = (ObsNode*)it->second;
						if (obs_node->next_node == NULL) {
							obs_node->next_node_id = new_local_ending_node->id;
							obs_node->next_node = new_local_ending_node;

							new_local_ending_node->ancestor_ids.push_back(obs_node->id);

							break;
						}
					}
				}

				this->scope_context->nodes[new_local_ending_node->id] = new_local_ending_node;

				new_local_ending_node->next_node_id = -1;
				new_local_ending_node->next_node = NULL;
			}

			new_scope_node->next_node_id = new_local_ending_node->id;
			new_scope_node->next_node = new_local_ending_node;

			new_local_ending_node->ancestor_ids.push_back(new_scope_node->id);
		} else {
			new_scope_node->next_node->ancestor_ids.push_back(new_scope_node->id);
		}

		switch (this->successful_location_starts[s_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->successful_location_starts[s_index];

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
				ScopeNode* scope_node = (ScopeNode*)this->successful_location_starts[s_index];

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
				BranchNode* branch_node = (BranchNode*)this->successful_location_starts[s_index];

				if (this->successful_location_is_branch[s_index]) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					branch_node->branch_next_node_id = new_scope_node->id;
					branch_node->branch_next_node = new_scope_node;

					new_scope_node->ancestor_ids.push_back(branch_node->id);
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

					new_scope_node->ancestor_ids.push_back(branch_node->id);
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->successful_location_starts[s_index];

				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
					if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
						obs_node->next_node->ancestor_ids.erase(
							obs_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				obs_node->next_node_id = new_scope_node->id;
				obs_node->next_node = new_scope_node;

				new_scope_node->ancestor_ids.push_back(obs_node->id);
			}
			break;
		}
	}
	this->successful_scope_nodes.clear();

	this->new_scope = NULL;
}
