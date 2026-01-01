#include "chase_experiment.h"

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
#include "tunnel.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void ChaseExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	bool is_tunnel = false;
	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		if (wrapper->curr_tunnel_parent == wrapper->scope_histories[l_index]->scope) {
			is_tunnel = true;
			break;
		}
	}
	if (is_tunnel) {
		ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
		history->tunnel_is_hit = true;

		this->num_instances_until_target--;
		if (this->num_instances_until_target <= 0) {
			history->stack_traces.push_back(wrapper->scope_histories);

			uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			ChaseExperimentState* new_experiment_state = new ChaseExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ChaseExperiment::train_new_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper) {
	ChaseExperimentState* experiment_state = (ChaseExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;

		this->obs_histories.push_back(obs);

		this->existing_true_network->activate(obs);
		history->existing_predicted_trues.push_back(
			this->existing_true_network->output->acti_vals[0]);

		this->existing_signal_network->activate(obs);
		history->existing_predicted_signals.push_back(
			this->existing_signal_network->output->acti_vals[0]);
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
			ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[experiment_state->step_index];

			ScopeHistory* scope_history = wrapper->scope_histories.back();

			ScopeNodeHistory* history = new ScopeNodeHistory(scope_node);
			scope_history->node_histories[scope_node->id] = history;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ChaseExperiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ChaseExperimentState* experiment_state = (ChaseExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	vector<double> obs = wrapper->problem->get_observations();
	wrapper->scope_histories.back()->obs_history = obs;

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ChaseExperiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
	if (history->tunnel_is_hit) {
		for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
			this->true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);

			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[i_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope == wrapper->curr_tunnel_parent) {
					if (!scope_history->tunnel_is_init[wrapper->curr_tunnel_index]) {
						scope_history->tunnel_is_init[wrapper->curr_tunnel_index] = true;
						Tunnel* tunnel = scope->tunnels[wrapper->curr_tunnel_index];
						scope_history->tunnel_vals[wrapper->curr_tunnel_index] = tunnel->get_signal(scope_history->obs_history);
					}
					this->signal_histories.push_back(scope_history->tunnel_vals[wrapper->curr_tunnel_index]
						- history->existing_predicted_signals[i_index]);

					break;
				}
			}
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->true_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			this->new_signal_network = new Network(this->obs_histories[0].size(),
												   NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->new_signal_network->activate(this->obs_histories[rand_index]);

				double error = this->signal_histories[rand_index] - this->new_signal_network->output->acti_vals[0];

				this->new_signal_network->backprop(error);
			}

			int num_positive = 0;
			double sum_predicted_true_improvement = 0.0;
			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				this->new_signal_network->activate(this->obs_histories[h_index]);
				if (this->new_signal_network->output->acti_vals[0] >= 0.0) {
					num_positive++;

					sum_predicted_true_improvement += this->true_histories[h_index];
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if ((num_positive > 0 && sum_predicted_true_improvement >= 0.0) || rand()%4 != 0) {
			#else
			if (num_positive > 0 && sum_predicted_true_improvement >= 0.0) {
			#endif /* MDEBUG */
				this->sum_true = 0.0;
				this->hit_count = 0;
				this->sum_signal = 0.0;
				this->tunnel_hit_count = 0;

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->state = CHASE_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
