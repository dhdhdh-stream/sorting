#include "new_info_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void NewInfoExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		if (this->is_pass_through) {
			new_pass_through(duplicate);
		} else if (this->use_existing) {
			new_existing(duplicate);
		} else {
			new_branch(duplicate);
		}

		for (int v_index = 0; v_index < (int)this->verify_experiments.size(); v_index++) {
			int index;
			for (int c_index = 0; c_index < (int)this->verify_experiments[v_index]->parent_experiment->child_experiments.size(); c_index++) {
				if (this->verify_experiments[v_index]->parent_experiment->child_experiments[c_index] == this->verify_experiments[v_index]) {
					index = c_index;
					break;
				}
			}
			this->verify_experiments[v_index]->parent_experiment->child_experiments.erase(
				this->verify_experiments[v_index]->parent_experiment->child_experiments.begin() + index);
		}
		for (int v_index = 0; v_index < (int)this->verify_experiments.size(); v_index++) {
			this->verify_experiments[v_index]->result = EXPERIMENT_RESULT_SUCCESS;
			this->verify_experiments[v_index]->finalize(duplicate);
			delete this->verify_experiments[v_index];
		}
	}

	for (int c_index = 0; c_index < (int)this->child_experiments.size(); c_index++) {
		this->child_experiments[c_index]->result = EXPERIMENT_RESULT_FAIL;
		this->child_experiments[c_index]->finalize(duplicate);
		delete this->child_experiments[c_index];
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

void NewInfoExperiment::new_branch(Solution* duplicate) {
	cout << "new_branch" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	this->new_info_scope->id = duplicate->info_scopes.size();
	duplicate->info_scopes.push_back(new_info_scope);

	for (map<int, AbstractNode*>::iterator it = this->new_info_scope->nodes.begin();
			it != this->new_info_scope->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_INFO_SCOPE) {
			InfoScopeNode* info_scope_node = (InfoScopeNode*)it->second;
			info_scope_node->scope = duplicate->info_scopes[info_scope_node->scope->id];
		}
	}

	new_info_scope->input_node_contexts = this->new_input_node_contexts;
	new_info_scope->input_obs_indexes = this->new_input_obs_indexes;
	new_info_scope->network = this->new_network;
	this->new_network = NULL;

	this->branch_node->parent = duplicate_local_scope;
	duplicate_local_scope->nodes[this->branch_node->id] = this->branch_node;

	this->branch_node->scope = new_info_scope;
	this->branch_node->is_negate = false;

	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];
	switch (duplicate_explore_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			if (action_node->next_node == NULL) {
				/**
				 * - ending node
				 */
				if (this->ending_node != NULL) {
					this->branch_node->original_next_node_id = this->ending_node->id;
					this->branch_node->original_next_node = this->ending_node;
				} else {
					ActionNode* new_ending_node = new ActionNode();
					new_ending_node->parent = duplicate_local_scope;
					new_ending_node->id = duplicate_local_scope->node_counter;
					duplicate_local_scope->node_counter++;
					duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->action = Action(ACTION_NOOP);

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				}
			} else {
				this->branch_node->original_next_node_id = action_node->next_node_id;
				this->branch_node->original_next_node = action_node->next_node;
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			this->branch_node->original_next_node_id = scope_node->next_node_id;
			this->branch_node->original_next_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				this->branch_node->original_next_node_id = branch_node->branch_next_node_id;
				this->branch_node->original_next_node = branch_node->branch_next_node;
			} else {
				this->branch_node->original_next_node_id = branch_node->original_next_node_id;
				this->branch_node->original_next_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				this->branch_node->original_next_node_id = info_branch_node->branch_next_node_id;
				this->branch_node->original_next_node = info_branch_node->branch_next_node;
			} else {
				this->branch_node->original_next_node_id = info_branch_node->original_next_node_id;
				this->branch_node->original_next_node = info_branch_node->original_next_node;
			}
		}
		break;
	}

	if (this->best_step_types.size() == 0) {
		this->branch_node->branch_next_node_id = this->best_exit_next_node->id;
		this->branch_node->branch_next_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->best_actions[0]->id;
			this->branch_node->branch_next_node = this->best_actions[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		}
	}

	switch (duplicate_explore_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			action_node->next_node_id = this->branch_node->id;
			action_node->next_node = this->branch_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			scope_node->next_node_id = this->branch_node->id;
			scope_node->next_node = this->branch_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				branch_node->branch_next_node_id = this->branch_node->id;
				branch_node->branch_next_node = this->branch_node;
			} else {
				branch_node->original_next_node_id = this->branch_node->id;
				branch_node->original_next_node = this->branch_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				info_branch_node->branch_next_node_id = this->branch_node->id;
				info_branch_node->branch_next_node = this->branch_node;
			} else {
				info_branch_node->original_next_node_id = this->branch_node->id;
				info_branch_node->original_next_node = this->branch_node;
			}
		}
		break;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
		} else {
			this->best_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];

			this->best_scopes[s_index]->scope = duplicate->scopes[this->best_scopes[s_index]->scope->id];

			duplicate_local_scope->scopes_used.insert(this->best_scopes[s_index]->scope->id);
		}
	}
	if (this->best_step_types.size() > 0) {
		if (this->best_step_types.back() == STEP_TYPE_ACTION) {
			if (this->best_actions.back()->next_node != NULL) {
				this->best_actions.back()->next_node = duplicate_local_scope
					->nodes[this->best_actions.back()->next_node->id];
			}
		} else {
			if (this->best_scopes.back()->next_node != NULL) {
				this->best_scopes.back()->next_node = duplicate_local_scope
					->nodes[this->best_scopes.back()->next_node->id];
			}
		}
	}

	#if defined(MDEBUG) && MDEBUG
	duplicate->verify_problems = this->verify_problems;
	this->verify_problems.clear();
	duplicate->verify_seeds = this->verify_seeds;

	new_info_scope->verify_key = this;
	new_info_scope->verify_scores = this->verify_scores;
	#endif /* MDEBUG */

	this->new_info_scope = NULL;
	this->best_actions.clear();
	this->best_scopes.clear();
	this->ending_node = NULL;
	this->branch_node = NULL;
}

