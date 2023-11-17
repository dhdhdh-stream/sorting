#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::verify_new_score_activate(
		AbstractNode*& curr_node,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
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
		curr_node = this->best_exit_node;
	} else {
		curr_node = NULL;

		exit_depth = this->best_exit_depth-1;
		exit_node = this->best_exit_node;
	}
}

void PassThroughExperiment::verify_new_score_backprop(
		double target_val) {
	this->o_target_val_histories.push_back(target_val);

	if ((int)this->o_target_val_histories.size() >= 2 * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < 2 * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / (2 * solution->curr_num_datapoints);

		cout << "PassThrough" << endl;
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
		if (this->best_exit_node == NULL) {
			cout << "this->best_exit_node_id: " << -1 << endl;
		} else {
			cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
		}

		double score_improvement = this->new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(2 * solution->curr_num_datapoints));

		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << "this->new_average_score: " << this->new_average_score << endl;
		cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		if (score_improvement_t_score > 2.326) {	// >99%
			cout << "score success" << endl;

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
			} else {
				new_branch_node->branch_next_node_id = this->best_sequences[0]->scope_node_placeholder->id;
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

					this->best_actions[s_index]->next_node_id = next_node->id;
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

					new_sequence_scope_node->next_node_id = next_node->id;
					new_sequence_scope_node->next_node = next_node;

					delete this->best_sequences[s_index];

					containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);
				}
			}
			this->best_actions.clear();
			this->best_sequences.clear();

			new_exit_node->exit_depth = this->best_exit_depth;
			if (this->best_exit_node == NULL) {
				new_exit_node->exit_node_parent_id = -1;
				new_exit_node->exit_node_id = -1;
			} else {
				new_exit_node->exit_node_parent_id = this->best_exit_node->parent->id;
				new_exit_node->exit_node_id = this->best_exit_node->id;
			}
			new_exit_node->exit_node = this->best_exit_node;

			this->state = PASS_THROUGH_EXPERIMENT_STATE_SUCCESS;
		} else {
			// reserve at least solution->curr_num_datapoints
			this->i_misguess_histories.reserve(solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS;
			this->state_iter = 0;
		}

		this->o_target_val_histories.clear();
	}
}
