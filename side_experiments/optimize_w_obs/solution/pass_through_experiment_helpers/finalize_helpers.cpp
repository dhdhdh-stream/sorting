#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "condition_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		new_pass_through(duplicate);
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

void PassThroughExperiment::new_pass_through(Solution* duplicate) {
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
	case NODE_TYPE_CONDITION:
		{
			ConditionNode* condition_node = (ConditionNode*)duplicate_explore_node;

			if (this->is_branch) {
				condition_node->branch_next_node_id = start_node_id;
				condition_node->branch_next_node = start_node;
			} else {
				condition_node->original_next_node_id = start_node_id;
				condition_node->original_next_node = start_node;
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

	if (this->conditions.size() > 0) {
		ConditionNode* new_condition_node = new ConditionNode();
		new_condition_node->parent = duplicate_local_scope;
		new_condition_node->id = duplicate_local_scope->node_counter;
		duplicate_local_scope->node_counter++;
		duplicate_local_scope->nodes[new_condition_node->id] = new_condition_node;

		new_condition_node->conditions = this->conditions;

		switch (duplicate_explore_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_explore_node;

				if (action_node->next_node == NULL) {
					/**
					 * - ending node
					 */
					if (this->ending_node != NULL) {
						new_condition_node->original_next_node_id = this->ending_node->id;
						new_condition_node->original_next_node = this->ending_node;
					} else {
						ActionNode* new_ending_node = new ActionNode();
						new_ending_node->parent = duplicate_local_scope;
						new_ending_node->id = duplicate_local_scope->node_counter;
						duplicate_local_scope->node_counter++;
						duplicate_local_scope->nodes[new_ending_node->id] = new_ending_node;

						new_ending_node->action = Action(ACTION_NOOP);

						new_ending_node->next_node_id = -1;
						new_ending_node->next_node = NULL;

						new_condition_node->original_next_node_id = new_ending_node->id;
						new_condition_node->original_next_node = new_ending_node;
					}
				} else {
					new_condition_node->original_next_node_id = action_node->next_node_id;
					new_condition_node->original_next_node = action_node->next_node;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

				new_condition_node->original_next_node_id = scope_node->next_node_id;
				new_condition_node->original_next_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

				if (this->is_branch) {
					new_condition_node->original_next_node_id = branch_node->branch_next_node_id;
					new_condition_node->original_next_node = branch_node->branch_next_node;
				} else {
					new_condition_node->original_next_node_id = branch_node->original_next_node_id;
					new_condition_node->original_next_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_CONDITION:
			{
				ConditionNode* condition_node = (ConditionNode*)duplicate_explore_node;

				if (this->is_branch) {
					new_condition_node->original_next_node_id = condition_node->branch_next_node_id;
					new_condition_node->original_next_node = condition_node->branch_next_node;
				} else {
					new_condition_node->original_next_node_id = condition_node->original_next_node_id;
					new_condition_node->original_next_node = condition_node->original_next_node;
				}
			}
			break;
		}

		new_condition_node->branch_next_node_id = start_node_id;
		new_condition_node->branch_next_node = start_node;

		for (int c_index = 0; c_index < (int)this->conditions.size(); c_index++) {
			Scope* scope = duplicate->scopes[this->conditions[c_index].first.first.back()];
			AbstractNode* node = scope->nodes[this->conditions[c_index].first.second.back()];
			switch (node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* input_action_node = (ActionNode*)node;

					bool is_existing = false;
					for (int ii_index = 0; ii_index < (int)input_action_node->input_scope_context_ids.size(); ii_index++) {
						if (input_action_node->input_scope_context_ids[ii_index] == this->conditions[c_index].first.first
								&& input_action_node->input_node_context_ids[ii_index] == this->conditions[c_index].first.second
								&& input_action_node->input_obs_indexes[ii_index] == this->conditions[c_index].second) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						input_action_node->input_scope_context_ids.push_back(this->conditions[c_index].first.first);
						input_action_node->input_node_context_ids.push_back(this->conditions[c_index].first.second);
						input_action_node->input_obs_indexes.push_back(this->conditions[c_index].second);
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* input_branch_node = (BranchNode*)node;

					bool is_existing = false;
					for (int ii_index = 0; ii_index < (int)input_branch_node->input_scope_context_ids.size(); ii_index++) {
						if (input_branch_node->input_scope_context_ids[ii_index] == this->conditions[c_index].first.first
								&& input_branch_node->input_node_context_ids[ii_index] == this->conditions[c_index].first.second) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						input_branch_node->input_scope_context_ids.push_back(this->conditions[c_index].first.first);
						input_branch_node->input_node_context_ids.push_back(this->conditions[c_index].first.second);
					}
				}
				break;
			}
		}

		switch (duplicate_explore_node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)duplicate_explore_node;

				action_node->next_node_id = new_condition_node->id;
				action_node->next_node = new_condition_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

				scope_node->next_node_id = new_condition_node->id;
				scope_node->next_node = new_condition_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

				if (this->is_branch) {
					branch_node->branch_next_node_id = new_condition_node->id;
					branch_node->branch_next_node = new_condition_node;
				} else {
					branch_node->original_next_node_id = new_condition_node->id;
					branch_node->original_next_node = new_condition_node;
				}
			}
			break;
		case NODE_TYPE_CONDITION:
			{
				ConditionNode* condition_node = (ConditionNode*)duplicate_explore_node;

				if (this->is_branch) {
					condition_node->branch_next_node_id = new_condition_node->id;
					condition_node->branch_next_node = new_condition_node;
				} else {
					condition_node->original_next_node_id = new_condition_node->id;
					condition_node->original_next_node = new_condition_node;
				}
			}
			break;
		}
	}

	this->best_actions.clear();
	this->best_scopes.clear();
	this->ending_node = NULL;
}
