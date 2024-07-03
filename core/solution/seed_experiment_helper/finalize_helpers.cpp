#include "seed_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "info_branch_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void SeedExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		if (this->is_pass_through) {
			new_pass_through(duplicate);
		} else {
			new_branch(duplicate);
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

void SeedExperiment::new_branch(Solution* duplicate) {
	cout << "SeedExperiment new_branch" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

	ActionNode* new_ending_node = NULL;

	// seed
	int exit_node_id;
	AbstractNode* exit_node;
	if (this->best_seed_exit_next_node == NULL) {
		new_ending_node = new ActionNode();
		new_ending_node->parent = duplicate_local_scope;
		new_ending_node->id = duplicate_local_scope->node_counter;
		duplicate_local_scope->node_counter++;
		duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->action = Action(ACTION_NOOP);

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->best_seed_exit_next_node->id;
		exit_node = duplicate_local_scope->nodes[this->best_seed_exit_next_node->id];
	}

	int start_node_id;
	AbstractNode* start_node;
	if (this->best_seed_step_types.size() == 0) {
		start_node_id = this->best_seed_exit_next_node->id;
		start_node = duplicate_local_scope->nodes[this->best_seed_exit_next_node->id];
	} else {
		if (this->best_seed_step_types[0] == STEP_TYPE_ACTION) {
			start_node_id = this->best_seed_actions[0]->id;
			start_node = this->best_seed_actions[0];
		} else {
			start_node_id = this->best_seed_scopes[0]->id;
			start_node = this->best_seed_scopes[0];
		}
	}

	switch (duplicate_explore_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			action_node->next_node_id = start_node_id;
			action_node->next_node = start_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			scope_node->next_node_id = start_node_id;
			scope_node->next_node = start_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				info_branch_node->branch_next_node_id = start_node_id;
				info_branch_node->branch_next_node = start_node;
			} else {
				info_branch_node->original_next_node_id = start_node_id;
				info_branch_node->original_next_node = start_node;
			}
		}
		break;
	}

	for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->best_seed_step_types.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			if (this->best_seed_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->best_seed_actions[s_index+1]->id;
				next_node = this->best_seed_actions[s_index+1];
			} else {
				next_node_id = this->best_seed_scopes[s_index+1]->id;
				next_node = this->best_seed_scopes[s_index+1];
			}
		}

		if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_seed_actions[s_index]->next_node_id = next_node_id;
			this->best_seed_actions[s_index]->next_node = next_node;

			this->best_seed_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_seed_actions[s_index]->id] = this->best_seed_actions[s_index];
		} else {
			this->best_seed_scopes[s_index]->next_node_id = next_node_id;
			this->best_seed_scopes[s_index]->next_node = next_node;

			this->best_seed_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_seed_scopes[s_index]->id] = this->best_seed_scopes[s_index];

			this->best_seed_scopes[s_index]->scope = duplicate->scopes[this->best_seed_scopes[s_index]->scope->id];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_back_step_types.size(); s_index++) {
		if (this->best_back_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_back_actions[s_index]->parent = duplicate_local_scope;
			this->best_back_actions[s_index]->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;
		} else {
			this->best_back_scopes[s_index]->parent = duplicate_local_scope;
			this->best_back_scopes[s_index]->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;
		}
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = duplicate_local_scope;
	new_branch_node->id = duplicate_local_scope->node_counter;
	duplicate_local_scope->node_counter++;
	duplicate_local_scope->nodes[new_branch_node->id] = new_branch_node;

	if (this->branch_index == (int)this->best_seed_step_types.size()-1) {
		new_branch_node->branch_next_node_id = exit_node_id;
		new_branch_node->branch_next_node = exit_node;
	} else {
		if (this->best_seed_step_types[this->branch_index+1] == STEP_TYPE_ACTION) {
			new_branch_node->branch_next_node_id = this->best_seed_actions[this->branch_index+1]->id;
			new_branch_node->branch_next_node = this->best_seed_actions[this->branch_index+1];
		} else {
			new_branch_node->branch_next_node_id = this->best_seed_scopes[this->branch_index+1]->id;
			new_branch_node->branch_next_node = this->best_seed_scopes[this->branch_index+1];
		}
	}

	if (this->best_back_step_types.size() == 0) {
		if (this->best_back_exit_next_node == NULL) {
			if (new_ending_node == NULL) {
				new_ending_node = new ActionNode();
				new_ending_node->parent = duplicate_local_scope;
				new_ending_node->id = duplicate_local_scope->node_counter;
				duplicate_local_scope->node_counter++;
				duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

				new_ending_node->action = Action(ACTION_NOOP);

				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;
			}

			new_branch_node->original_next_node_id = new_ending_node->id;
			new_branch_node->original_next_node = new_ending_node;
		} else {
			new_branch_node->original_next_node_id = this->best_back_exit_next_node->id;
			new_branch_node->original_next_node = duplicate_local_scope->nodes[this->best_back_exit_next_node->id];
		}
	} else {
		if (this->best_back_step_types[0] == STEP_TYPE_ACTION) {
			new_branch_node->original_next_node_id = this->best_back_actions[0]->id;
			new_branch_node->original_next_node = this->best_back_actions[0];
		} else {
			new_branch_node->original_next_node_id = this->best_back_scopes[0]->id;
			new_branch_node->original_next_node = this->best_back_scopes[0];
		}
	}

	if (this->branch_index == -1) {
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
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

				if (this->is_branch) {
					info_branch_node->branch_next_node_id = new_branch_node->id;
					info_branch_node->branch_next_node = new_branch_node;
				} else {
					info_branch_node->original_next_node_id = new_branch_node->id;
					info_branch_node->original_next_node = new_branch_node;
				}
			}
			break;

		}
	} else {
		if (this->best_seed_step_types[this->branch_index] == STEP_TYPE_ACTION) {
			this->best_seed_actions[this->branch_index]->next_node_id = new_branch_node->id;
			this->best_seed_actions[this->branch_index]->next_node = new_branch_node;
		} else {
			this->best_seed_scopes[this->branch_index]->next_node_id = new_branch_node->id;
			this->best_seed_scopes[this->branch_index]->next_node = new_branch_node;
		}
	}

	for (int s_index = 0; s_index < (int)this->best_back_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->best_back_step_types.size()-1) {
			if (this->best_back_exit_next_node == NULL) {
				if (new_ending_node == NULL) {
					new_ending_node = new ActionNode();
					new_ending_node->parent = duplicate_local_scope;
					new_ending_node->id = duplicate_local_scope->node_counter;
					duplicate_local_scope->node_counter++;
					duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->action = Action(ACTION_NOOP);

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;
				}

				next_node_id = new_ending_node->id;
				next_node = new_ending_node;
			} else {
				next_node_id = this->best_back_exit_next_node->id;
				next_node = duplicate_local_scope->nodes[this->best_back_exit_next_node->id];
			}
		} else {
			if (this->best_back_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->best_back_actions[s_index+1]->id;
				next_node = this->best_back_actions[s_index+1];
			} else {
				next_node_id = this->best_back_scopes[s_index+1]->id;
				next_node = this->best_back_scopes[s_index+1];
			}
		}

		if (this->best_back_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_back_actions[s_index]->next_node_id = next_node_id;
			this->best_back_actions[s_index]->next_node = next_node;

			this->best_back_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_back_actions[s_index]->id] = this->best_back_actions[s_index];
		} else {
			this->best_back_scopes[s_index]->next_node_id = next_node_id;
			this->best_back_scopes[s_index]->next_node = next_node;

			this->best_back_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_back_scopes[s_index]->id] = this->best_back_scopes[s_index];

			this->best_back_scopes[s_index]->scope = duplicate->scopes[this->best_back_scopes[s_index]->scope->id];
		}
	}

	for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
		vector<int> scope_context_ids;
		for (int c_index = 0; c_index < (int)this->new_input_scope_contexts[i_index].size(); c_index++) {
			scope_context_ids.push_back(this->new_input_scope_contexts[i_index][c_index]->id);
		}
		new_branch_node->input_scope_context_ids.push_back(scope_context_ids);

		vector<AbstractScope*> scope_context;
		for (int c_index = 0; c_index < (int)this->new_input_scope_contexts[i_index].size(); c_index++) {
			int scope_id = this->new_input_scope_contexts[i_index][c_index]->id;
			scope_context.push_back(duplicate->scopes[scope_id]);
		}
		new_branch_node->input_scope_contexts.push_back(scope_context);

		vector<int> node_context_ids;
		for (int c_index = 0; c_index < (int)this->new_input_node_contexts[i_index].size(); c_index++) {
			node_context_ids.push_back(this->new_input_node_contexts[i_index][c_index]->id);
		}
		new_branch_node->input_node_context_ids.push_back(node_context_ids);

		vector<AbstractNode*> node_context;
		for (int c_index = 0; c_index < (int)this->new_input_node_contexts[i_index].size(); c_index++) {
			node_context.push_back(new_branch_node->input_scope_contexts[i_index][c_index]
				->nodes[this->new_input_node_contexts[i_index][c_index]->id]);
		}
		new_branch_node->input_node_contexts.push_back(node_context);

		new_branch_node->input_obs_indexes.push_back(this->new_input_obs_indexes[i_index]);
	}
	new_branch_node->network = this->new_network;
	this->new_network = NULL;

	#if defined(MDEBUG) && MDEBUG
	if (this->verify_problems.size() > 0) {
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;

		new_branch_node->verify_key = this;
		new_branch_node->verify_scores = this->verify_scores;
	}
	#endif /* MDEBUG */

	this->best_seed_actions.clear();
	this->best_seed_scopes.clear();
	this->best_back_actions.clear();
	this->best_back_scopes.clear();
}

