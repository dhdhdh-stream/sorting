#include "branch_experiment.h"

using namespace std;

void BranchExperiment::finalize(map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
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
	new_branch_node->id = containing_scope->node_counter;
	containing_scope->node_counter++;
	containing_scope->nodes[new_branch_node->id] = new_branch_node;

	ExitNode* new_exit_node = new ExitNode();
	new_exit_node->id = containing_scope->node_counter;
	containing_scope->node_counter++;
	containing_scope->nodes[new_exit_node->id] = new_exit_node;

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = false;

	new_branch_node->original_score_mod = this->existing_average_score;
	new_branch_node->branch_score_mod = this->new_average_score;

	// HERE

	Scope* parent = solution->scopes[this->scope_context[0]];
	double score_standard_deviation = sqrt(parent->score_variance);
	for (int s_index = 0; s_index < this->containing_scope_num_input_states; s_index++) {
		if (abs(this->existing_starting_input_state_weights[s_index] - this->new_starting_input_state_weights[s_index]) > MIN_SCORE_IMPACT*score_standard_deviation) {
			new_branch_node->decision_state_is_local.push_back(false);
			new_branch_node->decision_state_indexes.push_back(s_index);
			new_branch_node->decision_original_weights.push_back(this->existing_starting_input_state_weights[s_index]);
			new_branch_node->decision_branch_weights.push_back(this->new_starting_input_state_weights[s_index]);
		}
	}

	for (int s_index = 0; s_index < this->containing_scope_num_local_states; s_index++) {
		if (abs(this->existing_starting_local_state_weights[s_index] - this->new_starting_local_state_weights[s_index]) > MIN_SCORE_IMPACT*score_standard_deviation) {
			new_branch_node->decision_state_is_local.push_back(true);
			new_branch_node->decision_state_indexes.push_back(s_index);
			new_branch_node->decision_original_weights.push_back(this->existing_starting_local_state_weights[s_index]);
			new_branch_node->decision_branch_weights.push_back(this->new_starting_local_state_weights[s_index]);
		}
	}

	if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;

		action_node->next_node_id = new_branch_node->id;
	} else if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;

		scope_node->next_node_id = new_branch_node->id;
	} else {
		BranchNode* branch_node = (BranchNode*)containing_scope->nodes[this->node_context.back()];

		if (branch_node->experiment_is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			branch_node->branch_next_node_id = new_branch_node->id;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			branch_node->original_next_node_id = new_branch_node->id;
		}
	}
	new_branch_node->branch_next_node_id = (int)containing_scope->nodes.size();

	map<int, ScopeNode*> sequence_scope_node_mappings;
	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->id = (int)containing_scope->nodes.size();
			containing_scope->nodes.push_back(this->best_actions[s_index]);

			this->best_actions[s_index]->next_node_id = (int)containing_scope->nodes.size();
		} else {
			ScopeNode* new_sequence_scope_node = finalize_sequence(
				this->scope_context,
				this->node_context,
				this->best_sequences[s_index],
				input_scope_depths_mappings,
				output_scope_depths_mappings);
			new_sequence_scope_node->id = (int)containing_scope->nodes.size();
			containing_scope->nodes.push_back(new_sequence_scope_node);

			new_sequence_scope_node->next_node_id = (int)containing_scope->nodes.size();

			delete this->best_sequences[s_index];

			containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);

			sequence_scope_node_mappings[new_sequence_scope_node->inner_scope->id] = new_sequence_scope_node;
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node_id = this->best_exit_node_id;
}

void BranchExperiment::new_pass_through() {
	cout << "new_pass_through" << endl;

	Scope* containing_scope = solution->scopes[this->scope_context.back()];
	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = (int)containing_scope->nodes.size();
	containing_scope->nodes.push_back(new_branch_node);

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = true;

	new_branch_node->original_score_mod = 0.0;
	new_branch_node->branch_score_mod = 0.0;

	if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;

		action_node->next_node_id = new_branch_node->id;
	} else if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;

		scope_node->next_node_id = new_branch_node->id;
	} else {
		BranchNode* branch_node = (BranchNode*)containing_scope->nodes[this->node_context.back()];

		if (branch_node->experiment_is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			branch_node->branch_next_node_id = new_branch_node->id;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			branch_node->original_next_node_id = new_branch_node->id;
		}
	}
	new_branch_node->branch_next_node_id = (int)containing_scope->nodes.size();

	map<int, ScopeNode*> sequence_scope_node_mappings;
	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->id = (int)containing_scope->nodes.size();
			containing_scope->nodes.push_back(this->best_actions[s_index]);

			this->best_actions[s_index]->next_node_id = (int)containing_scope->nodes.size();
		} else {
			ScopeNode* new_sequence_scope_node = finalize_sequence(
				this->scope_context,
				this->node_context,
				this->best_sequences[s_index],
				input_scope_depths_mappings,
				output_scope_depths_mappings);
			new_sequence_scope_node->id = (int)containing_scope->nodes.size();
			containing_scope->nodes.push_back(new_sequence_scope_node);

			new_sequence_scope_node->next_node_id = (int)containing_scope->nodes.size();

			delete this->best_sequences[s_index];

			containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);

			sequence_scope_node_mappings[new_sequence_scope_node->inner_scope->id] = new_sequence_scope_node;
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	ExitNode* new_exit_node = new ExitNode();

	new_exit_node->id = (int)containing_scope->nodes.size();
	containing_scope->nodes.push_back(new_exit_node);

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node_id = this->best_exit_node_id;

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			for (int c_index = 0; c_index < (int)this->new_state_node_contexts[s_index][n_index].size()-1; c_index++) {
				if (this->new_state_node_contexts[s_index][n_index][c_index] == -1) {
					int inner_scope_id = this->new_state_scope_contexts[s_index][n_index][c_index+1];
					ScopeNode* new_sequence_scope_node = sequence_scope_node_mappings[inner_scope_id];
					this->new_state_node_contexts[s_index][n_index][c_index] = new_sequence_scope_node->id;
					break;
				}
			}

			if (this->new_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_state_nodes[s_index][n_index];
				if (this->new_state_node_contexts[s_index][n_index].back() == -1) {
					this->new_state_node_contexts[s_index][n_index].back() = action_node->id;
				}
			}
		}

		add_state(parent_scope,
				  this->new_states[s_index],
				  this->new_state_weights[s_index],
				  this->new_state_nodes[s_index],
				  this->new_state_scope_contexts[s_index],
				  this->new_state_node_contexts[s_index],
				  this->new_state_obs_indexes[s_index]);

		solution->states[this->new_states[s_index]->id] = this->new_states[s_index];
	}
	this->new_states.clear();

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}
	this->new_states.clear();
}
