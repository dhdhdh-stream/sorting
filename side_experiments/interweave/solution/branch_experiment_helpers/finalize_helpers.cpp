#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void BranchExperiment::finalize() {
	this->node_context->experiment = NULL;

	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		vector<AbstractNode*> new_nodes;
		for (int s_index = 0; s_index < (int)this->step_types[this->best_concurrent_index].size(); s_index++) {
			if (this->step_types[this->best_concurrent_index][s_index] == STEP_TYPE_ACTION) {
				ActionNode* new_action_node = new ActionNode();
				new_action_node->parent = this->scope_context;
				new_action_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;
				this->scope_context->nodes[new_action_node->id] = new_action_node;

				new_action_node->action = this->actions[this->best_concurrent_index][s_index];

				new_nodes.push_back(new_action_node);
			} else {
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = this->scope_context;
				new_scope_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;
				this->scope_context->nodes[new_scope_node->id] = new_scope_node;

				new_scope_node->scope = this->scopes[this->best_concurrent_index][s_index];

				new_nodes.push_back(new_scope_node);
			}

			ObsNode* new_obs_node = new ObsNode();
			new_obs_node->parent = this->scope_context;
			new_obs_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
			this->scope_context->nodes[new_obs_node->id] = new_obs_node;

			new_nodes.push_back(new_obs_node);
		}

		ObsNode* new_ending_node = NULL;

		int exit_node_id;
		AbstractNode* exit_node;
		if (this->exit_next_node[this->best_concurrent_index] == NULL) {
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
			exit_node_id = this->exit_next_node[this->best_concurrent_index]->id;
			exit_node = this->exit_next_node[this->best_concurrent_index];
		}

		if (this->select_percentage[this->best_concurrent_index] == 1.0) {
			int start_node_id;
			AbstractNode* start_node;
			if (this->step_types[this->best_concurrent_index].size() == 0) {
				start_node_id = exit_node_id;
				start_node = exit_node;
			} else {
				start_node_id = new_nodes[0]->id;
				start_node = new_nodes[0];
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

					action_node->next_node_id = start_node_id;
					action_node->next_node = start_node;

					start_node->ancestor_ids.push_back(action_node->id);
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

					scope_node->next_node_id = start_node_id;
					scope_node->next_node = start_node;

					start_node->ancestor_ids.push_back(scope_node->id);
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

					obs_node->next_node_id = start_node_id;
					obs_node->next_node = start_node;

					start_node->ancestor_ids.push_back(obs_node->id);
				}
				break;
			}
		} else {
			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->parent = this->scope_context;
			new_branch_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
			this->scope_context->nodes[new_branch_node->id] = new_branch_node;

			new_branch_node->average_val = this->new_average_score[this->best_concurrent_index];
			new_branch_node->factor_ids = this->new_factor_ids[this->best_concurrent_index];
			new_branch_node->factor_weights = this->new_factor_weights[this->best_concurrent_index];

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
					action_node->next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = action_node->next_node_id;
					new_branch_node->original_next_node = action_node->next_node;
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
					scope_node->next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = scope_node->next_node_id;
					new_branch_node->original_next_node = scope_node->next_node;
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
						branch_node->branch_next_node->ancestor_ids.push_back(new_branch_node->id);

						new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
						new_branch_node->original_next_node = branch_node->branch_next_node;
					} else {
						for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
							if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
								branch_node->original_next_node->ancestor_ids.erase(
									branch_node->original_next_node->ancestor_ids.begin() + a_index);
								break;
							}
						}
						branch_node->original_next_node->ancestor_ids.push_back(new_branch_node->id);

						new_branch_node->original_next_node_id = branch_node->original_next_node_id;
						new_branch_node->original_next_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;

					if (obs_node->next_node == NULL) {
						if (new_ending_node != NULL) {
							new_ending_node->ancestor_ids.push_back(new_branch_node->id);

							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;
						} else {
							new_ending_node = new ObsNode();
							new_ending_node->parent = this->scope_context;
							new_ending_node->id = this->scope_context->node_counter;
							this->scope_context->node_counter++;

							for (map<int, AbstractNode*>::iterator it = this->scope_context->nodes.begin();
									it != this->scope_context->nodes.end(); it++) {
								if (it->second->type == NODE_TYPE_OBS) {
									ObsNode* p_obs_node = (ObsNode*)it->second;
									if (p_obs_node->next_node == NULL) {
										p_obs_node->next_node_id = new_ending_node->id;
										p_obs_node->next_node = new_ending_node;

										new_ending_node->ancestor_ids.push_back(p_obs_node->id);

										break;
									}
								}
							}

							this->scope_context->nodes[new_ending_node->id] = new_ending_node;

							new_ending_node->next_node_id = -1;
							new_ending_node->next_node = NULL;

							new_ending_node->ancestor_ids.push_back(new_branch_node->id);

							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;
						}
					} else {
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
					}
				}
				break;
			}

			if (this->step_types[this->best_concurrent_index].size() == 0) {
				exit_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->branch_next_node_id = exit_node_id;
				new_branch_node->branch_next_node = exit_node;
			} else {
				new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->branch_next_node_id = new_nodes[0]->id;
				new_branch_node->branch_next_node = new_nodes[0];
			}

			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;

					action_node->next_node_id = new_branch_node->id;
					action_node->next_node = new_branch_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;

					scope_node->next_node_id = new_branch_node->id;
					scope_node->next_node = new_branch_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;

					if (this->is_branch) {
						branch_node->branch_next_node_id = new_branch_node->id;
						branch_node->branch_next_node = new_branch_node;
					} else {
						branch_node->original_next_node_id = new_branch_node->id;
						branch_node->original_next_node = new_branch_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;

					obs_node->next_node_id = new_branch_node->id;
					obs_node->next_node = new_branch_node;
				}
				break;
			}
			new_branch_node->ancestor_ids.push_back(this->node_context->id);
		}

		for (int f_index = 0; f_index < (int)this->new_factor_ids[this->best_concurrent_index].size(); f_index++) {
			ObsNode* obs_node = (ObsNode*)this->scope_context->nodes[this->new_factor_ids[this->best_concurrent_index][f_index].first];
			Factor* factor = obs_node->factors[this->new_factor_ids[this->best_concurrent_index][f_index].second];

			factor->link();

			obs_node->is_used = true;
		}

		for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (n_index == (int)new_nodes.size()-1) {
				next_node_id = exit_node_id;
				next_node = exit_node;
			} else {
				next_node_id = new_nodes[n_index+1]->id;
				next_node = new_nodes[n_index+1];
			}

			switch (new_nodes[n_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)new_nodes[n_index];
					action_node->next_node_id = next_node_id;
					action_node->next_node = next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)new_nodes[n_index];
					scope_node->next_node_id = next_node_id;
					scope_node->next_node = next_node;
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)new_nodes[n_index];
					obs_node->next_node_id = next_node_id;
					obs_node->next_node = next_node;
				}
				break;
			}

			next_node->ancestor_ids.push_back(new_nodes[n_index]->id);
		}
	}
}
