#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "exit_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		Scope* duplicate_local_scope = duplicate->scopes[this->scope_context.back()->id];
		AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context.back()->id];

		int start_node_id;
		AbstractNode* start_node;
		if (this->scope_context.size() > 1) {
			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->parent = duplicate_local_scope;
			new_branch_node->id = duplicate_local_scope->node_counter;
			duplicate_local_scope->node_counter++;
			duplicate_local_scope->nodes[new_branch_node->id] = new_branch_node;

			new_branch_node->scope_context = this->scope_context;
			for (int c_index = 0; c_index < (int)new_branch_node->scope_context.size(); c_index++) {
				new_branch_node->scope_context_ids.push_back(new_branch_node->scope_context[c_index]->id);
			}
			for (int c_index = 0; c_index < (int)new_branch_node->scope_context.size(); c_index++) {
				new_branch_node->scope_context[c_index] = duplicate->scopes[new_branch_node->scope_context[c_index]->id];
			}
			new_branch_node->node_context = this->node_context;
			new_branch_node->node_context.back() = new_branch_node;
			for (int c_index = 0; c_index < (int)new_branch_node->node_context.size(); c_index++) {
				new_branch_node->node_context_ids.push_back(new_branch_node->node_context[c_index]->id);
			}
			for (int c_index = 0; c_index < (int)new_branch_node->node_context.size()-1; c_index++) {
				new_branch_node->node_context[c_index] = new_branch_node->scope_context[c_index]
					->nodes[new_branch_node->node_context[c_index]->id];
			}

			new_branch_node->is_pass_through = true;

			new_branch_node->original_average_score = 0.0;
			new_branch_node->branch_average_score = 0.0;

			new_branch_node->input_max_depth = 0;

			new_branch_node->original_network = NULL;
			new_branch_node->branch_network = NULL;

			if (duplicate_explore_node->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)duplicate_explore_node;

				new_branch_node->original_next_node_id = action_node->next_node_id;
				new_branch_node->original_next_node = action_node->next_node;
			} else if (duplicate_explore_node->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

				new_branch_node->original_next_node_id = scope_node->next_node_id;
				new_branch_node->original_next_node = scope_node->next_node;
			} else {
				BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

				if (this->is_branch) {
					new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
					new_branch_node->original_next_node = branch_node->branch_next_node;
				} else {
					new_branch_node->original_next_node_id = branch_node->original_next_node_id;
					new_branch_node->original_next_node = branch_node->original_next_node;
				}
			}

			if (this->best_step_types.size() == 0) {
				if (this->exit_node != NULL) {
					new_branch_node->branch_next_node_id = this->exit_node->id;
					new_branch_node->branch_next_node = this->exit_node;
				} else {
					new_branch_node->branch_next_node_id = this->best_exit_next_node->id;
					new_branch_node->branch_next_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
				}
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					new_branch_node->branch_next_node_id = this->best_actions[0]->id;
					new_branch_node->branch_next_node = this->best_actions[0];
				} else {
					new_branch_node->branch_next_node_id = this->best_scopes[0]->id;
					new_branch_node->branch_next_node = this->best_scopes[0];
				}
			}

			start_node_id = new_branch_node->id;
			start_node = new_branch_node;
		} else {
			if (this->best_step_types.size() == 0) {
				if (this->exit_node != NULL) {
					start_node_id = this->exit_node->id;
					start_node = this->exit_node;
				} else {
					start_node_id = this->best_exit_next_node->id;
					start_node = duplicate_local_scope->nodes[this->best_exit_next_node->id];
				}
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					start_node_id = this->best_actions[0]->id;
					start_node = this->best_actions[0];
				} else {
					start_node_id = this->best_scopes[0]->id;
					start_node = this->best_scopes[0];
				}
			}
		}

		if (duplicate_explore_node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)duplicate_explore_node;

			action_node->next_node_id = start_node_id;
			action_node->next_node = start_node;
		} else if (duplicate_explore_node->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

			scope_node->next_node_id = start_node_id;
			scope_node->next_node = start_node;
		} else {
			BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

			if (this->is_branch) {
				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}
		}

		if (this->exit_node != NULL) {
			this->exit_node->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->exit_node->id] = this->exit_node;

			this->exit_node->next_node_parent = duplicate->scopes[this->exit_node->next_node_parent->id];
			if (this->exit_node->next_node != NULL) {
				this->exit_node->next_node = this->exit_node->next_node_parent->nodes[
					this->exit_node->next_node->id];
			}
		}
		if (this->ending_node != NULL) {
			this->ending_node->parent = duplicate_local_scope;
			duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;

			duplicate_local_scope->current_ending_node_id = this->ending_node->id;
			duplicate_local_scope->current_ending_node = this->ending_node;
		}

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_actions[s_index]->parent = duplicate_local_scope;
				duplicate_local_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
			} else {
				this->best_scopes[s_index]->parent = duplicate_local_scope;
				duplicate_local_scope->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];

				Scope* duplicate_scope = duplicate->scopes[this->best_scopes[s_index]->scope->id];

				this->best_scopes[s_index]->scope = duplicate_scope;
				this->best_scopes[s_index]->starting_node = duplicate_scope->nodes[
					this->best_scopes[s_index]->starting_node->id];
				set<AbstractNode*> duplicate_exit_nodes;
				for (set<AbstractNode*>::iterator it = this->best_scopes[s_index]->exit_nodes.begin();
						it != this->best_scopes[s_index]->exit_nodes.end(); it++) {
					duplicate_exit_nodes.insert(duplicate_scope->nodes[(*it)->id]);
				}
				this->best_scopes[s_index]->exit_nodes = duplicate_exit_nodes;

				duplicate_scope->subscopes.insert(
					{this->best_scopes[s_index]->starting_node_id,
						this->best_scopes[s_index]->exit_node_ids});
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
		this->exit_node = NULL;

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
	for (int e_index = 0; e_index < (int)this->node_context.back()->experiments.size(); e_index++) {
		if (this->node_context.back()->experiments[e_index] == this) {
			experiment_index = e_index;
		}
	}
	this->node_context.back()->experiments.erase(this->node_context.back()->experiments.begin() + experiment_index);
}
