#include "experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
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

void Experiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->explore_experiment->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);
		
		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void Experiment::train_new_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

		this->obs_histories.push_back(obs);

		this->explore_experiment->existing_network->activate(obs);
		history->existing_predicted_scores.push_back(
			this->explore_experiment->existing_network->output->acti_vals[0]);

		history->stack_traces.push_back(wrapper->scope_histories);
	}

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::train_new_exit_step(SolutionWrapper* wrapper,
									 ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int s_index = 0; s_index < (int)history->stack_traces.size(); s_index++) {
			double sum_vals = target_val - wrapper->solution->curr_score;
			int sum_counts = 1;

			for (int l_index = 0; l_index < (int)history->stack_traces[s_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[s_index][l_index];
				Scope* scope = scope_history->scope;

				if (scope->consistency_network != NULL) {
					if (!scope_history->signal_initialized) {
						scope->pre_network->activate(scope_history->pre_obs);
						scope_history->pre_val = scope->pre_network->output->acti_vals[0];

						vector<double> inputs = scope_history->pre_obs;
						inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

						scope->post_network->activate(inputs);
						scope_history->post_val = scope->post_network->output->acti_vals[0];

						scope_history->signal_initialized = true;
					}

					sum_vals += (scope_history->post_val - scope_history->pre_val);
					sum_counts++;
				}

				if (this->state_iter < EXPERIMENT_SIGNAL_SAMPLES) {
					double average_val = sum_vals / sum_counts;

					if ((int)scope->signal_pre_obs.size() < MAX_SAMPLES) {
						scope->signal_pre_obs.push_back(scope_history->pre_obs);
						scope->signal_post_obs.push_back(scope_history->post_obs);
						scope->signal_target_vals.push_back(average_val);
					} else {
						scope->signal_pre_obs[scope->signal_index] = scope_history->pre_obs;
						scope->signal_post_obs[scope->signal_index] = scope_history->post_obs;
						scope->signal_target_vals[scope->signal_index] = average_val;
					}
					scope->signal_index++;
					if (scope->signal_index >= MAX_SAMPLES) {
						scope->signal_index = 0;
					}
				}
			}

			double average_val = sum_vals / sum_counts;
			this->target_val_histories.push_back(average_val - history->existing_predicted_scores[s_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			this->new_val_network = new Network(this->obs_histories[0].size(),
												NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->new_val_network->activate(this->obs_histories[rand_index]);

				double error = this->target_val_histories[rand_index] - this->new_val_network->output->acti_vals[0];

				this->new_val_network->backprop(error);
			}

			vector<double> network_outputs(this->obs_histories.size());
			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				this->new_val_network->activate(this->obs_histories[h_index]);

				network_outputs[h_index] = this->new_val_network->output->acti_vals[0];
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
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = this->scope_context;

						this->new_nodes.push_back(new_action_node);
					} else {
						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = this->scope_context;
						new_scope_node->id = this->scope_context->node_counter + s_index;

						new_scope_node->scope = this->scopes[s_index];

						this->new_nodes.push_back(new_scope_node);
					}
				}

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->sum_scores = 0.0;

				this->state = EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
