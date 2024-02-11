#include "outer_experiment.h"

using namespace std;

void OuterExperiment::finalize() {
	Scope* new_root_scope = new Scope();
	new_root_scope->id = solution->scope_counter;
	solution->scope_counter++;
	solution->scopes[new_root_scope->id] = new_root_scope;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_root_scope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	new_root_scope->nodes[starting_noop_node->id] = starting_noop_node;

	if (this->best_step_types[0] == STEP_TYPE_ACTION) {
		starting_noop_node->next_node_id = 1;
		starting_noop_node->next_node = this->best_actions[0];
	} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
		starting_noop_node->next_node_id = 1;
		starting_noop_node->next_node = this->best_existing_scopes[0];
	} else {
		starting_noop_node->next_node_id = 1;
		starting_noop_node->next_node = this->best_potential_scopes[0];
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->best_step_types.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->best_actions[s_index+1]->id;
				next_node = this->best_actions[s_index+1];
			} else if (this->best_step_types[s_index+1] == STEP_TYPE_POTENTIAL_SCOPE) {
				next_node_id = this->best_potential_scopes[s_index+1]->scope_node_placeholder->id;
				next_node = this->best_potential_scopes[s_index+1]->scope_node_placeholder;
			} else if (this->best_step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
				next_node_id = this->best_existing_scopes[s_index+1]->scope_node_placeholder->id;
				next_node = this->best_existing_scopes[s_index+1]->scope_node_placeholder;
			} else {
				next_node_id = this->best_root_scope_nodes[s_index+1]->id;
				next_node = this->best_root_scope_nodes[s_index+1];
			}
		}

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->parent = new_root_scope;
			new_root_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

			this->best_actions[s_index]->next_node_id = next_node_id;
			this->best_actions[s_index]->next_node = next_node;
		} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->best_existing_scopes[s_index]->parent = new_root_scope;
			new_root_scope->nodes[this->best_existing_scopes[s_index]->id] = this->best_existing_scopes[s_index];

			this->best_existing_scopes[s_index]->next_node_id = next_node_id;
			this->best_existing_scopes[s_index]->next_node = next_node;
		} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->best_potential_scopes[s_index]->parent = new_root_scope;
			new_root_scope->nodes[this->best_potential_scopes[s_index]->id] = this->best_potential_scopes[s_index];

			solution->scopes[this->best_potential_scopes[s_index]->scope->id] = this->best_potential_scopes[s_index]->scope;

			this->best_potential_scopes[s_index]->next_node_id = next_node_id;
			this->best_potential_scopes[s_index]->next_node = next_node;
		}
	}
	this->best_actions.clear();
	this->best_existing_scopes.clear();
	this->best_potential_scopes.clear();

	new_root_scope->node_counter = 1 + (int)this->best_step_types.size();

	new_root_scope->starting_node_id = 0;
	new_root_scope->starting_node = starting_noop_node;

	solution->root = new_root_scope;
}
