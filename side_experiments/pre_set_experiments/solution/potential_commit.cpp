#include "potential_commit.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

PotentialCommit::~PotentialCommit() {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		delete this->actions[a_index];
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void PotentialCommit::activate(AbstractNode*& curr_node,
							   Problem* problem,
							   vector<ContextLayer>& context,
							   RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->explore_activate(
				problem,
				run_helper);
		} else {
			this->scopes[s_index]->explore_activate(
				problem,
				context,
				run_helper);
		}
	}

	curr_node = this->exit_next_node;
}

void PotentialCommit::finalize() {
	Scope* parent_scope = this->node_context->parent;

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->parent = parent_scope;
			this->actions[s_index]->id = parent_scope->node_counter;
			parent_scope->node_counter++;
			parent_scope->nodes[this->actions[s_index]->id] = this->actions[s_index];

			this->actions[s_index]->average_instances_per_run = this->node_context->average_instances_per_run;
		} else {
			this->scopes[s_index]->parent = parent_scope;
			this->scopes[s_index]->id = parent_scope->node_counter;
			parent_scope->node_counter++;
			parent_scope->nodes[this->scopes[s_index]->id] = this->scopes[s_index];

			this->scopes[s_index]->average_instances_per_run = this->node_context->average_instances_per_run;
		}
	}

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		ActionNode* new_ending_node = new ActionNode();
		new_ending_node->parent = parent_scope;
		new_ending_node->id = parent_scope->node_counter;
		parent_scope->node_counter++;
		parent_scope->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->action = Action(ACTION_NOOP);

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		new_ending_node->average_instances_per_run = 1.0;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->step_types.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->actions[s_index+1]->id;
				next_node = this->actions[s_index+1];
			} else {
				next_node_id = this->scopes[s_index+1]->id;
				next_node = this->scopes[s_index+1];
			}
		}

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->next_node_id = next_node_id;
			this->actions[s_index]->next_node = next_node;
		} else {
			this->scopes[s_index]->next_node_id = next_node_id;
			this->scopes[s_index]->next_node = next_node;
		}
	}

	int start_node_id;
	AbstractNode* start_node;
	if (this->step_types[0] == STEP_TYPE_ACTION) {
		start_node_id = this->actions[0]->id;
		start_node = this->actions[0];
	} else {
		start_node_id = this->scopes[0]->id;
		start_node = this->scopes[0];
	}

	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			action_node->next_node_id = start_node_id;
			action_node->next_node = start_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			scope_node->next_node_id = start_node_id;
			scope_node->next_node = start_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}
		}
		break;
	}

	this->actions.clear();
	this->scopes.clear();
}
