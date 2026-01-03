#include "chase_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "tunnel.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void ChaseExperiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	bool is_tunnel = false;
	int tunnel_parent_id = wrapper->candidates[wrapper->tunnel_iter].first;
	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		if (tunnel_parent_id == wrapper->scope_histories[l_index]->scope->id) {
			is_tunnel = true;
			break;
		}
	}
	if (is_tunnel) {
		ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
		history->tunnel_is_hit = true;
		history->stack_traces.push_back(wrapper->scope_histories);

		ChaseExperimentState* new_experiment_state = new ChaseExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ChaseExperiment::train_existing_step(vector<double>& obs,
										  SolutionWrapper* wrapper) {
	this->obs_histories.push_back(obs);

	this->sum_num_tunnel_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ChaseExperiment::train_existing_backprop(double target_val,
											  SolutionWrapper* wrapper) {
	ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->sum_true += target_val;
		this->hit_count++;
	}

	if (history->tunnel_is_hit) {
		double sum_vals = 0.0;
		measure_tunnel_vals_helper(wrapper->scope_histories[0],
								   wrapper->candidates[wrapper->tunnel_iter].first,
								   wrapper->candidates[wrapper->tunnel_iter].second,
								   sum_vals);
		this->sum_signal += sum_vals;
		this->tunnel_hit_count++;

		int tunnel_parent_id = wrapper->candidates[wrapper->tunnel_iter].first;
		Tunnel* tunnel = wrapper->candidates[wrapper->tunnel_iter].second;
		for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
			this->true_histories.push_back(target_val);

			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[i_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->id == tunnel_parent_id) {
					// if (!scope_history->tunnel_is_init[wrapper->curr_tunnel_index]) {
					// 	scope_history->tunnel_is_init[wrapper->curr_tunnel_index] = true;
					// 	Tunnel* tunnel = scope->tunnels[wrapper->curr_tunnel_index];
					// 	scope_history->tunnel_vals[wrapper->curr_tunnel_index] = tunnel->get_signal(scope_history->obs_history);
					// }
					// this->signal_histories.push_back(scope_history->tunnel_vals[wrapper->curr_tunnel_index]);

					double signal = tunnel->get_signal(scope_history->obs_history);
					this->signal_histories.push_back(signal);

					break;
				}
			}
		}
	}

	if (this->tunnel_hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;
		this->existing_signal = this->sum_signal / this->tunnel_hit_count;

		uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);

		this->existing_true_network = new Network(this->obs_histories[0].size(),
												  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_true_network->activate(this->obs_histories[rand_index]);

			double error = this->true_histories[rand_index] - this->existing_true_network->output->acti_vals[0];

			this->existing_true_network->backprop(error);
		}

		this->existing_signal_network = new Network(this->obs_histories[0].size(),
													NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_signal_network->activate(this->obs_histories[rand_index]);

			double error = this->signal_histories[rand_index] - this->existing_signal_network->output->acti_vals[0];

			this->existing_signal_network->backprop(error);
		}

		this->obs_histories.clear();
		this->true_histories.clear();
		this->signal_histories.clear();

		this->average_instances_per_run = (double)this->sum_num_tunnel_instances / (double)this->tunnel_hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = CHASE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
