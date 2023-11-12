#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

// const int EXPLORE_ITERS = 500;
const int EXPLORE_ITERS = 5;

void BranchExperiment::explore_activate(AbstractNode*& curr_node,
										Problem& problem,
										vector<ContextLayer>& context,
										int& exit_depth,
										AbstractNode*& exit_node,
										RunHelper& run_helper) {
	bool is_target = false;
	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		explore_target_activate(curr_node,
								problem,
								context,
								exit_depth,
								exit_node,
								run_helper);
	}
}

void BranchExperiment::explore_target_activate(AbstractNode*& curr_node,
											   Problem& problem,
											   vector<ContextLayer>& context,
											   int& exit_depth,
											   AbstractNode*& exit_node,
											   RunHelper& run_helper) {
	double predicted_score = this->existing_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->existing_input_state_weights[c_index].find(it->first);
			if (weight_it != this->existing_input_state_weights[c_index].end()) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->existing_local_state_weights[c_index].find(it->first);
			if (weight_it != this->existing_local_state_weights[c_index].end()) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			map<State*, double>::iterator weight_it = this->existing_temp_state_weights[c_index].find(it->first);
			if (weight_it != this->existing_temp_state_weights[c_index].end()) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
	overall_history->existing_predicted_score = predicted_score;

	{
		// exit
		uniform_int_distribution<int> distribution(0, this->possible_exits.size()-1);
		int rand_index = distribution(generator);
		this->curr_exit_depth = this->possible_exits[rand_index].first;
		this->curr_exit_node = this->possible_exits[rand_index].second;
	}

	{
		// new path
		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 2);
		geometric_distribution<int> geometric_distribution(0.5);
		if (this->curr_exit_depth == 0
				&& this->curr_exit_node == curr_node) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}
	
		uniform_int_distribution<int> type_distribution(0, 1);
		uniform_int_distribution<int> action_distribution(0, 2);
		uniform_int_distribution<int> next_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (type_distribution(generator) == 0) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);
				this->curr_actions.push_back(new ActionNode());
				this->curr_actions.back()->action = Action(action_distribution(generator));
				this->curr_sequences.push_back(NULL);

				problem.perform_action(this->curr_actions.back()->action);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_SEQUENCE);
				this->curr_actions.push_back(NULL);

				Scope* containing_scope = solution->scopes[this->scope_context[0]];
				while (true) {
					if (containing_scope->child_scopes.size() > 0 && next_distribution(generator) == 0) {
						uniform_int_distribution<int> child_distribution(0, (int)containing_scope->child_scopes.size()-1);
						containing_scope = containing_scope->child_scopes[child_distribution(generator)];
					} else {
						break;
					}
				}
				Sequence* new_sequence = create_sequence(problem,
														 context,
														 (int)this->scope_context.size(),
														 containing_scope,
														 run_helper);
				this->curr_sequences.push_back(new_sequence);
			}
		}
	}

	{
		if (this->curr_exit_depth == 0) {
			curr_node = this->curr_exit_node;
		} else {
			curr_node = NULL;

			exit_depth = this->curr_exit_depth-1;
			exit_node = this->curr_exit_node;
		}
	}
}

void BranchExperiment::explore_backprop(double target_val,
										BranchExperimentOverallHistory* history) {
	if (history->has_target) {
		double curr_surprise = target_val - history->existing_predicted_score;
		// if (curr_surprise > this->best_surprise) {
		if (true) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_sequences[s_index];
				}
			}

			this->best_surprise = curr_surprise;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_sequences = this->curr_sequences;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_node = this->curr_exit_node;

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_sequences.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else {
					delete this->curr_sequences[s_index];
				}
			}

			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_sequences.clear();
		}

		this->state_iter++;
		if (this->state_iter >= EXPLORE_ITERS) {
			/**
			 * - if surprise isn't better than 0.0, don't bother
			 */
			cout << "this->best_surprise: " << this->best_surprise << endl;
			// if (this->best_surprise > 0.0) {
			if (rand()%2 == 0) {
				Scope* containing_scope = solution->scopes[this->scope_context.back()];
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = containing_scope;
						this->best_actions[s_index]->id = containing_scope->node_counter;
						containing_scope->node_counter++;
					} else {
						this->best_sequences[s_index]->scope_node_placeholder = new ScopeNode();
						this->best_sequences[s_index]->scope_node_placeholder->parent = containing_scope;
						this->best_sequences[s_index]->scope_node_placeholder->id = containing_scope->node_counter;
						containing_scope->node_counter++;

						this->best_sequences[s_index]->scope->id = solution->scope_counter;
						solution->scope_counter++;
					}
				}

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

				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE;
				this->state_iter = 0;
			} else {
				this->state = BRANCH_EXPERIMENT_STATE_FAIL;
			}
		}
	}
}
