#include "new_scope_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::add() {
	this->scope_context->nodes[this->successful_scope_node->id] = this->successful_scope_node;

	if (this->successful_scope_node->next_node == NULL) {
		ObsNode* new_ending_node = new ObsNode();
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

		this->successful_scope_node->next_node_id = new_ending_node->id;
		this->successful_scope_node->next_node = new_ending_node;

		new_ending_node->ancestor_ids.push_back(this->successful_scope_node->id);
	} else {
		this->successful_scope_node->next_node->ancestor_ids.push_back(this->successful_scope_node->id);
	}

	switch (this->node_context->type) {
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

			action_node->next_node_id = this->successful_scope_node->id;
			action_node->next_node = this->successful_scope_node;

			this->successful_scope_node->ancestor_ids.push_back(action_node->id);
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

			scope_node->next_node_id = this->successful_scope_node->id;
			scope_node->next_node = this->successful_scope_node;

			this->successful_scope_node->ancestor_ids.push_back(scope_node->id);
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

				branch_node->branch_next_node_id = this->successful_scope_node->id;
				branch_node->branch_next_node = this->successful_scope_node;

				this->successful_scope_node->ancestor_ids.push_back(branch_node->id);
			} else {
				for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->original_next_node->ancestor_ids.erase(
							branch_node->original_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				branch_node->original_next_node_id = this->successful_scope_node->id;
				branch_node->original_next_node = this->successful_scope_node;

				this->successful_scope_node->ancestor_ids.push_back(branch_node->id);
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
				if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
					obs_node->next_node->ancestor_ids.erase(
						obs_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}

			obs_node->next_node_id = this->successful_scope_node->id;
			obs_node->next_node = this->successful_scope_node;

			this->successful_scope_node->ancestor_ids.push_back(obs_node->id);
		}
		break;
	}

	this->successful_scope_node = NULL;
}
