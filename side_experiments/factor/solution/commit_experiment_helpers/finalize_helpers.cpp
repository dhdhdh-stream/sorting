#include "commit_experiment.h"

#include "action_node.h"
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

		duplicate_explore_node->was_commit = true;

		vector<AbstractNode*> new_nodes;
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* new_action_node = new ActionNode();
				new_action_node->parent = duplicate_local_scope;
				new_action_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_action_node->id] = new_action_node;

				new_action_node->action = this->best_actions[s_index];

				new_action_node->average_instances_per_run = this->node_context->average_instances_per_run;

				new_action_node->was_commit = true;

				new_nodes.push_back(new_action_node);
			} else {
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = duplicate_local_scope;
				new_scope_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_scope_node->id] = new_scope_node;

				new_scope_node->scope = duplicate->scopes[this->best_scopes[s_index]->id];

				new_scope_node->average_instances_per_run = this->node_context->average_instances_per_run;

				new_scope_node->was_commit = true;

				new_nodes.push_back(new_scope_node);
			}
		}

		duplicate->was_commit = true;
		duplicate->commit_scope = this->scope_context;
		duplicate->commit_start_node = this->node_context;
		duplicate->commit_is_branch = this->is_branch;
		duplicate->commit_exit_node = this->best_exit_next_node;

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

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (s_index == (int)this->best_step_types.size()-1) {
				next_node_id = exit_node_id;
				next_node = exit_node;
			} else {
				next_node_id = new_nodes[s_index+1]->id;
				next_node = new_nodes[s_index+1];
			}

			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)new_nodes[s_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			} else {
				ScopeNode* scope_node = (ScopeNode*)new_nodes[s_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}

			next_node->ancestor_ids.push_back(new_nodes[s_index]->id);
		}
	}

	int experiment_index;
	for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
		if (this->node_context->experiments[e_index] == this) {
			experiment_index = e_index;
			break;
		}
	}
	this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
}
