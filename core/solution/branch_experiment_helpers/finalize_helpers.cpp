#include "branch_experiment.h"

#include <iostream>
#include <stdexcept>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "solution_helpers.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void BranchExperiment::finalize(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	if (this->is_pass_through) {
		new_pass_through(input_scope_depths_mappings,
						 output_scope_depths_mappings);
	} else {
		new_branch(input_scope_depths_mappings,
				   output_scope_depths_mappings);
	}
}

void BranchExperiment::new_branch(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								  map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	cout << "new_branch" << endl << endl;

	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	parent_scope->temp_states.insert(parent_scope->temp_states.end(),
		this->new_states.begin(), this->new_states.end());
	parent_scope->temp_state_nodes.insert(parent_scope->temp_state_nodes.end(),
		this->new_state_nodes.begin(), this->new_state_nodes.end());
	parent_scope->temp_state_scope_contexts.insert(parent_scope->temp_state_scope_contexts.end(),
		this->new_state_scope_contexts.begin(), this->new_state_scope_contexts.end());
	parent_scope->temp_state_node_contexts.insert(parent_scope->temp_state_node_contexts.end(),
		this->new_state_node_contexts.begin(), this->new_state_node_contexts.end());
	parent_scope->temp_state_obs_indexes.insert(parent_scope->temp_state_obs_indexes.end(),
		this->new_state_obs_indexes.begin(), this->new_state_obs_indexes.end());
	parent_scope->temp_state_new_local_indexes.insert(parent_scope->temp_state_new_local_indexes.end(),
		this->new_states.size(), -1);

	this->new_states.clear();
	this->new_state_nodes.clear();
	this->new_state_scope_contexts.clear();
	this->new_state_node_contexts.clear();
	this->new_state_obs_indexes.clear();

	Scope* containing_scope = solution->scopes[this->scope_context.back()];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = containing_scope;
	new_branch_node->id = containing_scope->node_counter;
	containing_scope->node_counter++;
	containing_scope->nodes[new_branch_node->id] = new_branch_node;

	#if defined(MDEBUG) && MDEBUG
	new_branch_node->verify_key = this;
	new_branch_node->verify_original_scores = this->verify_original_scores;
	new_branch_node->verify_branch_scores = this->verify_branch_scores;
	new_branch_node->verify_factors = this->verify_factors;
	#endif /* MDEBUG */

	int new_exit_node_id;
	AbstractNode* new_exit_node;
	if (!this->best_is_exit || this->best_exit_depth != 0) {
		new_exit_node_id = containing_scope->node_counter;
		containing_scope->node_counter++;
		new_exit_node = new ExitNode();
		new_exit_node->parent = containing_scope;
		new_exit_node->id = new_exit_node_id;
		containing_scope->nodes[new_exit_node->id] = new_exit_node;

		((ExitNode*)new_exit_node)->is_exit = this->best_is_exit;
		((ExitNode*)new_exit_node)->exit_depth = this->best_exit_depth;
		((ExitNode*)new_exit_node)->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
		if (this->best_exit_node == NULL) {
			((ExitNode*)new_exit_node)->exit_node_id = -1;
		} else {
			((ExitNode*)new_exit_node)->exit_node_id = this->best_exit_node->id;
		}
		((ExitNode*)new_exit_node)->exit_node = this->best_exit_node;
	} else {
		if (this->best_exit_node == NULL) {
			new_exit_node_id = -1;
		} else {
			new_exit_node_id = this->best_exit_node->id;
		}
		new_exit_node = this->best_exit_node;
	}

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = false;

	new_branch_node->original_score_mod = this->existing_average_score;
	new_branch_node->branch_score_mod = this->new_average_score;

	new_branch_node->decision_standard_deviation = this->existing_standard_deviation;

	finalize_branch_node_states(new_branch_node,
								this->existing_input_state_weights,
								this->existing_local_state_weights,
								this->existing_temp_state_weights,
								this->new_input_state_weights,
								this->new_local_state_weights,
								this->new_temp_state_weights,
								input_scope_depths_mappings,
								output_scope_depths_mappings);

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
		new_branch_node->branch_next_node_id = new_exit_node_id;
		new_branch_node->branch_next_node = new_exit_node;
	} else if (this->best_step_types[0] == STEP_TYPE_ACTION) {
		new_branch_node->branch_next_node_id = this->best_actions[0]->id;
		new_branch_node->branch_next_node = this->best_actions[0];
	} else if (this->best_step_types[0] == STEP_TYPE_POTENTIAL_SCOPE) {
		new_branch_node->branch_next_node_id = this->best_potential_scopes[0]->scope_node_placeholder->id;
		new_branch_node->branch_next_node = this->best_potential_scopes[0]->scope_node_placeholder;
	} else {
		new_branch_node->branch_next_node_id = this->best_existing_scopes[0]->scope_node_placeholder->id;
		new_branch_node->branch_next_node = this->best_existing_scopes[0]->scope_node_placeholder;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->best_step_types.size()-1) {
			next_node_id = new_exit_node_id;
			next_node = new_exit_node;
		} else {
			if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node_id = this->best_actions[s_index+1]->id;
				next_node = this->best_actions[s_index+1];
			} else if (this->best_step_types[s_index+1] == STEP_TYPE_POTENTIAL_SCOPE) {
				next_node_id = this->best_potential_scopes[s_index+1]->scope_node_placeholder->id;
				next_node = this->best_potential_scopes[s_index+1]->scope_node_placeholder;
			} else {
				next_node_id = this->best_existing_scopes[s_index+1]->scope_node_placeholder->id;
				next_node = this->best_existing_scopes[s_index+1]->scope_node_placeholder;
			}
		}

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

			this->best_actions[s_index]->next_node_id = next_node_id;
			this->best_actions[s_index]->next_node = next_node;
		} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			// for (map<int, AbstractNode*>::iterator it = this->best_potential_scopes[s_index]->scope->nodes.begin();
			// 		it != this->best_potential_scopes[s_index]->scope->nodes.end(); it++) {
			// 	if (it->second->type == NODE_TYPE_SCOPE) {
			// 		ScopeNode* scope_node = (ScopeNode*)it->second;
			// 		scope_node->inner_scope->parent_scope_node_parent_ids.push_back(scope_node->parent->id);
			// 		scope_node->inner_scope->parent_scope_node_ids.push_back(scope_node->id);
			// 		scope_node->inner_scope->parent_scope_nodes.push_back(scope_node);
			// 	}
			// }

			finalize_potential_scope(this->scope_context,
									 this->node_context,
									 this->best_potential_scopes[s_index],
									 input_scope_depths_mappings,
									 output_scope_depths_mappings);
			ScopeNode* new_scope_node = this->best_potential_scopes[s_index]->scope_node_placeholder;
			this->best_potential_scopes[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->next_node_id = next_node_id;
			new_scope_node->next_node = next_node;

			delete this->best_potential_scopes[s_index];
		} else {
			finalize_potential_scope(this->scope_context,
									 this->node_context,
									 this->best_existing_scopes[s_index],
									 input_scope_depths_mappings,
									 output_scope_depths_mappings);
			ScopeNode* new_scope_node = this->best_existing_scopes[s_index]->scope_node_placeholder;
			this->best_existing_scopes[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->next_node_id = next_node_id;
			new_scope_node->next_node = next_node;

			delete this->best_existing_scopes[s_index];
		}
	}
	this->best_actions.clear();
	this->best_potential_scopes.clear();
	this->best_existing_scopes.clear();
}

