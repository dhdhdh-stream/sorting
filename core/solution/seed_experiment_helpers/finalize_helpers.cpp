#include "seed_experiment.h"

using namespace std;

void SeedExperiment::finalize() {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		int start_node_id;
		AbstractNode* start_node;
		if (this->scope_context.size() > 0) {
			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->parent = this->scope_context.back();
			new_branch_node->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;
			this->scope_context.back()->nodes[new_branch_node->id] = new_branch_node;

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

				if (this->is_branch) {
					new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
					new_branch_node->original_next_node = branch_node->branch_next_node;
				} else {
					new_branch_node->original_next_node_id = branch_node->original_next_node_id;
					new_branch_node->original_next_node = branch_node->original_next_node;
				}
			}

			if (this->best_step_types.size() == 0) {
				if (this->best_exit_depth == 0) {
					new_branch_node->branch_next_node_id = this->best_exit_next_node->id;
					new_branch_node->branch_next_node = this->best_exit_next_node;
				} else {
					new_branch_node->branch_next_node_id = this->best_exit_node->id;
					new_branch_node->branch_next_node = this->best_exit_node;
				}
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
				start_node_id = this->best_exit_next_node->id;
				start_node = this->best_exit_next_node;
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

			if (this->is_branch) {
				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}
		}

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
				solution->scopes[this->best_potential_scopes[s_index]->scope->id] = this->best_potential_scopes[s_index]->scope;
			}
		}

		for (int g_index = this->gathers.size()-1; g_index >= 0; g_index--) {
			this->gathers[g_index]->
		}
	}

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
			if (action_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		action_node->experiments.erase(action_node->experiments.begin() + e_index);
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
			if (scope_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		scope_node->experiments.erase(scope_node->experiments.begin() + e_index);
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
			if (branch_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		branch_node->experiments.erase(branch_node->experiments.begin() + e_index);
		branch_node->experiment_is_branch.erase(branch_node->experiment_is_branch.begin() + e_index);
	}
}