void NewInfoExperiment::new_pass_through(Solution* duplicate) {
	cout << "new_pass_through" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];
	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

	int start_node_id;
	AbstractNode* start_node;
	if (this->best_step_types.size() == 0) {
		start_node_id = this->best_exit_next_node->id;
		start_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			start_node_id = this->best_actions[0]->id;
			start_node = this->best_actions[0];
		} else {
			start_node_id = this->best_scopes[0]->id;
			start_node = this->best_scopes[0];
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

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
		} else {
			this->best_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];

			this->best_scopes[s_index]->scope = duplicate->scopes[this->best_scopes[s_index]->scope->id];

			duplicate_local_scope->scopes_used.insert(this->best_scopes[s_index]->scope->id);
		}
	}
	if (this->best_step_types.size() > 0) {
		if (this->best_step_types.back() == STEP_TYPE_ACTION) {
			if (this->best_actions.back()->next_node != NULL) {
				this->best_actions.back()->next_node = duplicate_local_scope
					->nodes[this->best_actions.back()->next_node->id];
			}
		} else {
			if (this->best_scopes.back()->next_node != NULL) {
				this->best_scopes.back()->next_node = duplicate_local_scope
					->nodes[this->best_scopes.back()->next_node->id];
			}
		}
	}

	this->best_actions.clear();
	this->best_scopes.clear();
	this->ending_node = NULL;
}

void NewInfoExperiment::new_existing(Solution* duplicate) {
	cout << "new_existing" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	this->branch_node->parent = duplicate_local_scope;
	duplicate_local_scope->nodes[this->branch_node->id] = this->branch_node;

	this->branch_node->scope = duplicate->info_scopes[this->existing_info_scope_index];
	this->branch_node->is_negate = this->existing_is_negate;

	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];
	switch (duplicate_explore_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			if (action_node->next_node == NULL) {
				/**
				 * - ending node
				 */
				if (this->ending_node != NULL) {
					this->branch_node->original_next_node_id = this->ending_node->id;
					this->branch_node->original_next_node = this->ending_node;
				} else {
					ActionNode* new_ending_node = new ActionNode();
					new_ending_node->parent = duplicate_local_scope;
					new_ending_node->id = duplicate_local_scope->node_counter;
					duplicate_local_scope->node_counter++;
					duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->action = Action(ACTION_NOOP);

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				}
			} else {
				this->branch_node->original_next_node_id = action_node->next_node_id;
				this->branch_node->original_next_node = action_node->next_node;
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			this->branch_node->original_next_node_id = scope_node->next_node_id;
			this->branch_node->original_next_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				this->branch_node->original_next_node_id = branch_node->branch_next_node_id;
				this->branch_node->original_next_node = branch_node->branch_next_node;
			} else {
				this->branch_node->original_next_node_id = branch_node->original_next_node_id;
				this->branch_node->original_next_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				this->branch_node->original_next_node_id = info_branch_node->branch_next_node_id;
				this->branch_node->original_next_node = info_branch_node->branch_next_node;
			} else {
				this->branch_node->original_next_node_id = info_branch_node->original_next_node_id;
				this->branch_node->original_next_node = info_branch_node->original_next_node;
			}
		}
		break;
	}

	if (this->best_step_types.size() == 0) {
		this->branch_node->branch_next_node_id = this->best_exit_next_node->id;
		this->branch_node->branch_next_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->best_actions[0]->id;
			this->branch_node->branch_next_node = this->best_actions[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		}
	}

	switch (duplicate_explore_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			action_node->next_node_id = this->branch_node->id;
			action_node->next_node = this->branch_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			scope_node->next_node_id = this->branch_node->id;
			scope_node->next_node = this->branch_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				branch_node->branch_next_node_id = this->branch_node->id;
				branch_node->branch_next_node = this->branch_node;
			} else {
				branch_node->original_next_node_id = this->branch_node->id;
				branch_node->original_next_node = this->branch_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				info_branch_node->branch_next_node_id = this->branch_node->id;
				info_branch_node->branch_next_node = this->branch_node;
			} else {
				info_branch_node->original_next_node_id = this->branch_node->id;
				info_branch_node->original_next_node = this->branch_node;
			}
		}
		break;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
		} else {
			this->best_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];

			this->best_scopes[s_index]->scope = duplicate->scopes[this->best_scopes[s_index]->scope->id];

			duplicate_local_scope->scopes_used.insert(this->best_scopes[s_index]->scope->id);
		}
	}
	if (this->best_step_types.size() > 0) {
		if (this->best_step_types.back() == STEP_TYPE_ACTION) {
			if (this->best_actions.back()->next_node != NULL) {
				this->best_actions.back()->next_node = duplicate_local_scope
					->nodes[this->best_actions.back()->next_node->id];
			}
		} else {
			if (this->best_scopes.back()->next_node != NULL) {
				this->best_scopes.back()->next_node = duplicate_local_scope
					->nodes[this->best_scopes.back()->next_node->id];
			}
		}
	}

	this->best_actions.clear();
	this->best_scopes.clear();
	this->ending_node = NULL;
	this->branch_node = NULL;
}
