#include "orientation_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "info_branch_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void OrientationExperiment::finalize(Solution* duplicate) {
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

void OrientationExperiment::new_branch(Solution* duplicate) {
	cout << "new_branch" << endl;

	Eval* duplicate_eval = duplicate->scopes[this->eval_context->parent_scope->id]->eval;
	Scope* duplicate_local_scope = duplicate_eval->subscope;

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = duplicate_local_scope;
	new_branch_node->id = duplicate_local_scope->node_counter;
	duplicate_local_scope->node_counter++;
	duplicate_local_scope->nodes[new_branch_node->id] = new_branch_node;

	for (int i_index = 0; i_index < (int)this->existing_input_scope_contexts.size(); i_index++) {
		vector<int> scope_context_ids;
		for (int c_index = 0; c_index < (int)this->existing_input_scope_contexts[i_index].size(); c_index++) {
			scope_context_ids.push_back(this->existing_input_scope_contexts[i_index][c_index]->id);
		}
		new_branch_node->original_input_scope_context_ids.push_back(scope_context_ids);

		vector<Scope*> scope_context;
		for (int c_index = 0; c_index < (int)this->existing_input_scope_contexts[i_index].size(); c_index++) {
			int scope_id = this->existing_input_scope_contexts[i_index][c_index]->id;
			if (scope_id == -1) {
				scope_context.push_back(duplicate_local_scope);
			} else {
				scope_context.push_back(duplicate->scopes[scope_id]);
			}
		}
		new_branch_node->original_input_scope_contexts.push_back(scope_context);

		vector<int> node_context_ids;
		for (int c_index = 0; c_index < (int)this->existing_input_node_contexts[i_index].size(); c_index++) {
			node_context_ids.push_back(this->existing_input_node_contexts[i_index][c_index]->id);
		}
		new_branch_node->original_input_node_context_ids.push_back(node_context_ids);

		vector<AbstractNode*> node_context;
		for (int c_index = 0; c_index < (int)this->existing_input_node_contexts[i_index].size(); c_index++) {
			node_context.push_back(new_branch_node->original_input_scope_contexts[i_index][c_index]
				->nodes[this->existing_input_node_contexts[i_index][c_index]->id]);
		}
		new_branch_node->original_input_node_contexts.push_back(node_context);

		new_branch_node->original_input_obs_indexes.push_back(this->existing_input_obs_indexes[i_index]);
	}
	new_branch_node->original_network = this->existing_network;
	this->existing_network = NULL;

	for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
		vector<int> scope_context_ids;
		for (int c_index = 0; c_index < (int)this->new_input_scope_contexts[i_index].size(); c_index++) {
			scope_context_ids.push_back(this->new_input_scope_contexts[i_index][c_index]->id);
		}
		new_branch_node->branch_input_scope_context_ids.push_back(scope_context_ids);

		vector<Scope*> scope_context;
		for (int c_index = 0; c_index < (int)this->new_input_scope_contexts[i_index].size(); c_index++) {
			int scope_id = this->new_input_scope_contexts[i_index][c_index]->id;
			if (scope_id == -1) {
				scope_context.push_back(duplicate_local_scope);
			} else {
				scope_context.push_back(duplicate->scopes[scope_id]);
			}
		}
		new_branch_node->branch_input_scope_contexts.push_back(scope_context);

		vector<int> node_context_ids;
		for (int c_index = 0; c_index < (int)this->new_input_node_contexts[i_index].size(); c_index++) {
			node_context_ids.push_back(this->new_input_node_contexts[i_index][c_index]->id);
		}
		new_branch_node->branch_input_node_context_ids.push_back(node_context_ids);

		vector<AbstractNode*> node_context;
		for (int c_index = 0; c_index < (int)this->new_input_node_contexts[i_index].size(); c_index++) {
			node_context.push_back(new_branch_node->branch_input_scope_contexts[i_index][c_index]
				->nodes[this->new_input_node_contexts[i_index][c_index]->id]);
		}
		new_branch_node->branch_input_node_contexts.push_back(node_context);

		new_branch_node->branch_input_obs_indexes.push_back(this->new_input_obs_indexes[i_index]);
	}
	new_branch_node->branch_network = this->new_network;
	this->new_network = NULL;

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
					new_branch_node->original_next_node_id = this->ending_node->id;
					new_branch_node->original_next_node = this->ending_node;
				} else {
					ActionNode* new_ending_node = new ActionNode();
					new_ending_node->parent = duplicate_local_scope;
					new_ending_node->id = duplicate_local_scope->node_counter;
					duplicate_local_scope->node_counter++;
					duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->action = Action(ACTION_NOOP);

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					new_branch_node->original_next_node_id = new_ending_node->id;
					new_branch_node->original_next_node = new_ending_node;
				}
			} else {
				new_branch_node->original_next_node_id = action_node->next_node_id;
				new_branch_node->original_next_node = action_node->next_node;
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			new_branch_node->original_next_node_id = scope_node->next_node_id;
			new_branch_node->original_next_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
				new_branch_node->original_next_node = branch_node->branch_next_node;
			} else {
				new_branch_node->original_next_node_id = branch_node->original_next_node_id;
				new_branch_node->original_next_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				new_branch_node->original_next_node_id = info_branch_node->branch_next_node_id;
				new_branch_node->original_next_node = info_branch_node->branch_next_node;
			} else {
				new_branch_node->original_next_node_id = info_branch_node->original_next_node_id;
				new_branch_node->original_next_node = info_branch_node->original_next_node;
			}
		}
		break;
	}

	if (this->step_types.size() == 0) {
		new_branch_node->branch_next_node_id = this->exit_next_node->id;
		new_branch_node->branch_next_node = duplicate_local_scope->nodes[this->exit_next_node->id];
	} else {
		if (this->step_types[0] == STEP_TYPE_ACTION) {
			new_branch_node->branch_next_node_id = this->actions[0]->id;
			new_branch_node->branch_next_node = this->actions[0];
		} else {
			new_branch_node->branch_next_node_id = this->scopes[0]->id;
			new_branch_node->branch_next_node = this->scopes[0];
		}
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

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->actions[s_index]->id] = this->actions[s_index];
		} else {
			this->scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->scopes[s_index]->id] = this->scopes[s_index];

			this->scopes[s_index]->scope = duplicate->scopes[this->scopes[s_index]->scope->id];
		}
	}
	if (this->step_types.size() > 0) {
		if (this->step_types.back() == STEP_TYPE_ACTION) {
			if (this->actions.back()->next_node != NULL) {
				this->actions.back()->next_node = duplicate_local_scope
					->nodes[this->actions.back()->next_node->id];
			}
		} else {
			if (this->scopes.back()->next_node != NULL) {
				this->scopes.back()->next_node = duplicate_local_scope
					->nodes[this->scopes.back()->next_node->id];
			}
		}
	}

	this->actions.clear();
	this->scopes.clear();
	this->ending_node = NULL;

	#if defined(MDEBUG) && MDEBUG
	duplicate->verify_key = this;
	duplicate->verify_problems = this->verify_problems;
	this->verify_problems.clear();
	duplicate->verify_seeds = this->verify_seeds;

	new_branch_node->verify_key = this;
	new_branch_node->verify_original_scores = this->verify_original_scores;
	new_branch_node->verify_branch_scores = this->verify_branch_scores;
	#endif /* MDEBUG */
}

