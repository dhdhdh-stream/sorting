#include "outer_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void OuterExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		OuterExperimentState* new_experiment_state = new OuterExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void OuterExperiment::train_new_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper) {
	OuterExperimentState* experiment_state = (OuterExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;

		this->new_obs_histories.push_back(obs);

		this->existing_true_network->activate(obs);
		history->existing_predicted_trues.push_back(
			this->existing_true_network->output->acti_vals[0]);
	}

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

void OuterExperiment::train_new_exit_step(SolutionWrapper* wrapper) {
	OuterExperimentState* experiment_state = (OuterExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void OuterExperiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;
	if (history->existing_predicted_trues.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
			Network* new_network = new Network(this->new_obs_histories[0].size(),
											   NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> new_input_distribution(0, this->new_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = new_input_distribution(generator);

				new_network->activate(this->new_obs_histories[rand_index]);

				double error = this->new_true_histories[rand_index] - new_network->output->acti_vals[0];

				new_network->backprop(error);
			}

			vector<double> network_outputs(this->new_obs_histories.size());
			for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
				new_network->activate(this->new_obs_histories[h_index]);

				network_outputs[h_index] = new_network->output->acti_vals[0];
			}

			int num_positive = 0;
			for (int i_index = 0; i_index < (int)this->new_obs_histories.size(); i_index++) {
				if (network_outputs[i_index] >= 0.0) {
					num_positive++;
				}
			}

			// // temp
			// cout << "this->scope_context->id: " << this->scope_context->id << endl;
			// cout << "this->new_obs_histories.size(): " << this->new_obs_histories.size() << endl;
			// cout << "num_positive: " << num_positive << endl;

			#if defined(MDEBUG) && MDEBUG
			if (num_positive > BRANCH_MIN_RATIO * (double)this->new_obs_histories.size() || rand()%4 != 0) {
			#else
			if (num_positive > BRANCH_MIN_RATIO * (double)this->new_obs_histories.size()) {
			#endif /* MDEBUG */
				this->new_obs_histories.clear();
				this->new_true_histories.clear();

				this->new_networks.push_back(new_network);

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->state = OUTER_EXPERIMENT_STATE_REMEASURE_EXISTING;
				this->state_iter = 0;
			} else {
				wrapper->curr_outer_experiment = NULL;
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
