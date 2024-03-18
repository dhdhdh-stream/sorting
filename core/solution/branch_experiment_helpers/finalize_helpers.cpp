#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void BranchExperiment::finalize() {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		if (this->original_count == 0) {
			new_pass_through();
		} else {
			new_branch();
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

void BranchExperiment::new_branch() {
	if (this->exit_node != NULL) {
		this->scope_context.back()->nodes[this->exit_node->id] = this->exit_node;
	}

	this->scope_context.back()->nodes[this->branch_node->id] = this->branch_node;

	this->branch_node->scope_context = this->scope_context;
	for (int c_index = 0; c_index < (int)this->branch_node->scope_context.size(); c_index++) {
		this->branch_node->scope_context_ids.push_back(this->branch_node->scope_context[c_index]->id);
	}
	this->branch_node->node_context = this->node_context;
	this->branch_node->node_context.back() = this->branch_node;
	for (int c_index = 0; c_index < (int)this->branch_node->node_context.size(); c_index++) {
		this->branch_node->node_context_ids.push_back(this->branch_node->node_context[c_index]->id);
	}

	this->branch_node->is_pass_through = false;

	this->branch_node->original_average_score = this->existing_average_score
		+ this->original_bias * this->existing_score_standard_deviation;
	this->branch_node->branch_average_score = this->new_average_score;

	this->branch_node->input_max_depth = 0;
	vector<int> input_mapping(this->input_scope_contexts.size(), -1);
	for (int i_index = 0; i_index < (int)this->existing_linear_weights.size(); i_index++) {
		if (this->existing_linear_weights[i_index] != 0.0) {
			if (input_mapping[i_index] == -1) {
				input_mapping[i_index] = (int)this->branch_node->input_scope_contexts.size();
				this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				vector<int> scope_context_ids;
				for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
					scope_context_ids.push_back(this->input_scope_contexts[i_index][c_index]->id);
				}
				this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
				this->branch_node->input_node_contexts.push_back(this->input_node_contexts[i_index]);
				vector<int> node_context_ids;
				for (int c_index = 0; c_index < (int)this->input_node_contexts[i_index].size(); c_index++) {
					node_context_ids.push_back(this->input_node_contexts[i_index][c_index]->id);
				}
				this->branch_node->input_node_context_ids.push_back(node_context_ids);
				if ((int)this->input_scope_contexts[i_index].size() > this->branch_node->input_max_depth) {
					this->branch_node->input_max_depth = (int)this->input_scope_contexts[i_index].size();
				}
			}
		}
	}
	for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
		for (int v_index = 0; v_index < (int)this->existing_network_input_indexes[i_index].size(); v_index++) {
			int original_index = this->existing_network_input_indexes[i_index][v_index];
			if (input_mapping[original_index] == -1) {
				input_mapping[original_index] = (int)this->branch_node->input_scope_contexts.size();
				this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[original_index]);
				vector<int> scope_context_ids;
				for (int c_index = 0; c_index < (int)this->input_scope_contexts[original_index].size(); c_index++) {
					scope_context_ids.push_back(this->input_scope_contexts[original_index][c_index]->id);
				}
				this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
				this->branch_node->input_node_contexts.push_back(this->input_node_contexts[original_index]);
				vector<int> node_context_ids;
				for (int c_index = 0; c_index < (int)this->input_node_contexts[original_index].size(); c_index++) {
					node_context_ids.push_back(this->input_node_contexts[original_index][c_index]->id);
				}
				this->branch_node->input_node_context_ids.push_back(node_context_ids);
				if ((int)this->input_scope_contexts[original_index].size() > this->branch_node->input_max_depth) {
					this->branch_node->input_max_depth = (int)this->input_scope_contexts[original_index].size();
				}
			}
		}
	}
	for (int i_index = 0; i_index < (int)this->new_linear_weights.size(); i_index++) {
		if (this->new_linear_weights[i_index] != 0.0) {
			if (input_mapping[i_index] == -1) {
				input_mapping[i_index] = (int)this->branch_node->input_scope_contexts.size();
				this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				vector<int> scope_context_ids;
				for (int c_index = 0; c_index < (int)this->input_scope_contexts[i_index].size(); c_index++) {
					scope_context_ids.push_back(this->input_scope_contexts[i_index][c_index]->id);
				}
				this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
				this->branch_node->input_node_contexts.push_back(this->input_node_contexts[i_index]);
				vector<int> node_context_ids;
				for (int c_index = 0; c_index < (int)this->input_node_contexts[i_index].size(); c_index++) {
					node_context_ids.push_back(this->input_node_contexts[i_index][c_index]->id);
				}
				this->branch_node->input_node_context_ids.push_back(node_context_ids);
				if ((int)this->input_scope_contexts[i_index].size() > this->branch_node->input_max_depth) {
					this->branch_node->input_max_depth = (int)this->input_scope_contexts[i_index].size();
				}
			}
		}
	}
	for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
		for (int v_index = 0; v_index < (int)this->new_network_input_indexes[i_index].size(); v_index++) {
			int original_index = this->new_network_input_indexes[i_index][v_index];
			if (input_mapping[original_index] == -1) {
				input_mapping[original_index] = (int)this->branch_node->input_scope_contexts.size();
				this->branch_node->input_scope_contexts.push_back(this->input_scope_contexts[original_index]);
				vector<int> scope_context_ids;
				for (int c_index = 0; c_index < (int)this->input_scope_contexts[original_index].size(); c_index++) {
					scope_context_ids.push_back(this->input_scope_contexts[original_index][c_index]->id);
				}
				this->branch_node->input_scope_context_ids.push_back(scope_context_ids);
				this->branch_node->input_node_contexts.push_back(this->input_node_contexts[original_index]);
				vector<int> node_context_ids;
				for (int c_index = 0; c_index < (int)this->input_node_contexts[original_index].size(); c_index++) {
					node_context_ids.push_back(this->input_node_contexts[original_index][c_index]->id);
				}
				this->branch_node->input_node_context_ids.push_back(node_context_ids);
				if ((int)this->input_scope_contexts[original_index].size() > this->branch_node->input_max_depth) {
					this->branch_node->input_max_depth = (int)this->input_scope_contexts[original_index].size();
				}
			}
		}
	}

	for (int i_index = 0; i_index < (int)this->existing_linear_weights.size(); i_index++) {
		if (this->existing_linear_weights[i_index] != 0.0) {
			this->branch_node->linear_original_input_indexes.push_back(input_mapping[i_index]);
			this->branch_node->linear_original_weights.push_back(this->existing_linear_weights[i_index]);
		}
	}
	for (int i_index = 0; i_index < (int)this->new_linear_weights.size(); i_index++) {
		if (this->new_linear_weights[i_index] != 0.0) {
			this->branch_node->linear_branch_input_indexes.push_back(input_mapping[i_index]);
			this->branch_node->linear_branch_weights.push_back(this->new_linear_weights[i_index]);
		}
	}

	for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
		vector<int> input_indexes;
		for (int v_index = 0; v_index < (int)this->existing_network_input_indexes[i_index].size(); v_index++) {
			input_indexes.push_back(input_mapping[this->existing_network_input_indexes[i_index][v_index]]);
		}
		this->branch_node->original_network_input_indexes.push_back(input_indexes);
	}
	this->branch_node->original_network = this->existing_network;
	this->existing_network = NULL;
	for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
		vector<int> input_indexes;
		for (int v_index = 0; v_index < (int)this->new_network_input_indexes[i_index].size(); v_index++) {
			input_indexes.push_back(input_mapping[this->new_network_input_indexes[i_index][v_index]]);
		}
		this->branch_node->branch_network_input_indexes.push_back(input_indexes);
	}
	this->branch_node->branch_network = this->new_network;
	this->new_network = NULL;

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		this->branch_node->original_next_node_id = action_node->next_node_id;
		this->branch_node->original_next_node = action_node->next_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		if (this->throw_id == -1) {
			this->branch_node->original_next_node_id = scope_node->next_node_id;
			this->branch_node->original_next_node = scope_node->next_node;
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

				this->branch_node->original_next_node_id = new_throw_node->id;
				this->branch_node->original_next_node = new_throw_node;
			} else {
				this->branch_node->original_next_node_id = it->second->id;
				this->branch_node->original_next_node = it->second;
			}
		}
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			this->branch_node->original_next_node_id = branch_node->branch_next_node_id;
			this->branch_node->original_next_node = branch_node->branch_next_node;
		} else {
			this->branch_node->original_next_node_id = branch_node->original_next_node_id;
			this->branch_node->original_next_node = branch_node->original_next_node;
		}
	}

	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			this->branch_node->branch_next_node_id = this->exit_node->id;
			this->branch_node->branch_next_node = this->exit_node;
		} else {
			if (this->best_exit_next_node == NULL) {
				this->branch_node->branch_next_node_id = -1;
			} else {
				this->branch_node->branch_next_node_id = this->best_exit_next_node->id;
			}
			this->branch_node->branch_next_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->best_actions[0]->id;
			this->branch_node->branch_next_node = this->best_actions[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		}
	}

	#if defined(MDEBUG) && MDEBUG
	solution->verify_key = this;
	solution->verify_problems = this->verify_problems;
	this->verify_problems.clear();
	solution->verify_seeds = this->verify_seeds;

	this->branch_node->verify_key = this;
	this->branch_node->verify_original_scores = this->verify_original_scores;
	this->branch_node->verify_branch_scores = this->verify_branch_scores;
	#endif /* MDEBUG */

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		action_node->next_node_id = this->branch_node->id;
		action_node->next_node = this->branch_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		if (this->throw_id == -1) {
			scope_node->next_node_id = this->branch_node->id;
			scope_node->next_node = this->branch_node;
		} else {
			scope_node->catch_ids[this->throw_id] = this->branch_node->id;
			scope_node->catches[this->throw_id] = this->branch_node;
		}
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			branch_node->branch_next_node_id = this->branch_node->id;
			branch_node->branch_next_node = this->branch_node;
		} else {
			branch_node->original_next_node_id = this->branch_node->id;
			branch_node->original_next_node = this->branch_node;
		}
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
		} else {
			this->scope_context.back()->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];
		}
	}

	this->best_actions.clear();
	this->best_scopes.clear();
	this->branch_node = NULL;
	this->exit_node = NULL;
}

