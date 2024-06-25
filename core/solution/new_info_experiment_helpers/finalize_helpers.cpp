#include "new_info_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void NewInfoExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		/**
		 * - for if child experiment
		 */
		switch (this->best_pre_exit_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->best_pre_exit_node;
				this->best_exit_next_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->best_pre_exit_node;
				this->best_exit_next_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* branch_end_node = (BranchEndNode*)this->best_pre_exit_node;
				this->best_exit_next_node = branch_end_node->next_node;
			}
			break;
		}

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

	BranchEndNode* new_ending_node = new BranchEndNode();
	new_ending_node->parent = duplicate_local_scope;
	new_ending_node->id = duplicate_local_scope->node_counter;
	duplicate_local_scope->node_counter++;
	duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

	this->new_info_scope->id = duplicate->info_scopes.size();
	duplicate->info_scopes.push_back(new_info_scope);

	duplicate_local_scope->info_scopes_used.insert(duplicate->info_scopes[this->new_info_scope->id]);

	new_info_scope->input_node_contexts = this->new_input_node_contexts;
	new_info_scope->input_obs_indexes = this->new_input_obs_indexes;
	new_info_scope->network = this->new_network;
	this->new_network = NULL;

	this->branch_node->parent = duplicate_local_scope;
	duplicate_local_scope->nodes[this->branch_node->id] = this->branch_node;

	this->branch_node->scope = new_info_scope;
	this->branch_node->is_negate = false;

	this->branch_node->branch_end_node_id = new_ending_node->id;
	this->branch_node->branch_end_node = new_ending_node;

	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			if (action_node->next_node == this->best_exit_next_node) {
				this->branch_node->original_next_node_id = new_ending_node->id;
				this->branch_node->original_next_node = new_ending_node;
			} else {
				this->branch_node->original_next_node_id = action_node->next_node_id;
				this->branch_node->original_next_node = duplicate_local_scope->nodes[action_node->next_node_id];
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			if (scope_node->next_node == this->best_exit_next_node) {
				this->branch_node->original_next_node_id = new_ending_node->id;
				this->branch_node->original_next_node = new_ending_node;
			} else {
				this->branch_node->original_next_node_id = scope_node->next_node_id;
				this->branch_node->original_next_node = duplicate_local_scope->nodes[scope_node->next_node_id];
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				if (branch_node->branch_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = branch_node->branch_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[branch_node->branch_next_node_id];
				}
			} else {
				if (branch_node->original_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = branch_node->original_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[branch_node->original_next_node_id];
				}
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;

			if (this->is_branch) {
				if (info_branch_node->branch_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = info_branch_node->branch_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[info_branch_node->branch_next_node_id];
				}
			} else {
				if (info_branch_node->original_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = info_branch_node->original_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[info_branch_node->original_next_node_id];
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;

			if (branch_end_node->next_node == this->best_exit_next_node) {
				this->branch_node->original_next_node_id = new_ending_node->id;
				this->branch_node->original_next_node = new_ending_node;
			} else {
				this->branch_node->original_next_node_id = branch_end_node->next_node_id;
				this->branch_node->original_next_node = duplicate_local_scope->nodes[branch_end_node->next_node_id];
			}
		}
		break;
	}

	if (this->best_step_types.size() == 0) {
		this->branch_node->branch_next_node_id = new_ending_node->id;
		this->branch_node->branch_next_node = new_ending_node;
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->best_actions[0]->id;
			this->branch_node->branch_next_node = this->best_actions[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		}
	}

	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];
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
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)duplicate_explore_node;

			branch_end_node->next_node_id = this->branch_node->id;
			branch_end_node->next_node = this->branch_node;
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

			duplicate_local_scope->scopes_used.insert(duplicate->scopes[this->best_scopes[s_index]->scope->id]);
		}
	}
	if (this->best_step_types.size() > 0) {
		if (this->best_step_types.back() == STEP_TYPE_ACTION) {
			this->best_actions.back()->next_node_id = new_ending_node->id;
			this->best_actions.back()->next_node = new_ending_node;
		} else {
			this->best_scopes.back()->next_node_id = new_ending_node->id;
			this->best_scopes.back()->next_node = new_ending_node;
		}
	}

	if (this->node_context != this->best_pre_exit_node) {
		AbstractNode* duplicate_pre_exit_node = duplicate_local_scope->nodes[this->best_pre_exit_node->id];
		switch (duplicate_pre_exit_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_pre_exit_node;
				action_node->next_node_id = new_ending_node->id;
				action_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_pre_exit_node;
				scope_node->next_node_id = new_ending_node->id;
				scope_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* branch_end_node = (BranchEndNode*)duplicate_pre_exit_node;
				branch_end_node->next_node_id = new_ending_node->id;
				branch_end_node->next_node = new_ending_node;
			}
			break;
		}
		/**
		 * - can be NODE_TYPE_BRANCH, NODE_TYPE_INFO_BRANCH only if node_context == best_pre_exit_node
		 */
	}

	new_ending_node->branch_start_node_id = this->branch_node->id;
	new_ending_node->branch_start_node = this->branch_node;

	if (this->best_exit_next_node == NULL) {
		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;
	} else {
		new_ending_node->next_node_id = this->best_exit_next_node->id;
		new_ending_node->next_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
	}

	#if defined(MDEBUG) && MDEBUG
	if (this->verify_problems.size() > 0) {
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;

		new_info_scope->verify_key = this;
		new_info_scope->verify_scores = this->verify_scores;
	}
	#endif /* MDEBUG */

	this->new_info_scope = NULL;
	this->best_actions.clear();
	this->best_scopes.clear();
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
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)duplicate_explore_node;

			branch_end_node->next_node_id = start_node_id;
			branch_end_node->next_node = start_node;
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

			duplicate_local_scope->scopes_used.insert(duplicate->scopes[this->best_scopes[s_index]->scope->id]);
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
}