void OrientationExperiment::new_pass_through(Solution* duplicate) {
	cout << "new_pass_through" << endl;

	Eval* duplicate_eval = duplicate->scopes[this->eval_context->parent_scope->id]->eval;
	Scope* duplicate_local_scope = duplicate_eval->subscope;
	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

	int start_node_id;
	AbstractNode* start_node;
	if (this->step_types.size() == 0) {
		start_node_id = this->exit_next_node->id;
		start_node = duplicate_local_scope->nodes[this->exit_next_node->id];
	} else {
		if (this->step_types[0] == STEP_TYPE_ACTION) {
			start_node_id = this->actions[0]->id;
			start_node = this->actions[0];
		} else {
			start_node_id = this->scopes[0]->id;
			start_node = this->scopes[0];
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

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->actions[s_index]->id] = this->actions[s_index];
		} else {
			this->scopes[s_index]->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->scopes[s_index]->id] = this->scopes[s_index];

			this->scopes[s_index]->scope = duplicate->scopes[this->scopes[s_index]->scope->id];
		}
	}
	if (this->step_types.size() > 0) {
		if (this->step_types.back() == STEP_TYPE_ACTION) {
			if (this->actions.back()->next_node != NULL) {
				this->actions.back()->next_node = duplicate_local_scope
					->nodes[this->actions.back()->next_node->id];
			}
		} else {
			if (this->scopes.back()->next_node != NULL) {
				this->scopes.back()->next_node = duplicate_local_scope
					->nodes[this->scopes.back()->next_node->id];
			}
		}
	}

	this->actions.clear();
	this->scopes.clear();
	this->ending_node = NULL;
}
