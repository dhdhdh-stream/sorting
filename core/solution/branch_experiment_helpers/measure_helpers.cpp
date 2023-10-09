#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
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

const int MEASURE_ITERS = 10000;

/**
 * - difference between simple_activate() is that increments branch_count
 */
void BranchExperiment::measure_activate(int& curr_node_id,
										Problem& problem,
										vector<ContextLayer>& context,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper) {
	double branch_score = this->average_score;
	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	double original_score = parent_scope->average_score;

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += normalized * scale_it->second->weight;
			}
			original_score += normalized * it->first->scale->weight;
		} else {
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += it->second.val * scale_it->second->weight;
			}
			original_score += it->second.val * it->first->scale->weight;
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			branch_score += normalized * it->first->scale->weight;
		} else {
			branch_score += it->second.val * it->first->scale->weight;
		}
	}

	if (branch_score > original_score) {
		this->branch_count++;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_actions[s_index]->branch_experiment_simple_activate(
					problem);
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
}

void BranchExperiment::measure_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		eval();
	}
}

void BranchExperiment::eval() {
	cout << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			cout << " " << this->best_actions[s_index]->action.to_string();
		} else {
			cout << " S";
		}
	}
	cout << endl;

	Scope* parent = solution->scopes[this->scope_context[0]];

	double combined_average_score = this->combined_score / MEASURE_ITERS;
	double combined_improvement = combined_average_score - parent->average_score;
	double score_standard_deviation = sqrt(parent->score_variance);
	double combined_improvement_t_score = combined_improvement
		/ (score_standard_deviation / sqrt(MEASURE_ITERS));

	cout << "combined_average_score: " << combined_average_score << endl;
	cout << "combined_improvement: " << combined_improvement << endl;
	cout << "score_standard_deviation: " << score_standard_deviation << endl;
	cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

	if (combined_improvement_t_score > 2.326) {		// >99%
		double branch_weight = this->branch_count / MEASURE_ITERS;
		if (branch_weight > 0.98) {
			new_pass_through();
		} else {
			new_branch();
		}

		// ofstream solution_save_file;
		// solution_save_file.open("saves/solution.txt");
		// solution->save(solution_save_file);
		// solution_save_file.close();
	} else {
		// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

		double score_improvement = this->average_score - parent->average_score;
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(20000));

		double misguess_improvement = parent->average_misguess - this->average_misguess;
		double misguess_standard_deviation = sqrt(parent->misguess_variance);
		double misguess_improvement_t_score = misguess_improvement
			/ (misguess_standard_deviation / sqrt(20000));

		cout << "score_improvement: " << score_improvement << endl;
		cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		cout << "misguess_improvement: " << misguess_improvement << endl;
		cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
		cout << "misguess_improvement_t_score: " << misguess_improvement_t_score << endl;

		if (score_improvement_t_score > -0.674	// 75%<
				&& misguess_improvement_t_score > 2.326) {
			new_pass_through();

			// ofstream solution_save_file;
			// solution_save_file.open("saves/solution.txt");
			// solution->save(solution_save_file);
			// solution_save_file.close();
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

			for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
					it != this->score_state_scales.end(); it++) {
				delete it->second;
			}
			this->score_state_scales.clear();

			for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
				delete this->new_score_states[s_index];
			}
			this->new_score_states.clear();
		}
	}

	cout << endl;

	this->state = BRANCH_EXPERIMENT_STATE_DONE;
}

