#include "branch_experiment.h"

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
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void BranchExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::train_new_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

		this->obs_histories.push_back(obs);

		this->existing_network->activate(obs);
		history->existing_predicted_scores.push_back(
			this->existing_network->output->acti_vals[0]);
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

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

void BranchExperiment::train_new_exit_step(SolutionWrapper* wrapper,
										   BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->target_val_histories.push_back((target_val - wrapper->solution->curr_score) - history->existing_predicted_scores[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			this->new_network = new Network(this->obs_histories[0].size(),
											NETWORK_SIZE_SMALL);
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
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = scope_context;

						new_action_node->action = this->best_actions[s_index];

						this->new_nodes.push_back(new_action_node);
					} else {
						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = scope_context;
						new_scope_node->id = scope_context->node_counter + s_index;

						new_scope_node->scope = this->best_scopes[s_index];

						this->new_nodes.push_back(new_scope_node);
					}
				}

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->sum_scores = 0.0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
