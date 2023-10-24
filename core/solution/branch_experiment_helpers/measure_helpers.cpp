#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "helpers.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::measure_pass_through_activate(
		int& curr_node_id,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		int& exit_node_id,
		RunHelper& run_helper) {
	if (this->recursion_protection) {
		context.back().added_recursion_protection_flags.push_back(this);
		run_helper.recursion_protection_flags.insert(this);
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// leave context.back().node_id as -1

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->branch_experiment_activate(
				problem,
				context,
				action_node_history);
			delete action_node_history;
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::measure_pass_through_backprop(double target_val,
													 BranchExperimentHistory* history) {
	{
		ScopeHistory* parent_scope_history = history->parent_scope_history;

		double predicted_score = this->new_average_score;

		for (map<int, StateStatus>::iterator it = parent_scope_history->input_state_snapshots.begin();
				it != parent_scope_history->input_state_snapshots.end(); it++) {
			if (it->first < (int)this->new_ending_input_state_weights.size()) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += this->new_ending_input_state_weights[it->first] * normalized;
				} else {
					predicted_score += this->new_ending_input_state_weights[it->first] * it->second.val;
				}
			}
		}

		for (map<int, StateStatus>::iterator it = parent_scope_history->local_state_snapshots.begin();
				it != parent_scope_history->local_state_snapshots.end(); it++) {
			if (it->first < (int)this->new_ending_local_state_weights.size()) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += this->new_ending_local_state_weights[it->first] * normalized;
				} else {
					predicted_score += this->new_ending_local_state_weights[it->first] * it->second.val;
				}
			}
		}

		for (map<int, StateStatus>::iterator it = parent_scope_history->experiment_state_snapshots.begin();
				it != parent_scope_history->experiment_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			predicted_score += new_state_weights[it->first] * normalized;
		}

		double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
		this->pass_through_misguess += curr_misguess;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		eval();
	}
}

void BranchExperiment::eval() {
	cout << "this->scope_context:" << endl;
	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		cout << c_index << ": " << this->scope_context[c_index] << endl;
	}
	cout << "this->node_context:" << endl;
	for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
		cout << c_index << ": " << this->node_context[c_index] << endl;
	}
	cout << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			cout << " " << this->best_actions[s_index]->action.to_string();
		} else {
			cout << " S";
		}
	}
	cout << endl;

	cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
	cout << "this->best_exit_node_id: " << this->best_exit_node_id << endl;

	Scope* parent = solution->scopes[this->scope_context[0]];

	double score_standard_deviation = sqrt(parent->score_variance);

	double existing_selected_average_score = this->existing_selected_sum_score / this->existing_selected_count;

	cout << "existing_selected_average_score: " << existing_selected_average_score << endl;
	cout << "this->existing_selected_count: " << this->existing_selected_count << endl;

	double combined_average_score = this->combined_score / MEASURE_ITERS;

	double combined_improvement = combined_average_score - existing_selected_average_score;
	double combined_improvement_t_score = combined_improvement
		/ (score_standard_deviation / sqrt(MEASURE_ITERS));

	cout << "score_standard_deviation: " << score_standard_deviation << endl;
	cout << "combined_average_score: " << combined_average_score << endl;
	cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

	double branch_weight = (double)this->branch_count / (double)this->branch_possible;
	cout << "branch_weight: " << branch_weight << endl;

	double pass_through_average_misguess = this->pass_through_misguess / MEASURE_ITERS;
	double pass_through_average_score = this->pass_through_score / this->pass_through_selected_count;
	double pass_through_improvement = pass_through_average_score - existing_selected_average_score;
	double pass_through_improvement_t_score = pass_through_improvement
		/ (score_standard_deviation / sqrt(max(this->existing_selected_count, this->pass_through_selected_count)));

	cout << "pass_through_average_score: " << pass_through_average_score << endl;
	cout << "this->pass_through_selected_count: " << this->pass_through_selected_count << endl;
	cout << "pass_through_improvement_t_score: " << pass_through_improvement_t_score << endl;

	double misguess_improvement = this->existing_average_misguess - pass_through_average_misguess;
	double misguess_standard_deviation = sqrt(parent->misguess_variance);
	double misguess_improvement_t_score = misguess_improvement
		/ (misguess_standard_deviation / sqrt(NUM_DATAPOINTS));

	cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
	cout << "pass_through_average_misguess: " << pass_through_average_misguess << endl;
	cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
	cout << "misguess_improvement_t_score: " << misguess_improvement_t_score << endl;

	cout << "this->recursion_protection: " << this->recursion_protection << endl;
	cout << "this->need_recursion_protection: " << this->need_recursion_protection << endl;

	if (branch_weight > 0.02
			&& combined_improvement_t_score > 2.326) {	// >99%
		if (branch_weight > 0.98
				|| pass_through_improvement_t_score - combined_improvement_t_score > -0.674) {
			new_pass_through();
		} else {
			new_branch();
		}

		ofstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution->save(solution_save_file);
		solution_save_file.close();

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();

		this->state = BRANCH_EXPERIMENT_STATE_SUCCESS;
	} else {
		if (pass_through_improvement_t_score > -0.674
				&& misguess_improvement_t_score > 2.326
				&& !this->need_recursion_protection) {
			new_pass_through();

			ofstream solution_save_file;
			solution_save_file.open("saves/solution.txt");
			solution->save(solution_save_file);
			solution_save_file.close();

			ofstream display_file;
			display_file.open("../display.txt");
			solution->save_for_display(display_file);
			display_file.close();

			this->state = BRANCH_EXPERIMENT_STATE_SUCCESS;
		} else {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_sequences[s_index];
				}
			}
			this->best_actions.clear();
			this->best_sequences.clear();

			for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
				delete this->new_states[s_index];
			}
			this->new_states.clear();

			this->state = BRANCH_EXPERIMENT_STATE_FAIL;
		}
	}

	cout << endl;
}

void BranchExperiment::new_branch() {
	cout << "new_branch" << endl;

	Scope* containing_scope = solution->scopes[this->scope_context.back()];
	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = (int)containing_scope->nodes.size();
	containing_scope->nodes.push_back(new_branch_node);

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = false;

	new_branch_node->original_score_mod = this->existing_average_score;
	new_branch_node->branch_score_mod = this->new_average_score;

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

	new_branch_node->recursion_protection = this->recursion_protection && this->need_recursion_protection;
	cout << "new_branch_node->recursion_protection: " << new_branch_node->recursion_protection << endl;

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
		if (abs(this->new_starting_experiment_state_weights[s_index]) > MIN_SCORE_IMPACT*score_standard_deviation) {
			finalize_new_state(parent_scope,
							   sequence_scope_node_mappings,
							   this->new_states[s_index],
							   this->new_state_nodes[s_index],
							   this->new_state_scope_contexts[s_index],
							   this->new_state_node_contexts[s_index],
							   this->new_state_obs_indexes[s_index],
							   new_branch_node,
							   this->new_starting_experiment_state_weights[s_index]);
		} else {
			// don't add
			delete this->new_states[s_index];
		}
	}
	this->new_states.clear();
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

	new_branch_node->recursion_protection = this->recursion_protection && this->need_recursion_protection;
	cout << "new_branch_node->recursion_protection: " << new_branch_node->recursion_protection << endl;

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
}
