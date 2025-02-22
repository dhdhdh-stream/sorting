#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void BranchExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
		AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

		vector<AbstractNode*> new_nodes;
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNode* new_action_node = new ActionNode();
				new_action_node->parent = duplicate_local_scope;
				new_action_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_action_node->id] = new_action_node;

				new_action_node->action = this->best_actions[s_index];

				new_nodes.push_back(new_action_node);
			} else {
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = duplicate_local_scope;
				new_scope_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_scope_node->id] = new_scope_node;

				new_scope_node->scope = duplicate->scopes[this->best_scopes[s_index]->id];

				new_nodes.push_back(new_scope_node);
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

			exit_node_id = new_ending_node->id;
			exit_node = new_ending_node;
		} else {
			exit_node_id = this->best_exit_next_node->id;
			exit_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
		}

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->select_percentage == 1.0) {
		#endif /* MDEBUG */
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
		} else {
			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->parent = duplicate_local_scope;
			new_branch_node->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;
			duplicate_local_scope->nodes[new_branch_node->id] = new_branch_node;

			new_branch_node->average_val = this->new_average_score;
			new_branch_node->factor_ids = this->new_factor_ids;
			new_branch_node->factor_weights = this->new_factor_weights;

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
					action_node->next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = action_node->next_node_id;
					new_branch_node->original_next_node = action_node->next_node;
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
					scope_node->next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = scope_node->next_node_id;
					new_branch_node->original_next_node = scope_node->next_node;
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
					ObsNode* obs_node = (ObsNode*)duplicate_explore_node;

					if (obs_node->next_node == NULL) {
						if (new_ending_node != NULL) {
							new_ending_node->ancestor_ids.push_back(new_branch_node->id);

							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;
						} else {
							new_ending_node = new ObsNode();
							new_ending_node->parent = duplicate_local_scope;
							new_ending_node->id = duplicate_local_scope->node_counter;
							duplicate_local_scope->node_counter++;

							for (map<int, AbstractNode*>::iterator it = duplicate_local_scope->nodes.begin();
									it != duplicate_local_scope->nodes.end(); it++) {
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

							duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

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

			if (this->best_step_types.size() == 0) {
				exit_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->branch_next_node_id = exit_node_id;
				new_branch_node->branch_next_node = exit_node;
			} else {
				new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->branch_next_node_id = new_nodes[0]->id;
				new_branch_node->branch_next_node = new_nodes[0];
			}

			switch (duplicate_explore_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)duplicate_explore_node;

					action_node->next_node_id = new_branch_node->id;
					action_node->next_node = new_branch_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

					scope_node->next_node_id = new_branch_node->id;
					scope_node->next_node = new_branch_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

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
					ObsNode* obs_node = (ObsNode*)duplicate_explore_node;

					obs_node->next_node_id = new_branch_node->id;
					obs_node->next_node = new_branch_node;
				}
				break;
			}
			new_branch_node->ancestor_ids.push_back(duplicate_explore_node->id);

			for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
				ObsNode* obs_node = (ObsNode*)duplicate_local_scope->nodes[this->new_factor_ids[f_index].first];
				Factor* factor = obs_node->factors[this->new_factor_ids[f_index].second];

				factor->link(duplicate);

				obs_node->is_used = true;
			}

			#if defined(MDEBUG) && MDEBUG
			if (this->verify_problems.size() > 0) {
				duplicate->verify_problems = this->verify_problems;
				this->verify_problems.clear();
				duplicate->verify_seeds = this->verify_seeds;
				duplicate->verify_can_random = this->verify_can_random;

				new_branch_node->verify_key = this;
				new_branch_node->verify_scores = this->verify_scores;
			}
			#endif /* MDEBUG */
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

	this->node_context->experiment= NULL;
}