void NewInfoExperiment::new_existing(Solution* duplicate) {
	cout << "new_existing" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];

	BranchEndNode* new_ending_node = new BranchEndNode();
	new_ending_node->parent = duplicate_local_scope;
	new_ending_node->id = duplicate_local_scope->node_counter;
	duplicate_local_scope->node_counter++;
	duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

	this->branch_node->parent = duplicate_local_scope;
	duplicate_local_scope->nodes[this->branch_node->id] = this->branch_node;

	this->branch_node->scope = duplicate->info_scopes[this->existing_info_scope_index];
	this->branch_node->is_negate = this->existing_is_negate;

	this->branch_node->branch_end_node_id = new_ending_node->id;
	this->branch_node->branch_end_node = new_ending_node;

	duplicate_local_scope->info_scopes_used.insert(duplicate->info_scopes[this->existing_info_scope_index]);

	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			if (action_node->next_node == this->best_exit_next_node) {
				this->branch_node->original_next_node_id = new_ending_node->id;
				this->branch_node->original_next_node = new_ending_node;
			} else {
				this->branch_node->original_next_node_id = action_node->next_node_id;
				this->branch_node->original_next_node = duplicate_local_scope->nodes[action_node->next_node_id];
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			if (scope_node->next_node == this->best_exit_next_node) {
				this->branch_node->original_next_node_id = new_ending_node->id;
				this->branch_node->original_next_node = new_ending_node;
			} else {
				this->branch_node->original_next_node_id = scope_node->next_node_id;
				this->branch_node->original_next_node = duplicate_local_scope->nodes[scope_node->next_node_id];
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				if (branch_node->branch_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = branch_node->branch_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[branch_node->branch_next_node_id];
				}
			} else {
				if (branch_node->original_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = branch_node->original_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[branch_node->original_next_node_id];
				}
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;

			if (this->is_branch) {
				if (info_branch_node->branch_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = info_branch_node->branch_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[info_branch_node->branch_next_node_id];
				}
			} else {
				if (info_branch_node->original_next_node == this->best_exit_next_node) {
					this->branch_node->original_next_node_id = new_ending_node->id;
					this->branch_node->original_next_node = new_ending_node;
				} else {
					this->branch_node->original_next_node_id = info_branch_node->original_next_node_id;
					this->branch_node->original_next_node = duplicate_local_scope->nodes[info_branch_node->original_next_node_id];
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;

			if (branch_end_node->next_node == this->best_exit_next_node) {
				this->branch_node->original_next_node_id = new_ending_node->id;
				this->branch_node->original_next_node = new_ending_node;
			} else {
				this->branch_node->original_next_node_id = branch_end_node->next_node_id;
				this->branch_node->original_next_node = duplicate_local_scope->nodes[branch_end_node->next_node_id];
			}
		}
		break;
	}

	if (this->best_step_types.size() == 0) {
		this->branch_node->branch_next_node_id = new_ending_node->id;
		this->branch_node->branch_next_node = new_ending_node;
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->best_actions[0]->id;
			this->branch_node->branch_next_node = this->best_actions[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		}
	}

	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];
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
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)duplicate_explore_node;

			branch_end_node->next_node_id = this->branch_node->id;
			branch_end_node->next_node = this->branch_node;
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

			duplicate_local_scope->scopes_used.insert(duplicate->scopes[this->best_scopes[s_index]->scope->id]);
		}
	}
	if (this->best_step_types.size() > 0) {
		if (this->best_step_types.back() == STEP_TYPE_ACTION) {
			this->best_actions.back()->next_node_id = new_ending_node->id;
			this->best_actions.back()->next_node = new_ending_node;
		} else {
			this->best_scopes.back()->next_node_id = new_ending_node->id;
			this->best_scopes.back()->next_node = new_ending_node;
		}
	}

	if (this->node_context != this->best_pre_exit_node) {
		AbstractNode* duplicate_pre_exit_node = duplicate_local_scope->nodes[this->best_pre_exit_node->id];
		switch (duplicate_pre_exit_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_pre_exit_node;
				action_node->next_node_id = new_ending_node->id;
				action_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_pre_exit_node;
				scope_node->next_node_id = new_ending_node->id;
				scope_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_BRANCH_END:
			{
				BranchEndNode* branch_end_node = (BranchEndNode*)duplicate_pre_exit_node;
				branch_end_node->next_node_id = new_ending_node->id;
				branch_end_node->next_node = new_ending_node;
			}
			break;
		}
		/**
		 * - can be NODE_TYPE_BRANCH, NODE_TYPE_INFO_BRANCH only if node_context == best_pre_exit_node
		 */
	}

	new_ending_node->branch_start_node_id = this->branch_node->id;
	new_ending_node->branch_start_node = this->branch_node;

	if (this->best_exit_next_node == NULL) {
		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;
	} else {
		new_ending_node->next_node_id = this->best_exit_next_node->id;
		new_ending_node->next_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
	}

	this->best_actions.clear();
	this->best_scopes.clear();
	this->branch_node = NULL;
}
