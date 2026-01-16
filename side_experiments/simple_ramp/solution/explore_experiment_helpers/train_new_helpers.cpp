#include "explore_experiment.h"

#include <iostream>

#include "constants.h"
#include "eval_experiment.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

void ExploreExperiment::train_new_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper,
		ExploreExperimentHistory* history) {
	if (history->is_on) {
		int average_instances_per_run = (this->sum_num_instances + (int)this->last_num_instances.size() - 1)
			/ (int)this->last_num_instances.size();
		if (average_instances_per_run > 4) {
			average_instances_per_run = 4;
		}
		uniform_int_distribution<int> is_new_distribution(0, average_instances_per_run-1);
		if (is_new_distribution(generator) == 0) {
			wrapper->has_explore = true;

			this->existing_network->activate(obs);
			history->existing_predicted_scores.push_back(this->existing_network->output->acti_vals[0]);

			this->obs_histories.push_back(obs);

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

void ExploreExperiment::train_new_backprop(double target_val,
										   ExploreExperimentHistory* history,
										   SolutionWrapper* wrapper) {
	if (history->existing_predicted_scores.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
			this->new_network = new Network(this->obs_histories[0].size());
			uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->new_network->activate(this->obs_histories[rand_index]);

				double error = this->target_val_histories[rand_index] - this->new_network->output->acti_vals[0];

				this->new_network->backprop(error);
			}

			vector<double> network_outputs(this->obs_histories.size());
			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				this->new_network->activate(this->obs_histories[h_index]);

				network_outputs[h_index] = this->new_network->output->acti_vals[0];
			}

			int num_positive = 0;
			for (int i_index = 0; i_index < (int)this->obs_histories.size(); i_index++) {
				if (network_outputs[i_index] >= 0.0) {
					num_positive++;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (num_positive > 0 || rand()%4 != 0) {
			#else
			if (num_positive > 0) {
			#endif /* MDEBUG */
				EvalExperiment* new_eval_experiment = new EvalExperiment();

				new_eval_experiment->node_context = this->node_context;
				new_eval_experiment->exit_next_node = this->exit_next_node;

				new_eval_experiment->new_network = this->new_network;
				this->new_network = NULL;

				new_eval_experiment->new_scope = this->best_new_scope;
				this->best_new_scope = NULL;
				new_eval_experiment->step_types = this->best_step_types;
				new_eval_experiment->actions = this->best_actions;
				new_eval_experiment->scopes = this->best_scopes;

				this->node_context->experiment = new_eval_experiment;
				delete this;
			} else {
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
