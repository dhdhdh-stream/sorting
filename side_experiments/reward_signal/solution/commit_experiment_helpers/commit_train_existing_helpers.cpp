// #include "commit_experiment.h"

// #include <cmath>
// #include <iostream>

// #include "action_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "network.h"
// #include "new_scope_experiment.h"
// #include "obs_node.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution_helpers.h"
// #include "solution_wrapper.h"

// using namespace std;

// #if defined(MDEBUG) && MDEBUG
// const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
// #else
// const int TRAIN_EXISTING_NUM_DATAPOINTS = 100;
// #endif /* MDEBUG */

// void CommitExperiment::commit_train_existing_check_activate(
// 		SolutionWrapper* wrapper) {
// 	CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
// 	new_experiment_state->is_save = false;
// 	new_experiment_state->step_index = 0;
// 	wrapper->experiment_context.back() = new_experiment_state;
// }

// void CommitExperiment::commit_train_existing_step(
// 		vector<double>& obs,
// 		int& action,
// 		bool& is_next,
// 		SolutionWrapper* wrapper,
// 		CommitExperimentState* experiment_state) {
// 	if (experiment_state->is_save) {
// 		if (experiment_state->step_index >= (int)this->save_step_types.size()) {
// 			wrapper->node_context.back() = this->save_exit_next_node;

// 			delete experiment_state;
// 			wrapper->experiment_context.back() = NULL;
// 		} else {
// 			if (this->save_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
// 				action = this->save_actions[experiment_state->step_index];
// 				is_next = true;

// 				wrapper->num_actions++;

// 				experiment_state->step_index++;
// 			} else {
// 				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[experiment_state->step_index]);
// 				wrapper->scope_histories.push_back(inner_scope_history);
// 				wrapper->node_context.push_back(this->save_scopes[experiment_state->step_index]->nodes[0]);
// 				wrapper->experiment_context.push_back(NULL);
// 			}
// 		}
// 	} else {
// 		if (experiment_state->step_index == this->step_iter) {
// 			this->num_instances_until_target--;
// 			if (this->num_instances_until_target <= 0) {
// 				this->scope_histories.push_back(new ScopeHistory(wrapper->scope_histories.back()));

// 				uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
// 				this->num_instances_until_target = 1 + until_distribution(generator);
// 			} else {
// 				experiment_state->is_save = true;
// 				experiment_state->step_index = 0;
// 				return;
// 			}
// 		}

// 		if (experiment_state->step_index >= (int)this->new_nodes.size()) {
// 			wrapper->node_context.back() = this->best_exit_next_node;

// 			delete experiment_state;
// 			wrapper->experiment_context.back() = NULL;
// 		} else {
// 			switch (this->new_nodes[experiment_state->step_index]->type) {
// 			case NODE_TYPE_ACTION:
// 				{
// 					ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

// 					action = node->action;
// 					is_next = true;

// 					wrapper->num_actions++;

// 					experiment_state->step_index++;
// 				}
// 				break;
// 			case NODE_TYPE_SCOPE:
// 				{
// 					ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

// 					ScopeHistory* scope_history = wrapper->scope_histories.back();

// 					ScopeNodeHistory* history = new ScopeNodeHistory(node);
// 					history->index = (int)scope_history->node_histories.size();
// 					scope_history->node_histories[node->id] = history;

// 					ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
// 					history->scope_history = inner_scope_history;
// 					wrapper->scope_histories.push_back(inner_scope_history);
// 					wrapper->node_context.push_back(node->scope->nodes[0]);
// 					wrapper->experiment_context.push_back(NULL);
// 				}
// 				break;
// 			case NODE_TYPE_OBS:
// 				{
// 					ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

// 					ScopeHistory* scope_history = wrapper->scope_histories.back();

// 					ObsNodeHistory* history = new ObsNodeHistory(node);
// 					history->index = (int)scope_history->node_histories.size();
// 					scope_history->node_histories[node->id] = history;

// 					history->obs_history = obs;

// 					experiment_state->step_index++;
// 				}
// 				break;
// 			}
// 		}
// 	}
// }

// void CommitExperiment::commit_train_existing_exit_step(
// 		SolutionWrapper* wrapper,
// 		CommitExperimentState* experiment_state) {
// 	if (experiment_state->is_save) {
// 		if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
// 			this->save_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
// 		}

// 		delete wrapper->scope_histories.back();

// 		wrapper->scope_histories.pop_back();
// 		wrapper->node_context.pop_back();
// 		wrapper->experiment_context.pop_back();

// 		experiment_state->step_index++;
// 	} else {
// 		ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

// 		if (node->scope->new_scope_experiment != NULL) {
// 			node->scope->new_scope_experiment->back_activate(wrapper);
// 		}

// 		wrapper->scope_histories.pop_back();
// 		wrapper->node_context.pop_back();
// 		wrapper->experiment_context.pop_back();

// 		experiment_state->step_index++;
// 	}
// }

// void CommitExperiment::commit_train_existing_backprop(
// 		double target_val,
// 		CommitExperimentHistory* history) {
// 	if (history->is_hit) {
// 		while (this->i_target_val_histories.size() < this->scope_histories.size()) {
// 			this->i_target_val_histories.push_back(target_val);
// 		}

// 		this->state_iter++;
// 		if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS
// 				&& (int)this->i_target_val_histories.size() >= TRAIN_EXISTING_NUM_DATAPOINTS) {
// 			double average_score;
// 			vector<Input> factor_inputs;
// 			vector<double> factor_input_averages;
// 			vector<double> factor_input_standard_deviations;
// 			vector<double> factor_weights;
// 			vector<Input> network_inputs;
// 			Network* network = NULL;
// 			double select_percentage;
// 			bool is_success = train_new(this->scope_histories,
// 										this->i_target_val_histories,
// 										average_score,
// 										factor_inputs,
// 										factor_input_averages,
// 										factor_input_standard_deviations,
// 										factor_weights,
// 										network_inputs,
// 										network,
// 										select_percentage);

// 			for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
// 				delete this->scope_histories[h_index];
// 			}
// 			this->scope_histories.clear();
// 			this->i_target_val_histories.clear();

// 			if (is_success) {
// 				this->commit_existing_average_score = average_score;
// 				this->commit_existing_inputs = factor_inputs;
// 				this->commit_existing_input_averages = factor_input_averages;
// 				this->commit_existing_input_standard_deviations = factor_input_standard_deviations;
// 				this->commit_existing_weights = factor_weights;
// 				this->commit_existing_network_inputs = network_inputs;
// 				this->commit_existing_network = network;

// 				this->state = COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW;
// 				this->state_iter = 0;
// 			} else {
// 				if (network != NULL) {
// 					delete network;
// 				}

// 				this->result = EXPERIMENT_RESULT_FAIL;
// 			}
// 		}
// 	}
// }
