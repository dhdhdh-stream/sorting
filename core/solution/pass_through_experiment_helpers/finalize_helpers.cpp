#include "pass_through_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::score_finalize() {
	cout << "score success" << endl;

	Scope* containing_scope = solution->scopes[this->scope_context.back()];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = containing_scope;
	new_branch_node->id = containing_scope->node_counter;
	containing_scope->node_counter++;
	containing_scope->nodes[new_branch_node->id] = new_branch_node;

	ExitNode* new_exit_node = new ExitNode();
	new_exit_node->parent = containing_scope;
	new_exit_node->id = containing_scope->node_counter;
	containing_scope->node_counter++;
	containing_scope->nodes[new_exit_node->id] = new_exit_node;

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = true;

	new_branch_node->original_score_mod = 0.0;
	new_branch_node->branch_score_mod = 0.0;

	new_branch_node->decision_standard_deviation = 0.0;

	if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;
		new_branch_node->original_next_node = action_node->next_node;

		action_node->next_node_id = new_branch_node->id;
		action_node->next_node = new_branch_node;
	} else {
		ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;
		new_branch_node->original_next_node = scope_node->next_node;

		scope_node->next_node_id = new_branch_node->id;
		scope_node->next_node = new_branch_node;
	}

	if (this->best_step_types.size() == 0) {
		new_branch_node->branch_next_node_id = new_exit_node->id;
		new_branch_node->branch_next_node = new_exit_node;
	} else if (this->best_step_types[0] == STEP_TYPE_ACTION) {
		new_branch_node->branch_next_node_id = this->best_actions[0]->id;
		new_branch_node->branch_next_node = this->best_actions[0];
	} else {
		new_branch_node->branch_next_node_id = this->best_potential_scopes[0]->scope_node_placeholder->id;
		new_branch_node->branch_next_node = this->best_potential_scopes[0]->scope_node_placeholder;
	}

	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		AbstractNode* next_node;
		if (s_index == (int)this->best_step_types.size()-1) {
			next_node = new_exit_node;
		} else {
			if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node = this->best_actions[s_index+1];
			} else {
				next_node = this->best_potential_scopes[s_index+1]->scope_node_placeholder;
			}
		}

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

			this->best_actions[s_index]->next_node_id = next_node->id;
			this->best_actions[s_index]->next_node = next_node;
		} else {
			finalize_potential_scope(this->scope_context,
									 this->node_context,
									 this->best_potential_scopes[s_index],
									 input_scope_depths_mappings,
									 output_scope_depths_mappings);
			ScopeNode* new_scope_node = this->best_potential_scopes[s_index]->scope_node_placeholder;
			this->best_potential_scopes[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->is_loop = false;
			new_scope_node->continue_score_mod = 0.0;
			new_scope_node->halt_score_mod = 0.0;
			new_scope_node->decision_standard_deviation = 0.0;
			new_scope_node->max_iters = 0;

			new_scope_node->next_node_id = next_node->id;
			new_scope_node->next_node = next_node;

			delete this->best_potential_scopes[s_index];
		}
	}
	this->best_actions.clear();
	this->best_potential_scopes.clear();

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
	if (this->best_exit_node == NULL) {
		new_exit_node->exit_node_id = -1;
	} else {
		new_exit_node->exit_node_id = this->best_exit_node->id;
	}
	new_exit_node->exit_node = this->best_exit_node;

	this->state = PASS_THROUGH_EXPERIMENT_STATE_SUCCESS;
}
