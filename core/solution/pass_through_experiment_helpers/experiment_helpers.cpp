#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

const int NUM_EXPERIMENTS = 20;

void PassThroughExperiment::experiment_activate(AbstractNode*& curr_node,
												Problem& problem,
												vector<ContextLayer>& context,
												int& exit_depth,
												AbstractNode*& exit_node,
												RunHelper& run_helper,
												AbstractExperimentHistory*& history) {
	PassThroughExperimentInstanceHistory* instance_history = new PassThroughExperimentInstanceHistory(this);
	history = instance_history;

	for (int s_index = 0; s_index < this->branch_experiment_step_index+1; s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			instance_history->pre_step_histories.push_back(action_node_history);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
		} else {
			if (this->branch_experiment->state == BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY) {
				instance_history->pre_step_histories.push_back(NULL);
				this->best_potential_scopes[s_index]->capture_verify_activate(
					problem,
					context,
					run_helper);
			} else {
				PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
				instance_history->pre_step_histories.push_back(potential_scope_node_history);
				this->best_potential_scopes[s_index]->activate(problem,
															   context,
															   run_helper,
															   potential_scope_node_history);
			}
		}
	}

	if (this->branch_experiment_step_index == (int)this->best_step_types.size()-1) {
		if (this->best_exit_depth == 0) {
			curr_node = this->best_exit_node;
		} else {
			curr_node = NULL;

			exit_depth = this->best_exit_depth-1;
			exit_node = this->best_exit_node;
		}
	} else {
		if (this->best_step_types[this->branch_experiment_step_index+1] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[this->branch_experiment_step_index+1];
		} else {
			curr_node = this->best_potential_scopes[this->branch_experiment_step_index+1]->scope_node_placeholder;
		}
	}

	this->branch_experiment->activate(curr_node,
									  problem,
									  context,
									  exit_depth,
									  exit_node,
									  run_helper,
									  instance_history->branch_experiment_history);

	if (exit_depth == -1) {
		map<AbstractNode*, int>::iterator it = this->node_to_step_index.find(curr_node);
		if (it != this->node_to_step_index.end()) {
			for (int s_index = it->second; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
					instance_history->post_step_histories.push_back(action_node_history);
					this->best_actions[s_index]->activate(
						curr_node,
						problem,
						context,
						exit_depth,
						exit_node,
						run_helper,
						action_node_history);
				} else {
					if (this->branch_experiment->state == BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY) {
						instance_history->post_step_histories.push_back(NULL);
						this->best_potential_scopes[s_index]->capture_verify_activate(
							problem,
							context,
							run_helper);
					} else {
						PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
						instance_history->post_step_histories.push_back(potential_scope_node_history);
						this->best_potential_scopes[s_index]->activate(problem,
																	   context,
																	   run_helper,
																	   potential_scope_node_history);
					}
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
	}
}

void PassThroughExperiment::experiment_backprop(
		double target_val,
		RunHelper& run_helper,
		PassThroughExperimentOverallHistory* history) {
	if (history->branch_experiment_history != NULL) {
		this->branch_experiment->backprop(target_val,
										  run_helper,
										  history->branch_experiment_history);

		if (this->branch_experiment->state == BRANCH_EXPERIMENT_STATE_SUCCESS) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					this->best_potential_scopes[s_index]->scope_node_placeholder->verify_key = this->branch_experiment;
				}
			}

			cout << "PassThrough experiment success" << endl;
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
				new_branch_node->branch_next_node_id = this->best_potential_scopes[0]->scope_node_placeholder->id;
				new_branch_node->branch_next_node = this->best_potential_scopes[0]->scope_node_placeholder;
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
						next_node = this->best_potential_scopes[s_index+1]->scope_node_placeholder;
					}
				}

				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

					this->best_actions[s_index]->next_node_id = next_node->id;
					this->best_actions[s_index]->next_node = next_node;
				} else {
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
				}
			}
			this->best_actions.clear();
			this->best_potential_scopes.clear();

			new_exit_node->exit_depth = this->best_exit_depth;
			new_exit_node->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth];
			if (this->best_exit_node == NULL) {
				new_exit_node->exit_node_id = -1;
			} else {
				new_exit_node->exit_node_id = this->best_exit_node->id;
			}
			new_exit_node->exit_node = this->best_exit_node;

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
						delete this->best_potential_scopes[s_index];
					}
				}
				this->best_actions.clear();
				this->best_potential_scopes.clear();

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
				if (this->best_step_types[this->branch_experiment_step_index] == STEP_TYPE_ACTION) {
					this->branch_experiment->node_context.back() = this->best_actions[this->branch_experiment_step_index]->id;
				} else {
					this->branch_experiment->node_context.back() = this->best_potential_scopes[this->branch_experiment_step_index]->scope_node_placeholder->id;
				}
				this->branch_experiment->parent_pass_through_experiment = this;
			}
		}
	}
}
