#include "commit_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

void CommitExperiment::commit_train_new_check_activate(
		SolutionWrapper* wrapper,
		CommitExperimentHistory* history) {
	CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
	new_experiment_state->is_save = false;
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void CommitExperiment::commit_train_new_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper,
		CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (experiment_state->step_index >= (int)this->save_step_types.size()) {
			wrapper->node_context.back() = this->save_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
		} else {
			if (this->save_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
				action = this->save_actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[experiment_state->step_index]);
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(this->save_scopes[experiment_state->step_index]->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
			}
		}
	} else {
		if (experiment_state->step_index >= this->step_iter) {
			ScopeHistory* scope_history = wrapper->scope_histories.back();

			double sum_vals = this->commit_existing_average_score;

			for (int i_index = 0; i_index < (int)this->commit_existing_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->commit_existing_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - this->commit_existing_input_averages[i_index]) / this->commit_existing_input_standard_deviations[i_index];
					sum_vals += this->commit_existing_weights[i_index] * normalized_val;
				}
			}

			if (this->commit_existing_network != NULL) {
				vector<double> input_vals(this->commit_existing_network_inputs.size());
				vector<bool> input_is_on(this->commit_existing_network_inputs.size());
				for (int i_index = 0; i_index < (int)this->commit_existing_network_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope_history,
									   this->commit_existing_network_inputs[i_index],
									   0,
									   val,
									   is_on);
					input_vals[i_index] = val;
					input_is_on[i_index] = is_on;
				}
				this->commit_existing_network->activate(input_vals,
														input_is_on);
				sum_vals += this->commit_existing_network->output->acti_vals[0];
			}

			CommitExperimentHistory* history = (CommitExperimentHistory*)wrapper->experiment_history;
			history->existing_predicted_scores.push_back(sum_vals);

			this->scope_histories.push_back(new ScopeHistory(wrapper->scope_histories.back()));

			experiment_state->is_save = true;
			experiment_state->step_index = 0;
		} else {
			switch (this->new_nodes[experiment_state->step_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

					action = node->action;
					is_next = true;

					wrapper->num_actions++;

					experiment_state->step_index++;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ScopeNodeHistory* history = new ScopeNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
					history->scope_history = inner_scope_history;
					wrapper->scope_histories.push_back(inner_scope_history);
					wrapper->node_context.push_back(node->scope->nodes[0]);
					wrapper->experiment_context.push_back(NULL);
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ObsNodeHistory* history = new ObsNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					history->obs_history = obs;

					history->factor_initialized = vector<bool>(node->factors.size(), false);
					history->factor_values = vector<double>(node->factors.size());

					experiment_state->step_index++;
				}
				break;
			}
		}
	}
}

void CommitExperiment::commit_train_new_exit_step(
		SolutionWrapper* wrapper,
		CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
			this->save_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
		}

		delete wrapper->scope_histories.back();

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();

		experiment_state->step_index++;
	} else {
		ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

		if (node->scope->new_scope_experiment != NULL) {
			node->scope->new_scope_experiment->back_activate(wrapper);
		}

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();

		experiment_state->step_index++;
	}
}

void CommitExperiment::commit_train_new_backprop(
		double target_val,
		CommitExperimentHistory* history) {
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->i_target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->i_target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			double average_score;
			vector<Input> factor_inputs;
			vector<double> factor_input_averages;
			vector<double> factor_input_standard_deviations;
			vector<double> factor_weights;
			vector<Input> network_inputs;
			Network* network = NULL;
			double select_percentage;
			bool is_success = train_new(this->scope_histories,
										this->i_target_val_histories,
										average_score,
										factor_inputs,
										factor_input_averages,
										factor_input_standard_deviations,
										factor_weights,
										network_inputs,
										network,
										select_percentage);

			for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
				delete this->scope_histories[h_index];
			}
			this->scope_histories.clear();
			this->i_target_val_histories.clear();

			if (is_success) {
				this->commit_new_average_score = average_score;
				this->commit_new_inputs = factor_inputs;
				this->commit_new_input_averages = factor_input_averages;
				this->commit_new_input_standard_deviations = factor_input_standard_deviations;
				this->commit_new_weights = factor_weights;
				this->commit_new_network_inputs = network_inputs;
				this->commit_new_network = network;

				this->state = COMMIT_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				if (network != NULL) {
					delete network;
				}

				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
