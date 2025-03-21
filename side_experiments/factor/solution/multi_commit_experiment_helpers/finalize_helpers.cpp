#include "multi_commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void MultiCommitExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
		AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

		for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
			this->new_nodes[n_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->new_nodes[n_index]->id] = this->new_nodes[n_index];

			switch (this->new_nodes[n_index]->type) {
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->new_nodes[n_index];
					scope_node->scope = duplicate->scopes[scope_node->scope->id];
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->new_nodes[n_index];
					for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
						Factor* factor = obs_node->factors[f_index];
						for (int i_index = 0; i_index < (int)factor->inputs.size(); i_index++) {
							for (int l_index = 0; l_index < (int)factor->inputs[i_index].first.first.size(); l_index++) {
								factor->inputs[i_index].first.first[l_index] =
									duplicate->scopes[factor->inputs[i_index].first.first[l_index]->id];
							}
						}
					}
				}
				break;
			}
		}

		ObsNode* new_ending_node = NULL;

		int exit_node_id;
		AbstractNode* exit_node;
		if (this->best_exit_next_node == NULL) {
			new_ending_node = new ObsNode();
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

		vector<AbstractNode*> save_new_nodes;
		for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
			if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* new_action_node = new ActionNode();
				new_action_node->parent = duplicate_local_scope;
				new_action_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_action_node->id] = new_action_node;

				new_action_node->action = this->save_actions[s_index];

				save_new_nodes.push_back(new_action_node);
			} else {
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = duplicate_local_scope;
				new_scope_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_scope_node->id] = new_scope_node;

				new_scope_node->scope = duplicate->scopes[this->save_scopes[s_index]->id];

				save_new_nodes.push_back(new_scope_node);
			}
		}

		int save_exit_node_id;
		AbstractNode* save_exit_node;
		if (this->save_exit_next_node == NULL) {
			if (new_ending_node == NULL) {
				new_ending_node = new ObsNode();
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
			}

			save_exit_node_id = new_ending_node->id;
			save_exit_node = new_ending_node;
		} else {
			save_exit_node_id = this->save_exit_next_node->id;
			save_exit_node = duplicate_local_scope->nodes[this->save_exit_next_node->id];
		}

		BranchNode* new_branch_node = new BranchNode();
		new_branch_node->parent = duplicate_local_scope;
		new_branch_node->id = duplicate_local_scope->node_counter;
		duplicate_local_scope->node_counter++;
		duplicate_local_scope->nodes[new_branch_node->id] = new_branch_node;

		new_branch_node->average_val = 1.0;
		// new_branch_node->average_val = this->commit_new_average_score;
		// new_branch_node->factor_ids = this->commit_new_factor_ids;
		// new_branch_node->factor_weights = this->commit_new_factor_weights;

		ObsNode* obs_node = (ObsNode*)this->new_nodes[this->step_iter-1];

		for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
			if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
				obs_node->next_node->ancestor_ids.erase(
					obs_node->next_node->ancestor_ids.begin() + a_index);
				break;
			}
		}
		obs_node->next_node->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->original_next_node_id = obs_node->next_node_id;
		new_branch_node->original_next_node = obs_node->next_node;

		if (this->save_step_types.size() == 0) {
			save_exit_node->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->branch_next_node_id = save_exit_node_id;
			new_branch_node->branch_next_node = save_exit_node;
		} else {
			save_new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->branch_next_node_id = save_new_nodes[0]->id;
			new_branch_node->branch_next_node = save_new_nodes[0];
		}

		obs_node->next_node_id = new_branch_node->id;
		obs_node->next_node = new_branch_node;

		new_branch_node->ancestor_ids.push_back(this->new_nodes[this->step_iter-1]->id);

		for (int f_index = 0; f_index < (int)this->commit_new_factor_ids.size(); f_index++) {
			ObsNode* obs_node = (ObsNode*)duplicate_local_scope->nodes[this->commit_new_factor_ids[f_index].first];
			Factor* factor = obs_node->factors[this->commit_new_factor_ids[f_index].second];

			factor->link(duplicate);

			obs_node->is_used = true;
		}

		for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (s_index == (int)this->save_step_types.size()-1) {
				next_node_id = save_exit_node_id;
				next_node = save_exit_node;
			} else {
				next_node_id = save_new_nodes[s_index+1]->id;
				next_node = save_new_nodes[s_index+1];
			}

			if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)save_new_nodes[s_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			} else {
				ScopeNode* scope_node = (ScopeNode*)save_new_nodes[s_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}

			next_node->ancestor_ids.push_back(save_new_nodes[s_index]->id);
		}

		this->new_nodes.clear();
	}

	this->node_context->experiment = NULL;
}
