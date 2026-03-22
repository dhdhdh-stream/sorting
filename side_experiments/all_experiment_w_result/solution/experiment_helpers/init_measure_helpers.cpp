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
const int INIT_MEASURE_NUM_DATAPOINTS = 20;
#else
const int INIT_MEASURE_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void Experiment::init_measure_check_activate(
		vector<double>& obs,
		bool& is_next,
		bool& is_done,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
	uniform_int_distribution<int> on_distribution(0, 9);
	if (on_distribution(generator) == 0) {
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
			ExperimentState* new_experiment_state = new ExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;

			double next_clean_result = clean_result_helper(wrapper);
			double diff = next_clean_result - wrapper->prev_clean_result;
			this->new_sum_scores += diff;
			this->new_count++;
			wrapper->prev_clean_result = next_clean_result;

			if (diff < 0.0) {
				wrapper->run_is_fail = true;
				is_next = true;
				is_done = true;
			}
		}
	}
}

void Experiment::init_measure_step(vector<double>& obs,
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

void Experiment::init_measure_exit_step(SolutionWrapper* wrapper,
										ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::init_measure_backprop(double target_val,
									   ExperimentHistory* history,
									   SolutionWrapper* wrapper) {
	if (this->new_count >= INIT_MEASURE_NUM_DATAPOINTS) {
		if (this->new_sum_scores > 0.0) {
			this->curr_ramp = 0;

			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			this->state = EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
		} else {
			this->node_context->experiment = NULL;
			delete this;
		}
	}
}

void Experiment::result_init_measure_step(vector<double>& obs,
										  int& action,
										  bool& is_next,
										  SolutionWrapper* wrapper,
										  ExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->result_node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->result_experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->result_num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->result_scope_histories.push_back(inner_scope_history);
			wrapper->result_node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->result_experiment_context.push_back(NULL);
		}
	}
}

void Experiment::result_init_measure_exit_step(SolutionWrapper* wrapper,
											   ExperimentState* experiment_state) {
	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	experiment_state->step_index++;
}