void BranchExperiment::new_pass_through() {
	if (this->exit_node != NULL) {
		this->scope_context.back()->nodes[this->exit_node->id] = this->exit_node;
	}

	/**
	 * - simply always add this->branch_node even if this->scope_context.size() == 1
	 */
	this->scope_context.back()->nodes[this->branch_node->id] = this->branch_node;

	this->branch_node->scope_context = this->scope_context;
	for (int c_index = 0; c_index < (int)this->branch_node->scope_context.size(); c_index++) {
		this->branch_node->scope_context_ids.push_back(this->branch_node->scope_context[c_index]->id);
	}
	this->branch_node->node_context = this->node_context;
	this->branch_node->node_context.back() = this->branch_node;
	for (int c_index = 0; c_index < (int)this->branch_node->node_context.size(); c_index++) {
		this->branch_node->node_context_ids.push_back(this->branch_node->node_context[c_index]->id);
	}

	this->branch_node->is_pass_through = true;

	this->branch_node->original_average_score = 0.0;
	this->branch_node->branch_average_score = 0.0;

	this->branch_node->input_max_depth = 0;

	this->branch_node->original_network = NULL;
	this->branch_node->branch_network = NULL;

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		this->branch_node->original_next_node_id = action_node->next_node_id;
		this->branch_node->original_next_node = action_node->next_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		if (this->throw_id == -1) {
			this->branch_node->original_next_node_id = scope_node->next_node_id;
			this->branch_node->original_next_node = scope_node->next_node;
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

				this->branch_node->original_next_node_id = new_throw_node->id;
				this->branch_node->original_next_node = new_throw_node;
			} else {
				this->branch_node->original_next_node_id = it->second->id;
				this->branch_node->original_next_node = it->second;
			}
		}
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			this->branch_node->original_next_node_id = branch_node->branch_next_node_id;
			this->branch_node->original_next_node = branch_node->branch_next_node;
		} else {
			this->branch_node->original_next_node_id = branch_node->original_next_node_id;
			this->branch_node->original_next_node = branch_node->original_next_node;
		}
	}

	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			this->branch_node->branch_next_node_id = this->exit_node->id;
			this->branch_node->branch_next_node = this->exit_node;
		} else {
			if (this->best_exit_next_node == NULL) {
				this->branch_node->branch_next_node_id = -1;
			} else {
				this->branch_node->branch_next_node_id = this->best_exit_next_node->id;
			}
			this->branch_node->branch_next_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			this->branch_node->branch_next_node_id = this->best_actions[0]->id;
			this->branch_node->branch_next_node = this->best_actions[0];
		} else {
			this->branch_node->branch_next_node_id = this->best_scopes[0]->id;
			this->branch_node->branch_next_node = this->best_scopes[0];
		}
	}

	if (this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();

		action_node->next_node_id = this->branch_node->id;
		action_node->next_node = this->branch_node;
	} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->node_context.back();

		if (this->throw_id == -1) {
			scope_node->next_node_id = this->branch_node->id;
			scope_node->next_node = this->branch_node;
		} else {
			scope_node->catch_ids[this->throw_id] = this->branch_node->id;
			scope_node->catches[this->throw_id] = this->branch_node;
		}
	} else {
		BranchNode* branch_node = (BranchNode*)this->node_context.back();

		if (this->is_branch) {
			branch_node->branch_next_node_id = this->branch_node->id;
			branch_node->branch_next_node = this->branch_node;
		} else {
			branch_node->original_next_node_id = this->branch_node->id;
			branch_node->original_next_node = this->branch_node;
		}
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->scope_context.back()->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];
		} else {
			this->scope_context.back()->nodes[this->best_scopes[s_index]->id] = this->best_scopes[s_index];
		}
	}

	this->best_actions.clear();
	this->best_scopes.clear();
	this->branch_node = NULL;
	this->exit_node = NULL;
}
