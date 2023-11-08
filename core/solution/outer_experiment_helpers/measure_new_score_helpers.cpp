#include "outer_experiment.h"

using namespace std;

void OuterExperiment::measure_new_score_activate(
		Problem& problem,
		RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	// unused
	AbstractNode* curr_node = NULL;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

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
		} else if (this->best_step_types[s_index] == STEP_TYPE_SEQUENCE) {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_root_scope_nodes[s_index]);
			this->best_root_scope_nodes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}
	}
}

void OuterExperiment::measure_new_score_backprop(double target_val) {
	this->target_val_histories.push_back(target_val);

	if ((int)this->target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / solution->curr_num_datapoints;

		this->target_val_histories.clear();

		cout << "Outer" << endl;
		cout << "new explore path:";
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				cout << " " << this->best_actions[s_index]->action.to_string();
			} else if (this->best_step_types[s_index] == STEP_TYPE_SEQUENCE) {
				cout << " S";
			} else {
				cout << " R";
			}
		}
		cout << endl;

		double score_improvement = new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << "new_average_score: " << new_average_score << endl;
		cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		if (score_improvement_t_score > 2.326) {	// >99%
			Scope* new_root_scope = new Scope();
			new_root_scope->id = solution->scope_counter;
			solution->scope_counter++;
			solution->scopes[new_root_scope->id] = new_root_scope;

			new_root_scope->num_input_states = 0;
			new_root_scope->num_local_states = 0;

			ActionNode* starting_noop_node = new ActionNode();
			starting_noop_node->parent = new_root_scope;
			starting_noop_node->id = 0;
			starting_noop_node->action = Action(ACTION_NOOP);
			new_root_scope->nodes[0] = starting_noop_node

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				AbstractNode* next_node;
				if (s_index == (int)this->best_step_types.size()-1) {
					next_node = NULL;
				} else {
					if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
						next_node = this->best_actions[s_index+1];
					} else if (this->best_step_types[s_index+1] == STEP_TYPE_SEQUENCE) {
						next_node = this->best_sequences[s_index+1]->scope_node_placeholder;
					} else {
						next_node = this->best_root_scope_nodes[s_index+1]->id;
					}
				}

				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					new_root_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

					this->best_actions[s_index]->next_node = next_node;
				} else if (this->best_step_types[s_index] == STEP_TYPE_SEQUENCE) {
					ScopeNode* new_sequence_scope_node = this->best_sequences[s_index]->scope_node_placeholder;
					this->best_sequences[s_index]->scope_node_placeholder = NULL;
					new_root_scope->nodes[new_sequence_scope_node->id] = new_sequence_scope_node;

					solution->scopes[this->best_sequences[s_index]->scope->id] = this->best_sequences[s_index]->scope;
					new_sequence_scope_node->inner_scope = this->best_sequences[s_index]->scope;
					this->best_sequences[s_index]->scope = NULL;

					new_sequence_scope_node->starting_nodes = vector<AbstractNode*>{this->best_sequences[s_index]->scope->nodes[0]};

					new_sequence_scope_node->next_node = next_node;

					delete this->best_sequences[s_index];

					new_root_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);
				} else {
					new_root_scope->nodes[this->best_root_scope_nodes[s_index]->id] = this->best_root_scope_nodes[s_index];

					this->best_root_scope_nodes[s_index]->next_node = next_node;
				}
			}
			this->best_actions.clear();
			this->best_sequences.clear();
			this->best_root_scope_nodes.clear();

			new_root_scope->node_counter = 1 + (int)this->best_step_types.size();

			solution->root = new_root_scope;
			solution->root_starting_node = starting_noop_node;

			this->state = OUTER_EXPERIMENT_STATE_SUCCESS;
		} else {
			this->best_score = 0.0;
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else if (this->best_step_types[s_index] == STEP_TYPE_SEQUENCE) {
					delete this->best_sequences[s_index];
				} else {
					delete this->best_root_scope_nodes[s_index];
				}
			}
			this->best_step_types.clear();
			this->best_actions.clear();
			this->best_sequences.clear();
			this->best_root_scope_nodes.clear();

			this->state = OUTER_EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		}
	}
}