void BranchExperiment::new_pass_through(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
										map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	cout << "new_pass_through" << endl << endl;

	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	parent_scope->temp_states.insert(parent_scope->temp_states.end(),
		this->new_states.begin(), this->new_states.end());
	parent_scope->temp_state_nodes.insert(parent_scope->temp_state_nodes.end(),
		this->new_state_nodes.begin(), this->new_state_nodes.end());
	parent_scope->temp_state_scope_contexts.insert(parent_scope->temp_state_scope_contexts.end(),
		this->new_state_scope_contexts.begin(), this->new_state_scope_contexts.end());
	parent_scope->temp_state_node_contexts.insert(parent_scope->temp_state_node_contexts.end(),
		this->new_state_node_contexts.begin(), this->new_state_node_contexts.end());
	parent_scope->temp_state_obs_indexes.insert(parent_scope->temp_state_obs_indexes.end(),
		this->new_state_obs_indexes.begin(), this->new_state_obs_indexes.end());
	parent_scope->temp_state_new_local_indexes.insert(parent_scope->temp_state_new_local_indexes.end(),
		this->new_states.size(), -1);

	this->new_states.clear();
	this->new_state_nodes.clear();
	this->new_state_scope_contexts.clear();
	this->new_state_node_contexts.clear();
	this->new_state_obs_indexes.clear();

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
	} else if (this->best_step_types[0] == STEP_TYPE_POTENTIAL_SCOPE) {
		new_branch_node->branch_next_node_id = this->best_potential_scopes[0]->scope_node_placeholder->id;
		new_branch_node->branch_next_node = this->best_potential_scopes[0]->scope_node_placeholder;
	} else {
		new_branch_node->branch_next_node_id = this->best_existing_scopes[0]->scope_node_placeholder->id;
		new_branch_node->branch_next_node = this->best_existing_scopes[0]->scope_node_placeholder;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		AbstractNode* next_node;
		if (s_index == (int)this->best_step_types.size()-1) {
			next_node = new_exit_node;
		} else {
			if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
				next_node = this->best_actions[s_index+1];
			} else if (this->best_step_types[s_index+1] == STEP_TYPE_POTENTIAL_SCOPE) {
				next_node = this->best_potential_scopes[s_index+1]->scope_node_placeholder;
			} else {
				next_node = this->best_existing_scopes[s_index+1]->scope_node_placeholder;
			}
		}

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

			this->best_actions[s_index]->next_node_id = next_node->id;
			this->best_actions[s_index]->next_node = next_node;
		} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			// for (map<int, AbstractNode*>::iterator it = this->best_potential_scopes[s_index]->scope->nodes.begin();
			// 		it != this->best_potential_scopes[s_index]->scope->nodes.end(); it++) {
			// 	if (it->second->type == NODE_TYPE_SCOPE) {
			// 		ScopeNode* scope_node = (ScopeNode*)it->second;
			// 		scope_node->inner_scope->parent_scope_node_parent_ids.push_back(scope_node->parent->id);
			// 		scope_node->inner_scope->parent_scope_node_ids.push_back(scope_node->id);
			// 		scope_node->inner_scope->parent_scope_nodes.push_back(scope_node);
			// 	}
			// }

			finalize_potential_scope(this->scope_context,
									 this->node_context,
									 this->best_potential_scopes[s_index],
									 input_scope_depths_mappings,
									 output_scope_depths_mappings);
			ScopeNode* new_scope_node = this->best_potential_scopes[s_index]->scope_node_placeholder;
			this->best_potential_scopes[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->next_node_id = next_node->id;
			new_scope_node->next_node = next_node;

			delete this->best_potential_scopes[s_index];
		} else {
			finalize_potential_scope(this->scope_context,
									 this->node_context,
									 this->best_existing_scopes[s_index],
									 input_scope_depths_mappings,
									 output_scope_depths_mappings);
			ScopeNode* new_scope_node = this->best_existing_scopes[s_index]->scope_node_placeholder;
			this->best_existing_scopes[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->next_node_id = next_node->id;
			new_scope_node->next_node = next_node;

			delete this->best_existing_scopes[s_index];
		}
	}
	this->best_actions.clear();
	this->best_potential_scopes.clear();
	this->best_existing_scopes.clear();

	new_exit_node->is_exit = this->best_is_exit;
	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
	if (this->best_exit_node == NULL) {
		new_exit_node->exit_node_id = -1;
	} else {
		new_exit_node->exit_node_id = this->best_exit_node->id;
	}
	new_exit_node->exit_node = this->best_exit_node;
}
