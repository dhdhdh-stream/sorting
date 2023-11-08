#include "pass_through_experiment.h"

using namespace std;

const int NUM_EXPERIMENTS = 20;

void PassThroughExperiment::experiment_activate(AbstractNode*& curr_node,
												Problem& problem,
												vector<ContextLayer>& context,
												int& exit_depth,
												AbstractNode*& exit_node,
												RunHelper& run_helper,
												AbstractExperimentHistory*& history) {
	history = new PassThroughExperimentInstanceHistory(this);

	for (int s_index = 0; s_index < this->branch_experiment_step_index+1; s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			history->pre_step_histories.push_back(action_node_history);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			history->pre_step_histories.push_back(sequence_history);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
		}
	}

	this->branch_experiment->activate(curr_node,
									  problem,
									  context,
									  exit_depth,
									  exit_node,
									  run_helper,
									  history->branch_experiment_history);

	if (exit_depth == -1) {
		map<AbstractNode*, int>::iterator it = this->node_to_step_index.find(curr_node);
		if (it != this->node_to_step_index.end()) {
			for (int s_index = it->second; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
					history->post_step_histories.push_back(action_node_history);
					this->best_actions[s_index]->activate(
						curr_node,
						problem,
						context,
						exit_depth,
						exit_node,
						run_helper,
						action_node_history);
				} else {
					SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
					history->post_step_histories.push_back(sequence_history);
					this->best_sequences[s_index]->activate(problem,
															context,
															run_helper,
															sequence_history);
				}
			}

			if (this->best_exit_depth == 0) {
				curr_node = this->best_exit_node;
			} else {
				exit_depth = this->best_exit_depth-1;
				exit_node = this->best_exit_node;
			}
		}
	}
}

void PassThroughExperiment::experiment_backprop(
		double target_val,
		PassThroughExperimentOverallHistory* history) {
	if (history->branch_experiment_history != NULL) {
		this->branch_experiment->backprop(target_val,
										  history->branch_experiment_history);

		if (this->branch_experiment->state == BRANCH_EXPERIMENT_STATE_SUCCESS) {
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

			this->branch_experiment->finalize(input_scope_depths_mappings,
											  output_scope_depths_mappings);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_SUCCESS;
		} else if (this->branch_experiment->state == BRANCH_EXPERIMENT_STATE_FAIL) {
			delete this->branch_experiment;
			this->branch_experiment = NULL;

			this->state_iter++;
			if (this->state_iter >= NUM_EXPERIMENTS) {
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

				this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
			} else {
				uniform_int_distribution<int> distribution(0, (int)this->best_step_types.size()-1);
				this->branch_experiment_step_index = distribution(generator);

				this->branch_experiment = new BranchExperiment(
					this->scope_context,
					this->node_context);
				if (this->best_step_types[rand_step_index] == STEP_TYPE_ACTION) {
					this->branch_experiment->node_context.back() = this->best_actions[rand_step_index]->id;
				} else {
					this->branch_experiment->node_context.back() = this->best_sequences[rand_step_index]->scope_node_placeholder->id;
				}
				this->branch_experiment->parent_pass_through_experiment = this;
			}
		}
	}
}
