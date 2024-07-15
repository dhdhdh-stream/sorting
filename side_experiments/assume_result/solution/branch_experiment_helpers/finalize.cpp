#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void BranchExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		new_branch(duplicate);
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

void BranchExperiment::new_branch(Solution* duplicate) {
	cout << "new_branch" << endl;

	Scope* duplicate_local_scope = duplicate->scopes[this->scope_context->id];

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	this->branch_node->parent = duplicate_local_scope;
	duplicate_local_scope->nodes[this->branch_node->id] = this->branch_node;

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

					new_ending_node->action = Action(ACTION_NOOP, 0);

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
	case NODE_TYPE_RETURN:
		{
			ReturnNode* return_node = (ReturnNode*)duplicate_explore_node;

			this->branch_node->original_next_node_id = return_node->next_node_id;
			this->branch_node->original_next_node = return_node->next_node;
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
		} else if (this->best_step_types[0] == STEP_TYPE_SCOPE) {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_returns[0]->id;
			this->branch_node->branch_next_node = this->best_returns[0];
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
	case NODE_TYPE_RETURN:
		{
			ReturnNode* return_node = (ReturnNode*)duplicate_explore_node;

			return_node->next_node_id = this->branch_node->id;
			return_node->next_node = this->branch_node;
		}
		break;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
		} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
			this->best_scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];

			this->best_scopes[s_index]->scope = duplicate->scopes[this->best_scopes[s_index]->scope->id];
		} else {
			this->best_returns[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->best_returns[s_index]->id] = this->best_returns[s_index];

			this->best_returns[s_index]->previous_location = duplicate_local_scope->nodes[
				this->best_returns[s_index]->previous_location_id];
		}
	}
	if (this->best_step_types.size() > 0) {
		if (this->best_step_types.back() == STEP_TYPE_ACTION) {
			if (this->best_actions.back()->next_node != NULL) {
				this->best_actions.back()->next_node = duplicate_local_scope
					->nodes[this->best_actions.back()->next_node_id];
			}
		} else if (this->best_step_types.back() == STEP_TYPE_SCOPE) {
			if (this->best_scopes.back()->next_node != NULL) {
				this->best_scopes.back()->next_node = duplicate_local_scope
					->nodes[this->best_scopes.back()->next_node_id];
			}
		} else {
			if (this->best_returns.back()->next_node != NULL) {
				this->best_returns.back()->next_node = duplicate_local_scope
					->nodes[this->best_returns.back()->next_node_id];
			}
		}
	}

	this->branch_node->previous_location_id = this->best_previous_location->id;
	this->branch_node->previous_location = duplicate_local_scope->nodes[this->best_previous_location->id];

	this->branch_node->analyze_size = this->new_analyze_size;
	this->branch_node->network = this->new_network;
	this->new_network = NULL;

	#if defined(MDEBUG) && MDEBUG
	if (this->verify_problems.size() > 0) {
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;

		this->branch_node->verify_key = this;
		this->branch_node->verify_scores = this->verify_scores;
	}
	#endif /* MDEBUG */

	this->best_actions.clear();
	this->best_scopes.clear();
	this->best_returns.clear();
	this->ending_node = NULL;
	this->branch_node = NULL;
}
