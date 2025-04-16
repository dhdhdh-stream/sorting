#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void BranchExperiment::clean() {
	this->node_context->experiment = NULL;
}

void BranchExperiment::add() {
	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = this->scope_context;
			new_action_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
			this->scope_context->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->best_actions[s_index];

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = this->scope_context;
			new_scope_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
			this->scope_context->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = this->best_scopes[s_index];

			new_nodes.push_back(new_scope_node);
		}
	}

	if (this->select_percentage == 1.0) {
		int start_node_id;
		AbstractNode* start_node;
		if (this->best_step_types.size() == 0) {
			if (this->exit_next_node == NULL) {
				start_node_id = -1;
				start_node = NULL;
			} else {
				start_node_id = this->exit_next_node->id;
				start_node = this->exit_next_node;
			}
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

				if (start_node != NULL) {
					start_node->ancestor_ids.push_back(action_node->id);
				}
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

				if (start_node != NULL) {
					start_node->ancestor_ids.push_back(scope_node->id);
				}
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

				if (start_node != NULL) {
					start_node->ancestor_ids.push_back(branch_node->id);
				}
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

				if (start_node != NULL) {
					start_node->ancestor_ids.push_back(obs_node->id);
				}
			}
			break;
		}
	} else {
		BranchNode* new_branch_node = new BranchNode();
		new_branch_node->parent = this->scope_context;
		new_branch_node->id = this->scope_context->node_counter;
		this->scope_context->node_counter++;
		this->scope_context->nodes[new_branch_node->id] = new_branch_node;

		new_branch_node->average_val = this->new_average_score;
		new_branch_node->factor_ids = this->new_factor_ids;
		new_branch_node->factor_weights = this->new_factor_weights;

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

				if (obs_node->next_node != NULL) {
					for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
						if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
							obs_node->next_node->ancestor_ids.erase(
								obs_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					obs_node->next_node->ancestor_ids.push_back(new_branch_node->id);
				}

				new_branch_node->original_next_node_id = obs_node->next_node_id;
				new_branch_node->original_next_node = obs_node->next_node;
			}
			break;
		}

		if (this->best_step_types.size() == 0) {
			if (this->exit_next_node == NULL) {
				new_branch_node->branch_next_node_id = -1;
				new_branch_node->branch_next_node = NULL;
			} else {
				this->exit_next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->branch_next_node_id = this->exit_next_node->id;
				new_branch_node->branch_next_node = this->exit_next_node;
			}
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

		for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
			ObsNode* obs_node = (ObsNode*)this->scope_context->nodes[this->new_factor_ids[f_index].first];
			Factor* factor = obs_node->factors[this->new_factor_ids[f_index].second];

			factor->link(solution);

			obs_node->is_used = true;
		}

		#if defined(MDEBUG) && MDEBUG
		if (this->verify_problems.size() > 0) {
			solution->verify_problems = this->verify_problems;
			this->verify_problems.clear();
			solution->verify_seeds = this->verify_seeds;

			new_branch_node->verify_key = this;
			new_branch_node->verify_scores = this->verify_scores;
		}
		#endif /* MDEBUG */
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->best_step_types.size()-1) {
			if (this->exit_next_node == NULL) {
				next_node_id = -1;
				next_node = NULL;
			} else {
				next_node_id = this->exit_next_node->id;
				next_node = this->exit_next_node;
			}
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

		if (next_node != NULL) {
			next_node->ancestor_ids.push_back(new_nodes[s_index]->id);
		}
	}
}
