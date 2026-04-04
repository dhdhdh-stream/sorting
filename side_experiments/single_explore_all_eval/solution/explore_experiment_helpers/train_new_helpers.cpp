#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_experiment.h"
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
const int TRAIN_NEW_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

void ExploreExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;

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

void ExploreExperiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	this->total_count++;

	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;
	if (history->existing_predicted_trues.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
			Network* new_network = new Network(this->new_obs_histories[0].size(),
											   NETWORK_SIZE_SMALL);

			uniform_int_distribution<int> distribution(0, this->new_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = distribution(generator);

				new_network->activate(this->new_obs_histories[rand_index]);

				double error = this->new_true_histories[rand_index] - new_network->output->acti_vals[0];

				new_network->backprop(error);
			}

			int num_positive = 0;
			for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
				new_network->activate(this->new_obs_histories[h_index]);
				if (new_network->output->acti_vals[0] >= 0.0) {
					num_positive++;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (num_positive > 0 || rand()%3 != 0) {
			#else
			if (num_positive > 0) {
			#endif /* MDEBUG */
				this->new_obs_histories.clear();
				this->new_true_histories.clear();

				this->new_networks.push_back(new_network);

				EvalExperiment* new_eval_experiment = new EvalExperiment(wrapper);

				new_eval_experiment->scope_context = this->scope_context;
				new_eval_experiment->node_context = this->node_context;
				new_eval_experiment->is_branch = this->is_branch;
				new_eval_experiment->exit_next_node = this->exit_next_node;

				new_eval_experiment->best_new_scope = this->best_new_scope;
				this->best_new_scope = NULL;
				new_eval_experiment->best_step_types = this->best_step_types;
				new_eval_experiment->best_actions = this->best_actions;
				new_eval_experiment->best_scopes = this->best_scopes;

				new_eval_experiment->new_networks = this->new_networks;
				this->new_networks.clear();

				wrapper->curr_explore_experiment = NULL;
				this->node_context->experiment = new_eval_experiment;
				delete this;
			} else {
				delete new_network;

				wrapper->curr_explore_experiment = NULL;
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