void SeedExperiment::new_pass_through(Solution* duplicate) {
	cout << "SeedExperiment new_pass_through" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->best_seed_exit_next_node == NULL) {
		ActionNode* new_ending_node = new ActionNode();
		new_ending_node->parent = duplicate_local_scope;
		new_ending_node->id = duplicate_local_scope->node_counter;
		duplicate_local_scope->node_counter++;
		duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->action = Action(ACTION_NOOP);

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->best_seed_exit_next_node->id;
		exit_node = duplicate_local_scope->nodes[this->best_seed_exit_next_node->id];
	}

	int start_node_id;
	AbstractNode* start_node;
	if (this->best_seed_step_types.size() == 0) {
		start_node_id = this->best_seed_exit_next_node->id;
		start_node = duplicate_local_scope->nodes[this->best_seed_exit_next_node->id];
	} else {
		if (this->best_seed_step_types[0] == STEP_TYPE_ACTION) {
			start_node_id = this->best_seed_actions[0]->id;
			start_node = this->best_seed_actions[0];
		} else {
			start_node_id = this->best_seed_scopes[0]->id;
			start_node = this->best_seed_scopes[0];
		}
	}

	switch (duplicate_explore_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			action_node->next_node_id = start_node_id;
			action_node->next_node = start_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			scope_node->next_node_id = start_node_id;
			scope_node->next_node = start_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				info_branch_node->branch_next_node_id = start_node_id;
				info_branch_node->branch_next_node = start_node;
			} else {
				info_branch_node->original_next_node_id = start_node_id;
				info_branch_node->original_next_node = start_node;
			}
		}
		break;
	}

	for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->best_seed_step_types.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			if (this->best_seed_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->best_seed_actions[s_index+1]->id;
				next_node = this->best_seed_actions[s_index+1];
			} else {
				next_node_id = this->best_seed_scopes[s_index+1]->id;
				next_node = this->best_seed_scopes[s_index+1];
			}
		}

		if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_seed_actions[s_index]->next_node_id = next_node_id;
			this->best_seed_actions[s_index]->next_node = next_node;

			this->best_seed_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_seed_actions[s_index]->id] = this->best_seed_actions[s_index];
		} else {
			this->best_seed_scopes[s_index]->next_node_id = next_node_id;
			this->best_seed_scopes[s_index]->next_node = next_node;

			this->best_seed_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_seed_scopes[s_index]->id] = this->best_seed_scopes[s_index];

			this->best_seed_scopes[s_index]->scope = duplicate->scopes[this->best_seed_scopes[s_index]->scope->id];
		}
	}

	this->best_seed_actions.clear();
	this->best_seed_scopes.clear();
}
