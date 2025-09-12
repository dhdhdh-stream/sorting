#include "explore_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "eval_experiment.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

void ExploreExperiment::train_new_check_activate(
		SolutionWrapper* wrapper,
		ExploreExperimentHistory* history) {
	if (history->is_on) {
		this->num_instances_until_target--;
		if (this->num_instances_until_target <= 0) {
			ScopeHistory* scope_history = wrapper->scope_histories.back();

			double sum_vals = this->existing_average_score;
			for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->existing_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
					sum_vals += this->existing_weights[i_index] * normalized_val;
				}
			}
			history->existing_predicted_scores.push_back(sum_vals);

			ScopeHistory* scope_history_copy = new ScopeHistory(wrapper->scope_histories.back());
			scope_history_copy->num_actions_snapshot = wrapper->num_actions;
			this->scope_histories.push_back(scope_history_copy);

			int average_instances_per_run = (this->sum_num_instances + (int)this->last_num_instances.size() - 1)
				/ (int)this->last_num_instances.size();
			uniform_int_distribution<int> until_distribution(1, average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper,
									   ExploreExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ExploreExperiment::train_new_exit_step(SolutionWrapper* wrapper,
											ExploreExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (history->existing_predicted_scores.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
			this->scope_histories.insert(this->scope_histories.begin(), this->best_scope_history);
			this->best_scope_history = NULL;
			this->target_val_histories.insert(this->target_val_histories.begin(), this->best_surprise);

			double average_score;
			vector<Input> factor_inputs;
			vector<double> factor_input_averages;
			vector<double> factor_input_standard_deviations;
			vector<double> factor_weights;
			vector<Input> network_inputs;
			Network* network = NULL;
			double select_percentage;
			bool is_success = train_new(this->scope_histories,
										this->target_val_histories,
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
			this->target_val_histories.clear();

			if (is_success && select_percentage > 0.0) {
				EvalExperiment* new_eval_experiment = new EvalExperiment();

				new_eval_experiment->scope_context = this->scope_context;
				new_eval_experiment->node_context = this->node_context;
				new_eval_experiment->is_branch = this->is_branch;
				new_eval_experiment->exit_next_node = this->exit_next_node;

				new_eval_experiment->select_percentage = select_percentage;

				new_eval_experiment->new_average_score = average_score;
				new_eval_experiment->new_inputs = factor_inputs;
				new_eval_experiment->new_input_averages = factor_input_averages;
				new_eval_experiment->new_input_standard_deviations = factor_input_standard_deviations;
				new_eval_experiment->new_weights = factor_weights;
				new_eval_experiment->new_network_inputs = network_inputs;
				new_eval_experiment->new_network = network;

				new_eval_experiment->new_scope = this->best_new_scope;
				this->best_new_scope = NULL;
				new_eval_experiment->step_types = this->best_step_types;
				new_eval_experiment->actions = this->best_actions;
				new_eval_experiment->scopes = this->best_scopes;

				this->node_context->experiment = new_eval_experiment;
				delete this;
			} else {
				if (network != NULL) {
					delete network;
				}

				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
