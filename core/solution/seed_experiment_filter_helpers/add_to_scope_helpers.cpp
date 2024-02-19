#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::add_to_scope() {
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->parent = this->scope_context.back();
			this->actions[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->existing_scopes[s_index]->parent = this->scope_context.back();
			this->existing_scopes[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
		} else {
			this->potential_scopes[s_index]->parent = this->scope_context.back();
			this->potential_scopes[s_index]->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;

			int new_scope_id = solution->scope_counter;
			solution->scope_counter++;
			this->potential_scopes[s_index]->scope->id = new_scope_id;

			for (map<int, AbstractNode*>::iterator it = this->potential_scopes[s_index]->scope->nodes.begin();
					it != this->potential_scopes[s_index]->scope->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_BRANCH) {
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->scope_context_ids[0] = new_scope_id;
					for (int i_index = 0; i_index < (int)branch_node->input_scope_context_ids.size(); i_index++) {
						if (branch_node->input_scope_context_ids[i_index].size() > 0) {
							branch_node->input_scope_context_ids[i_index][0] = new_scope_id;
						}
					}
				}
			}
		}
	}

	int end_node_id;
	AbstractNode* end_node;
	if (this->exit_depth > 0) {
		ExitNode* new_exit_node = new ExitNode();
		new_exit_node->parent = this->scope_context.back();
		new_exit_node->id = this->scope_context.back()->node_counter;
		this->scope_context.back()->node_counter++;
		this->scope_context.back()->nodes[new_exit_node->id] = new_exit_node;

		new_exit_node->exit_depth = this->exit_depth;
		new_exit_node->next_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->exit_depth]->id;
		if (this->exit_next_node == NULL) {
			new_exit_node->next_node_id = -1;
		} else {
			new_exit_node->next_node_id = this->exit_next_node->id;
		}
		new_exit_node->next_node = this->exit_next_node;

		this->exit_node = new_exit_node;

		end_node_id = new_exit_node->id;
		end_node = new_exit_node;
	} else {
		if (this->exit_next_node == NULL) {
			end_node_id = -1;
		} else {
			end_node_id = this->exit_next_node->id;
		}
		end_node = this->exit_next_node;
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->step_types.size()-1) {
			next_node_id = end_node_id;
			new_exit_node = end_node;
		} else {
			if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->actions[s_index+1]->id;
				next_node = this->actions[s_index+1];
			} else if (this->step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
				next_node_id = this->existing_scopes[s_index+1]->id;
				next_node = this->existing_scopes[s_index+1];
			} else {
				next_node_id = this->potential_scopes[s_index+1]->id;
				next_node = this->potential_scopes[s_index+1];
			}
		}

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes[this->actions[s_index]->id] = this->actions[s_index];

			this->actions[s_index]->next_node_id = next_node_id;
			this->actions[s_index]->next_node = next_node;
		} else if (this->step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			this->scope_context.back()->nodes[this->existing_scopes[s_index]->id] = this->existing_scopes[s_index];

			this->existing_scopes[s_index]->next_node_id = next_node_id;
			this->existing_scopes[s_index]->next_node = next_node;
		} else {
			this->scope_context.back()->nodes[this->potential_scopes[s_index]->id] = this->potential_scopes[s_index];

			this->potential_scopes[s_index]->next_node_id = next_node_id;
			this->potential_scopes[s_index]->next_node = next_node;
		}
	}
}
