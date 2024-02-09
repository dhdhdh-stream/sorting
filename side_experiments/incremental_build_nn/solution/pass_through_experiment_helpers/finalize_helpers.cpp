#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::finalize() {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		Scope* containing_scope = solution->scopes[this->scope_context.back()];

		int exit_node_id;
		AbstractNode* exit_node;
		if (this->best_exit_depth > 0) {
			ExitNode* new_exit_node = new ExitNode();
			new_exit_node->parent = containing_scope;
			new_exit_node->id = containing_scope->node_counter;
			containing_scope->node_counter++;
			containing_scope->nodes[new_exit_node->id] = new_exit_node;

			new_exit_node->exit_depth = this->best_exit_depth;
			new_exit_node->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
			if (this->best_exit_node == NULL) {
				new_exit_node->exit_node_id = -1;
			} else {
				new_exit_node->exit_node_id = this->best_exit_node->id;
			}
			new_exit_node->exit_node = this->best_exit_node;

			exit_node_id = new_exit_node->id;
			exit_node = new_exit_node;
		} else {
			if (this->best_exit_node == NULL) {
				exit_node_id = -1;
			} else {
				exit_node_id = this->best_exit_node->id;
			}
			exit_node = this->best_exit_node;
		}

		int start_node_id;
		AbstractNode* start_node;
		if (this->best_exit_depth > 0) {
			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->parent = containing_scope;
			new_branch_node->id = containing_scope->node_counter;
			containing_scope->node_counter++;
			containing_scope->nodes[new_branch_node->id] = new_branch_node;

			new_branch_node->scope_context = this->scope_context;
			for (int c_index = 0; c_index < (int)new_branch_node->scope_context.size(); c_index++) {
				new_branch_node->scope_context_ids.push_back(new_branch_node->scope_context[c_index]->id);
			}
			new_branch_node->node_context = this->node_context;
			new_branch_node->node_context.back() = new_branch_node;
			for (int c_index = 0; c_index < (int)new_branch_node->node_context.size(); c_index++) {
				new_branch_node->node_context_ids.push_back(new_branch_node->node_context[c_index]->id);
			}

			new_branch_node->is_pass_through = true;

			new_branch_node->original_average_score = 0.0;
			new_branch_node->branch_average_score = 0.0;

			new_branch_node->original_network = NULL;
			new_branch_node->branch_network = NULL;

			if (this->node_context.back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->node_context.back();

				new_branch_node->original_next_node_id = action_node->next_node_id;
				new_branch_node->original_next_node = action_node->next_node;
			} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

				new_branch_node->original_next_node_id = scope_node->next_node_id;
				new_branch_node->original_next_node = scope_node->next_node;
			} else {
				BranchNode* branch_node = (BranchNode*)this->node_context.back();

				if (branch_node->experiment_is_branch) {
					new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
					new_branch_node->original_next_node = branch_node->branch_next_node;
				} else {
					new_branch_node->original_next_node_id = branch_node->original_next_node_id;
					new_branch_node->original_next_node = branch_node->original_next_node;
				}
			}

			if (this->best_step_types.size() == 0) {
				new_branch_node->branch_next_node_id = exit_node_id;
				new_branch_node->branch_next_node = exit_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					new_branch_node->branch_next_node_id = this->best_actions[0]->id;
					new_branch_node->branch_next_node = this->best_actions[0];
				} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
					new_branch_node->branch_next_node_id = this->best_existing_scopes[0]->id;
					new_branch_node->branch_next_node = this->best_existing_scopes[0];
				} else {
					new_branch_node->branch_next_node_id = this->best_potential_scopes[0]->id;
					new_branch_node->branch_next_node = this->best_potential_scopes[0];
				}
			}

			start_node_id = new_branch_node->id;
			start_node = new_branch_node;
		} else {
			if (this->best_step_types.size() == 0) {
				start_node_id = exit_node_id;
				start_node = exit_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					start_node_id = this->best_actions[0]->id;
					start_node = this->best_actions[0];
				} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
					start_node_id = this->best_existing_scopes[0]->id;
					start_node = this->best_existing_scopes[0];
				} else {
					start_node_id = this->best_potential_scopes[0]->id;
					start_node = this->best_potential_scopes[0];
				}
			}
		}

		if (this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();

			action_node->next_node_id = start_node_id;
			action_node->next_node = start_node;
		} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

			scope_node->next_node_id = start_node_id;
			scope_node->next_node = start_node;
		} else {
			BranchNode* branch_node = (BranchNode*)this->node_context.back();

			if (branch_node->experiment_is_branch) {
				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}
		}

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (s_index == (int)this->best_step_types.size()-1) {
				next_node_id = exit_node_id;
				next_node = exit_node;
			} else {
				if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
					next_node_id = this->best_actions[s_index+1]->id;
					next_node = this->best_actions[s_index+1];
				} else if (this->best_step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
					next_node_id = this->best_existing_scopes[s_index+1]->id;
					next_node = this->best_existing_scopes[s_index+1];
				} else {
					next_node_id = this->best_potential_scopes[s_index+1]->id;
					next_node = this->best_potential_scopes[s_index+1];
				}
			}

			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

				this->best_actions[s_index]->next_node_id = next_node_id;
				this->best_actions[s_index]->next_node = next_node;
			} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				containing_scope->nodes[this->best_existing_scopes[s_index]->id] = this->best_existing_scopes[s_index];

				this->best_existing_scopes[s_index]->next_node_id = next_node_id;
				this->best_existing_scopes[s_index]->next_node = next_node;
			} else {
				containing_scope->nodes[this->best_potential_scopes[s_index]->id] = this->best_potential_scopes[s_index];

				this->best_potential_scopes[s_index]->next_node_id = next_node_id;
				this->best_potential_scopes[s_index]->next_node = next_node;
			}
		}
		this->best_actions.clear();
		this->best_existing_scopes.clear();
		this->best_potential_scopes.clear();

		if (this->branch_experiment != NULL) {
			this->branch_experiment->finalize();
		}
	}

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		action_node->experiment = NULL;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
		scope_node->experiment = NULL;
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();
		branch_node->experiment = NULL;
	}
}
