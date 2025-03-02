#include "commit_experiment.h"

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void CommitExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
		AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

		for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
			duplicate_local_scope->nodes[this->new_nodes[n_index]->id] = this->new_nodes[n_index];
		}

		int exit_node_id;
		AbstractNode* exit_node;
		if (this->best_exit_next_node == NULL) {
			ObsNode* new_ending_node = new ObsNode();
			new_ending_node->parent = duplicate_local_scope;
			new_ending_node->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;

			for (map<int, AbstractNode*>::iterator it = duplicate_local_scope->nodes.begin();
					it != duplicate_local_scope->nodes.end(); it++) {
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

			duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			new_ending_node->average_instances_per_run = this->node_context->average_instances_per_run;

			exit_node_id = new_ending_node->id;
			exit_node = new_ending_node;
		} else {
			exit_node_id = this->best_exit_next_node->id;
			exit_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
		}

		int start_node_id;
		AbstractNode* start_node;
		if (this->best_step_types.size() == 0) {
			start_node_id = exit_node_id;
			start_node = exit_node;
		} else {
			start_node_id = new_nodes[0]->id;
			start_node = new_nodes[0];
		}

		switch (duplicate_explore_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_explore_node;

				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				action_node->next_node_id = start_node_id;
				action_node->next_node = start_node;

				start_node->ancestor_ids.push_back(action_node->id);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

				for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
					if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
						scope_node->next_node->ancestor_ids.erase(
							scope_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				scope_node->next_node_id = start_node_id;
				scope_node->next_node = start_node;

				start_node->ancestor_ids.push_back(scope_node->id);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

				if (this->is_branch) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					branch_node->branch_next_node_id = start_node_id;
					branch_node->branch_next_node = start_node;
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					branch_node->original_next_node_id = start_node_id;
					branch_node->original_next_node = start_node;
				}

				start_node->ancestor_ids.push_back(branch_node->id);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)duplicate_explore_node;

				if (obs_node->next_node != NULL) {
					for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
						if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
							obs_node->next_node->ancestor_ids.erase(
								obs_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
				}

				obs_node->next_node_id = start_node_id;
				obs_node->next_node = start_node;

				start_node->ancestor_ids.push_back(obs_node->id);
			}
			break;
		}

		for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (n_index == (int)this->new_nodes.size()-1) {
				next_node_id = exit_node_id;
				next_node = exit_node;
			} else {
				next_node_id = this->new_nodes[n_index+1]->id;
				next_node = this->new_nodes[n_index+1];
			}

			switch (this->new_nodes[n_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->new_nodes[n_index];
					action_node->next_node_id = next_node_id;
					action_node->next_node = next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->new_nodes[n_index];
					scope_node->next_node_id = next_node_id;
					scope_node->next_node = next_node;
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->new_nodes[n_index];
					obs_node->next_node_id = next_node_id;
					obs_node->next_node = next_node;
				}
				break;
			}

			next_node->ancestor_ids.push_back(this->new_nodes[n_index]->id);
		}
		this->new_nodes.clear();

		this->best_experiment->finalize(duplicate);
	}

	this->node_context->experiment = NULL;
}
