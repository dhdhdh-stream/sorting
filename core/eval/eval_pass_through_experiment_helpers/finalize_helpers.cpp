#include "eval_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void EvalPassThroughExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		if (this->info_scope == NULL) {
			new_pass_through(duplicate);
		} else {
			new_branch(duplicate);
		}

		duplicate->eval->input_node_contexts.clear();
		duplicate->eval->input_obs_indexes.clear();

		duplicate->eval->linear_input_indexes.clear();
		duplicate->eval->linear_weights.clear();

		duplicate->eval->network_input_indexes.clear();
		if (duplicate->eval->network != NULL) {
			delete duplicate->eval->network;
			duplicate->eval->network = NULL;
		}

		duplicate->eval->average_score = this->new_average_score;

		vector<int> input_mapping(this->input_node_contexts.size(), -1);
		for (int i_index = 0; i_index < (int)this->linear_weights.size(); i_index++) {
			if (this->linear_weights[i_index] != 0.0) {
				if (input_mapping[i_index] == -1) {
					input_mapping[i_index] = (int)duplicate->eval->input_node_contexts.size();
					duplicate->eval->input_node_contexts.push_back(
						duplicate->eval->subscope->nodes[this->input_node_contexts[i_index]->id]);
					duplicate->eval->input_obs_indexes.push_back(this->input_obs_indexes[i_index]);
				}
			}
		}
		for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
			for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
				int original_index = this->network_input_indexes[i_index][v_index];
				if (input_mapping[original_index] == -1) {
					input_mapping[original_index] = (int)duplicate->eval->input_node_contexts.size();
					duplicate->eval->input_node_contexts.push_back(
						duplicate->eval->subscope->nodes[this->input_node_contexts[original_index]->id]);
					duplicate->eval->input_obs_indexes.push_back(this->input_obs_indexes[original_index]);
				}
			}
		}

		for (int i_index = 0; i_index < (int)this->linear_weights.size(); i_index++) {
			if (this->linear_weights[i_index] != 0.0) {
				duplicate->eval->linear_input_indexes.push_back(input_mapping[i_index]);
				duplicate->eval->linear_weights.push_back(this->linear_weights[i_index]);
			}
		}

		for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
			vector<int> input_indexes;
			for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
				input_indexes.push_back(input_mapping[this->network_input_indexes[i_index][v_index]]);
			}
			duplicate->eval->network_input_indexes.push_back(input_indexes);
		}
		duplicate->eval->network = this->network;
		this->network = NULL;
	}

	/**
	 * - set to NULL in original
	 */
	solution->eval->experiment = NULL;

	int experiment_index;
	for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
		if (this->node_context->experiments[e_index] == this) {
			experiment_index = e_index;
		}
	}
	this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
}

void EvalPassThroughExperiment::new_branch(Solution* duplicate) {
	Scope* duplicate_local_scope = duplicate->eval->subscope;

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	InfoBranchNode* new_branch_node = new InfoBranchNode();
	new_branch_node->parent = duplicate_local_scope;
	new_branch_node->id = duplicate_local_scope->node_counter;
	duplicate_local_scope->node_counter++;
	duplicate_local_scope->nodes[new_branch_node->id] = new_branch_node;

	new_branch_node->scope = duplicate->info_scopes[this->info_scope->id];
	new_branch_node->is_negate = this->is_negate;

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
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* info_scope_node = (InfoScopeNode*)duplicate_explore_node;

			new_branch_node->original_next_node_id = info_scope_node->next_node_id;
			new_branch_node->original_next_node = info_scope_node->next_node;
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
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* info_scope_node = (InfoScopeNode*)duplicate_explore_node;

			info_scope_node->next_node_id = new_branch_node->id;
			info_scope_node->next_node = new_branch_node;
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

			this->scopes[s_index]->scope = duplicate->info_scopes[this->scopes[s_index]->scope->id];
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

void EvalPassThroughExperiment::new_pass_through(Solution* duplicate) {
	Scope* duplicate_local_scope = duplicate->eval->subscope;
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
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* info_scope_node = (InfoScopeNode*)duplicate_explore_node;

			info_scope_node->next_node_id = start_node_id;
			info_scope_node->next_node = start_node;
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

			this->scopes[s_index]->scope = duplicate->info_scopes[this->scopes[s_index]->scope->id];
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
