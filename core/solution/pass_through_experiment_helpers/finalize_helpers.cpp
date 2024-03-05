#include "pass_through_experiment.h"

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

void PassThroughExperiment::finalize() {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		int start_node_id;
		AbstractNode* start_node;
		if (this->scope_context.size() > 1) {
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

				if (this->throw_id == -1) {
					new_branch_node->original_next_node_id = scope_node->next_node_id;
					new_branch_node->original_next_node = scope_node->next_node;
				} else {
					map<int, AbstractNode*>::iterator it = scope_node->catches.find(this->throw_id);
					if (it == scope_node->catches.end()) {
						ExitNode* new_throw_node = new ExitNode();
						new_throw_node->parent = this->scope_context.back();
						new_throw_node->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;

						this->scope_context.back()->nodes[new_throw_node->id] = new_throw_node;

						new_throw_node->exit_depth = -1;
						new_throw_node->next_node_parent_id = -1;
						new_throw_node->next_node_id = -1;
						new_throw_node->next_node = NULL;
						new_throw_node->throw_id = this->throw_id;

						new_branch_node->original_next_node_id = new_throw_node->id;
						new_branch_node->original_next_node = new_throw_node;
					} else {
						new_branch_node->original_next_node_id = it->second->id;
						new_branch_node->original_next_node = it->second;
					}
				}
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
				if (this->exit_node != NULL) {
					new_branch_node->branch_next_node_id = this->exit_node->id;
					new_branch_node->branch_next_node = this->exit_node;
				} else {
					if (this->best_exit_next_node == NULL) {
						new_branch_node->branch_next_node_id = -1;
					} else {
						new_branch_node->branch_next_node_id = this->best_exit_next_node->id;
					}
					new_branch_node->branch_next_node = this->best_exit_next_node;
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
				if (this->exit_node != NULL) {
					start_node_id = this->exit_node->id;
					start_node = this->exit_node;
				} else {
					if (this->best_exit_next_node == NULL) {
						start_node_id = -1;
					} else {
						start_node_id = this->best_exit_next_node->id;
					}
					start_node = this->best_exit_next_node;
				}
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

			if (this->throw_id == -1) {
				scope_node->next_node_id = start_node_id;
				scope_node->next_node = start_node;
			} else {
				scope_node->catch_ids[this->throw_id] = start_node_id;
				scope_node->catches[this->throw_id] = start_node;
			}
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

		if (this->exit_node != NULL) {
			this->scope_context.back()->nodes[this->exit_node->id] = this->exit_node;
		}

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->scope_context.back()->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
			} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				this->scope_context.back()->nodes[this->best_existing_scopes[s_index]->id] = this->best_existing_scopes[s_index];
			} else {
				this->scope_context.back()->nodes[this->best_potential_scopes[s_index]->id] = this->best_potential_scopes[s_index];

				solution->scopes[this->best_potential_scopes[s_index]->scope->id] = this->best_potential_scopes[s_index]->scope;
			}
		}

		this->best_actions.clear();
		this->best_existing_scopes.clear();
		this->best_potential_scopes.clear();
		this->exit_node = NULL;

		for (int v_index = 0; v_index < (int)this->verify_experiments.size(); v_index++) {
			PassThroughExperiment* parent;
			if (this->verify_experiments[v_index]->type == EXPERIMENT_TYPE_BRANCH) {
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments[v_index];
				parent = pass_through_experiment->parent_experiment;
			} else {
				BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments[v_index];
				parent = branch_experiment->parent_experiment;
			}

			int matching_index;
			for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
				if (parent->child_experiments[c_index] == this->verify_experiments[v_index]) {
					matching_index = c_index;
					break;
				}
			}
			parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

			this->verify_experiments[v_index]->result = EXPERIMENT_RESULT_SUCCESS;
			this->verify_experiments[v_index]->finalize();
			delete this->verify_experiments[v_index];
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
		action_node->experiments.erase(action_node->experiments.begin() + experiment_index);
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
			if (scope_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		scope_node->experiments.erase(scope_node->experiments.begin() + experiment_index);
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();
		int experiment_index;
		for (int e_index = 0; e_index < (int)branch_node->experiments.size(); e_index++) {
			if (branch_node->experiments[e_index] == this) {
				experiment_index = e_index;
			}
		}
		branch_node->experiments.erase(branch_node->experiments.begin() + experiment_index);
		branch_node->experiment_types.erase(branch_node->experiment_types.begin() + experiment_index);
	}
}
