#include "branch_experiment.h"

using namespace std;

void BranchExperiment::finalize(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	if (this->state == BRANCH_EXPERIMENT_STATE_SUCCESS) {
		double branch_weight = (double)this->branch_count / (double)this->branch_possible;
		if (branch_weight > 0.99) {
			new_pass_through(input_scope_depths_mappings,
							 output_scope_depths_mappings);
		} else {
			new_branch(input_scope_depths_mappings,
					   output_scope_depths_mappings);
		}

		ofstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution->save(solution_save_file);
		solution_save_file.close();

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}
}

void BranchExperiment::new_branch(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								  map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	cout << "new_branch" << endl;

	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	parent_scope->temp_states.insert(parent_scope->temp_states.end(),
		this->new_states.begin(), this->new_states.end());
	this->new_states.clear();
	parent_scope->temp_state_nodes.insert(parent_scope->temp_state_nodes.end(),
		this->new_state_nodes.begin(), this->new_state_nodes.end());
	this->new_state_nodes.clear();
	parent_scope->temp_state_scope_contexts.insert(parent_scope->temp_state_scope_contexts.end(),
		this->new_state_scope_contexts.begin(), this->new_state_scope_contexts.end());
	this->new_state_scope_contexts.clear();
	parent_scope->temp_state_node_contexts.insert(parent_scope->temp_state_node_contexts.end(),
		this->new_state_node_contexts.begin(), this->new_state_node_contexts.end());
	this->new_state_node_contexts.clear();
	parent_scope->temp_state_obs_indexes.insert(parent_scope->temp_state_obs_indexes.end(),
		this->new_state_obs_indexes.begin(), this->new_state_obs_indexes.end());
	this->new_state_obs_indexes.clear();
	parent_scope->temp_state_new_local_indexes.insert(parent_scope->temp_state_new_local_indexes.end(),
		this->new_states.size(), -1);

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

	new_branch_node->branch_is_pass_through = false;

	new_branch_node->original_score_mod = this->existing_average_score;
	new_branch_node->branch_score_mod = this->new_average_score;

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

		new_branch_node->original_next_node = action_node->next_node;

		action_node->next_node = new_branch_node;
	} else {
		ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node = scope_node->next_node;

		scope_node->next_node = new_branch_node;
	}

	if (this->best_step_types.size() == 0) {
		new_branch_node->branch_next_node = new_exit_node;
	} else if (this->best_step_types[0] == STEP_TYPE_ACTION) {
		new_branch_node->branch_next_node = this->best_actions[0];
	} else {
		new_branch_node->branch_next_node = this->best_sequences[0]->scope_node_placeholder;
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
				next_node = this->best_sequences[s_index+1]->scope_node_placeholder;
			}
		}

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

			this->best_actions[s_index]->next_node = next_node;
		} else {
			finalize_sequence(this->scope_context,
							  this->node_context,
							  this->best_sequences[s_index],
							  input_scope_depths_mappings,
							  output_scope_depths_mappings);
			ScopeNode* new_sequence_scope_node = this->best_sequences[s_index]->scope_node_placeholder;
			this->best_sequences[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_sequence_scope_node->id] = new_sequence_scope_node;

			new_sequence_scope_node->next_node = next_node;

			delete this->best_sequences[s_index];

			containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node = this->best_exit_node;
}

void BranchExperiment::new_pass_through(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
										map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	cout << "new_pass_through" << endl;

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

	if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node = action_node->next_node;

		action_node->next_node = new_branch_node;
	} else {
		ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node = scope_node->next_node;

		scope_node->next_node = new_branch_node;
	}

	if (this->best_step_types.size() == 0) {
		new_branch_node->branch_next_node = new_exit_node;
	} else if (this->best_step_types[0] == STEP_TYPE_ACTION) {
		new_branch_node->branch_next_node = this->best_actions[0];
	} else {
		new_branch_node->branch_next_node = this->best_sequences[0]->scope_node_placeholder;
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
				next_node = this->best_sequences[s_index+1]->scope_node_placeholder;
			}
		}

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

			this->best_actions[s_index]->next_node = next_node;
		} else {
			finalize_sequence(this->scope_context,
							  this->node_context,
							  this->best_sequences[s_index],
							  input_scope_depths_mappings,
							  output_scope_depths_mappings);
			ScopeNode* new_sequence_scope_node = this->best_sequences[s_index]->scope_node_placeholder;
			this->best_sequences[s_index]->scope_node_placeholder = NULL;
			containing_scope->nodes[new_sequence_scope_node->id] = new_sequence_scope_node;

			new_sequence_scope_node->next_node = next_node;

			delete this->best_sequences[s_index];

			containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node = this->best_exit_node;

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}
	this->new_states.clear();
}