void BranchExperiment::new_branch() {
	cout << "new_branch" << endl;

	Scope* starting_scope = solution->scopes[this->scope_context.back()];
	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = (int)starting_scope->nodes.size();
	starting_scope->nodes.push_back(new_branch_node);

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = false;

	/**
	 * TODO: check if state used at decision point so can delete
	 */
	for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
			it != this->score_state_scales.end(); it++) {
		finalize_existing_state(parent_scope,
								it->first,
								new_branch_node,
								it->second->weight);

		delete it->second;
	}
	this->score_state_scales.clear();

	map<int, Sequence*> sequence_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_SEQUENCE) {
			sequence_mappings[this->best_sequences[s_index]->scope->id] = this->best_sequences[s_index];
		}
	}
	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		finalize_new_state(parent_scope,
						   sequence_mappings,
						   this->new_score_states[s_index],
						   this->new_score_state_nodes[s_index],
						   this->new_score_state_scope_contexts[s_index],
						   this->new_score_state_node_contexts[s_index],
						   this->new_score_state_obs_indexes[s_index],
						   new_branch_node);
	}
	this->new_score_states.clear();

	if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;

		action_node->next_node_id = new_branch_node->id;
	} else if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;

		scope_node->next_node_id = new_branch_node->id;
	} else {
		BranchNode* branch_node = (BranchNode*)starting_scope->nodes[this->node_context.back()];

		if (branch_node->experiment_is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			branch_node->branch_next_node_id = new_branch_node->id;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			branch_node->original_next_node_id = new_branch_node->id;
		}
	}
	new_branch_node->branch_next_node_id = (int)starting_scope->nodes.size();

	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(this->best_actions[s_index]);

			this->best_actions[s_index]->next_node_id = (int)starting_scope->nodes.size();
		} else {
			ScopeNode* new_scope_node = finalize_sequence(this->scope_context,
														  this->node_context,
														  this->best_sequences[s_index],
														  input_scope_depths_mappings,
														  output_scope_depths_mappings);
			new_scope_node->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(new_scope_node);

			new_scope_node->next_node_id = (int)starting_scope->nodes.size();

			delete this->best_sequences[s_index];

			starting_scope->child_scopes.push_back(new_scope_node->inner_scope);
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	{
		ExitNode* new_exit_node = new ExitNode();

		new_exit_node->id = (int)starting_scope->nodes.size();
		starting_scope->nodes.push_back(new_exit_node);

		new_exit_node->exit_depth = this->best_exit_depth;
		new_exit_node->exit_node_id = this->best_exit_node_id;
	}

	parent_scope->average_score = this->combined_score;
}

void BranchExperiment::new_pass_through() {
	cout << "new_pass_through" << endl;

	Scope* starting_scope = solution->scopes[this->scope_context.back()];
	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = (int)starting_scope->nodes.size();
	starting_scope->nodes.push_back(new_branch_node);

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = true;

	if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;

		action_node->next_node_id = new_branch_node->id;
	} else if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;

		scope_node->next_node_id = new_branch_node->id;
	} else {
		BranchNode* branch_node = (BranchNode*)starting_scope->nodes[this->node_context.back()];

		if (branch_node->experiment_is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			branch_node->branch_next_node_id = new_branch_node->id;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			branch_node->original_next_node_id = new_branch_node->id;
		}
	}
	new_branch_node->branch_next_node_id = (int)starting_scope->nodes.size();

	map<int, int> new_scope_node_id_mappings;

	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(this->best_actions[s_index]);

			this->best_actions[s_index]->next_node_id = (int)starting_scope->nodes.size();
		} else {
			ScopeNode* new_scope_node = finalize_sequence(this->scope_context,
														  this->node_context,
														  this->best_sequences[s_index],
														  input_scope_depths_mappings,
														  output_scope_depths_mappings);
			new_scope_node->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(new_scope_node);

			new_scope_node->next_node_id = (int)starting_scope->nodes.size();

			delete this->best_sequences[s_index];

			starting_scope->child_scopes.push_back(new_scope_node->inner_scope);

			new_scope_node_id_mappings[new_scope_node->inner_scope->id] = new_scope_node->id;
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	{
		ExitNode* new_exit_node = new ExitNode();

		new_exit_node->id = (int)starting_scope->nodes.size();
		starting_scope->nodes.push_back(new_exit_node);

		new_exit_node->exit_depth = this->best_exit_depth;
		new_exit_node->exit_node_id = this->best_exit_node_id;
	}

	for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
			it != this->score_state_scales.end(); it++) {
		delete it->first->scale;

		it->first->scale = it->second;
	}
	this->score_state_scales.clear();

	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		finalize_new_score_state(parent_scope,
								 new_scope_node_id_mappings,
								 this->new_score_states[s_index],
								 this->new_score_state_nodes[s_index],
								 this->new_score_state_scope_contexts[s_index],
								 this->new_score_state_node_contexts[s_index],
								 this->new_score_state_obs_indexes[s_index]);
	}
	this->new_score_states.clear();

	parent_scope->average_score = this->average_score;
}
