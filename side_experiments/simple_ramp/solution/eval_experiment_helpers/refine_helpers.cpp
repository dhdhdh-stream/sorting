#include "eval_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int REFINE_NUM_DATAPOINTS = 20;
#else
const int REFINE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void EvalExperiment::refine_check_activate(
		AbstractNode* experiment_node,
		vector<double>& obs,
		SolutionWrapper* wrapper,
		EvalExperimentHistory* history) {
	if (history->is_on) {
		bool is_branch = true;
		for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
			this->new_networks[n_index]->activate(obs);
			if (this->new_networks[n_index]->output->acti_vals[0] < 0.0) {
				is_branch = false;
				break;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		if (is_branch) {
			history->hit_branch = true;

			this->new_obs_histories.push_back(obs);

			EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	} else {
		/**
		 * - to guarantee enough samples
		 */
		if (this->existing_obs_histories.size() < 2 * this->new_obs_histories.size()) {
			this->existing_obs_histories.push_back(obs);
		}
	}
}

void EvalExperiment::refine_backprop(double target_val,
									 EvalExperimentHistory* history,
									 SolutionWrapper* wrapper) {
	if (history->is_on) {
		if (history->hit_branch) {
			this->num_branch++;
		} else {
			this->num_original++;
			if (this->num_original == BRANCH_RATIO_CHECK_ITER) {
				double branch_ratio = (double)this->num_branch / ((double)this->num_original + (double)this->num_branch);
				if (branch_ratio < BRANCH_MIN_RATIO) {
					this->node_context->experiment = NULL;
					delete this;
					return;
				}
			}
		}
	}

	while (this->existing_target_val_histories.size() < this->existing_obs_histories.size()) {
		this->existing_target_val_histories.push_back(target_val);
	}

	if (this->new_target_val_histories.size() < this->new_obs_histories.size()) {
		while (this->new_target_val_histories.size() < this->new_obs_histories.size()) {
			this->new_target_val_histories.push_back(target_val);
		}

		this->state_iter++;
		if (this->state_iter >= REFINE_NUM_DATAPOINTS) {
			Network* existing_network = new Network(this->existing_obs_histories[0].size());
			uniform_int_distribution<int> existing_distribution(0, this->existing_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = existing_distribution(generator);

				existing_network->activate(this->existing_obs_histories[rand_index]);

				double error = this->existing_target_val_histories[rand_index] - existing_network->output->acti_vals[0];

				existing_network->backprop(error);
			}

			for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
				existing_network->activate(this->new_obs_histories[h_index]);
				this->new_target_val_histories[h_index] -= existing_network->output->acti_vals[0];
			}
			delete existing_network;

			Network* new_network = new Network(this->new_obs_histories[0].size());
			uniform_int_distribution<int> new_distribution(0, this->new_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = new_distribution(generator);

				new_network->activate(this->new_obs_histories[rand_index]);

				double error = this->new_target_val_histories[rand_index] - new_network->output->acti_vals[0];

				new_network->backprop(error);
			}

			this->new_networks.push_back(new_network);

			this->num_original = 0;
			this->num_branch = 0;

			this->curr_ramp = 0;
			this->measure_status = MEASURE_STATUS_N_A;

			this->state = EVAL_EXPERIMENT_STATE_INIT;
			this->state_iter = 0;
			this->num_fail = 0;
		}
	}
}
