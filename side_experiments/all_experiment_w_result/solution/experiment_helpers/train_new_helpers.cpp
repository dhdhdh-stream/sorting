#include "experiment.h"

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

const double MIN_EXPERIMENT_DIFF_RATIO = 0.8;

void Experiment::train_new_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
	#if defined(MDEBUG) && MDEBUG
	uniform_int_distribution<int> on_distribution(0, 9);
	#else
	uniform_int_distribution<int> on_distribution(0, 99);
	#endif /* MDEBUG */
	if (on_distribution(generator) == 0) {
		this->new_obs_histories.push_back(obs);

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;

		double next_clean_result = clean_result_helper(wrapper);
		this->new_target_val_histories.push_back(next_clean_result - wrapper->prev_clean_result);
		wrapper->prev_clean_result = next_clean_result;
	}
}

void Experiment::train_new_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper,
								ExperimentState* experiment_state) {
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

void Experiment::train_new_exit_step(SolutionWrapper* wrapper,
									 ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::train_new_backprop(double target_val,
									ExperimentHistory* history,
									SolutionWrapper* wrapper) {
	if ((int)this->new_obs_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
		Network* new_network = new Network(this->new_obs_histories[0].size());
		uniform_int_distribution<int> new_input_distribution(0, this->new_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = new_input_distribution(generator);

			new_network->activate(this->new_obs_histories[rand_index]);

			double error = this->new_target_val_histories[rand_index] - new_network->output->acti_vals[0];

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

		this->new_obs_histories.clear();
		this->new_target_val_histories.clear();

		#if defined(MDEBUG) && MDEBUG
		if (num_positive > BRANCH_MIN_RATIO * (double)this->new_obs_histories.size() || rand()%4 != 0) {
		#else
		if (num_positive > BRANCH_MIN_RATIO * (double)this->new_obs_histories.size()) {
		#endif /* MDEBUG */
			this->new_networks.push_back(new_network);

			this->curr_ramp = 0;
			this->measure_status = MEASURE_STATUS_N_A;

			this->state = EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
		} else {
			delete new_network;

			this->node_context->experiment = new Experiment(this->node_context);
			delete this;
		}
	}
}
