#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::measure_score_activate(int& curr_node_id,
												   Problem& problem,
												   vector<ContextLayer>& context,
												   int& exit_depth,
												   int& exit_node_id,
												   RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node_id
				problem,
				context,
				exit_depth,
				exit_node_id,
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
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void PassThroughExperiment::measure_score_backprop(double target_val) {
	this->new_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->new_score /= solution->curr_num_datapoints;

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
		cout << "this->best_exit_node_id: " << this->best_exit_node_id << endl;

		Scope* parent = solution->scopes[this->scope_context[0]];
		double score_standard_deviation = sqrt(parent->score_variance);

		double score_improvement = this->new_score - this->existing_score;
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		cout << "this->existing_score: " << this->existing_score << endl;
		cout << "this->new_score: " << this->new_score << endl;
		cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		if (score_improvement_t_score > 2.326) {	// >99%
			// Scope* containing_scope = solution->scopes[this->scope_context.back()];
			// Scope* parent_scope = solution->scopes[this->scope_context[0]];

			// BranchNode* new_branch_node = new BranchNode();
			// new_branch_node->id = (int)containing_scope->nodes.size();
			// containing_scope->nodes.push_back(new_branch_node);

			// new_branch_node->branch_scope_context = this->scope_context;
			// new_branch_node->branch_node_context = this->node_context;
			// new_branch_node->branch_node_context.back() = new_branch_node->id;

			// new_branch_node->branch_is_pass_through = true;

			// new_branch_node->original_score_mod = 0.0;
			// new_branch_node->branch_score_mod = 0.0;

			// if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
			// 	ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

			// 	new_branch_node->original_next_node_id = action_node->next_node_id;

			// 	action_node->next_node_id = new_branch_node->id;
			// } else if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
			// 	ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

			// 	new_branch_node->original_next_node_id = scope_node->next_node_id;

			// 	scope_node->next_node_id = new_branch_node->id;
			// } else {
			// 	BranchNode* branch_node = (BranchNode*)containing_scope->nodes[this->node_context.back()];

			// 	if (branch_node->experiment_is_branch) {
			// 		new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			// 		branch_node->branch_next_node_id = new_branch_node->id;
			// 	} else {
			// 		new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			// 		branch_node->original_next_node_id = new_branch_node->id;
			// 	}
			// }
			// new_branch_node->branch_next_node_id = (int)containing_scope->nodes.size();

			// map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
			// map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
			// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			// 		this->best_actions[s_index]->id = (int)containing_scope->nodes.size();
			// 		containing_scope->nodes.push_back(this->best_actions[s_index]);

			// 		this->best_actions[s_index]->next_node_id = (int)containing_scope->nodes.size();
			// 	} else {
			// 		ScopeNode* new_sequence_scope_node = finalize_sequence(
			// 			this->scope_context,
			// 			this->node_context,
			// 			this->best_sequences[s_index],
			// 			input_scope_depths_mappings,
			// 			output_scope_depths_mappings);
			// 		new_sequence_scope_node->id = (int)containing_scope->nodes.size();
			// 		containing_scope->nodes.push_back(new_sequence_scope_node);

			// 		new_sequence_scope_node->next_node_id = (int)containing_scope->nodes.size();

			// 		delete this->best_sequences[s_index];

			// 		containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);

			// 		sequence_scope_node_mappings[new_sequence_scope_node->inner_scope->id] = new_sequence_scope_node;
			// 	}
			// }
			// this->best_actions.clear();
			// this->best_sequences.clear();

			// ExitNode* new_exit_node = new ExitNode();

			// new_exit_node->id = (int)containing_scope->nodes.size();
			// containing_scope->nodes.push_back(new_exit_node);

			// new_exit_node->exit_depth = this->best_exit_depth;
			// new_exit_node->exit_node_id = this->best_exit_node_id;

			this->state = PASS_THROUGH_EXPERIMENT_STATE_SUCCESS;
		} else if (score_improvement_t_score > -0.674) {	// <75%

		} else {

		}

		cout << endl;
	}
}
