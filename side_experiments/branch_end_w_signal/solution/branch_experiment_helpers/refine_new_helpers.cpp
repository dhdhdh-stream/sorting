#include "branch_experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int REFINE_NEW_NUM_DATAPOINTS = 20;
#else
const int REFINE_NEW_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

const int MAX_NUM_REFINES = 2;

void BranchExperiment::refine_new_check_activate(SolutionWrapper* wrapper) {
	BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void BranchExperiment::refine_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		this->num_instances_until_target--;
		if (this->num_instances_until_target <= 0) {
			BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

			this->existing_network->activate(obs);
			history->existing_predicted_scores.push_back(this->existing_network->output->acti_vals[0]);

			this->obs_histories.push_back(obs);

			history->signal_sum_vals.push_back(0.0);
			history->signal_sum_counts.push_back(0);

			wrapper->experiment_callbacks.push_back(wrapper->branch_node_stack);

			uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);
		} else {
			this->new_network->activate(obs);

			bool decision_is_branch;
			#if defined(MDEBUG) && MDEBUG
			if (wrapper->curr_run_seed%2 == 0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
			#else
			if (this->new_network->output->acti_vals[0] >= 0.0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			#endif /* MDEBUG */

			if (!decision_is_branch) {
				delete experiment_state;
				wrapper->experiment_context.back() = NULL;
				return;
			}
		}
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

void BranchExperiment::refine_new_exit_step(SolutionWrapper* wrapper,
											BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::refine_new_backprop(double target_val,
										   SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int s_index = 0; s_index < (int)history->signal_sum_vals.size(); s_index++) {
			history->signal_sum_vals[s_index] += (target_val - wrapper->solution->curr_score);
			history->signal_sum_counts[s_index]++;

			double average_val = history->signal_sum_vals[s_index] / history->signal_sum_counts[s_index];

			this->target_val_histories.push_back(average_val - history->existing_predicted_scores[s_index]);
		}

		this->state_iter++;
		if (this->state_iter >= REFINE_NEW_NUM_DATAPOINTS
				&& (int)this->target_val_histories.size() >= REFINE_NEW_NUM_DATAPOINTS) {
			uniform_int_distribution<int> input_distribution(0, this->obs_histories.size()-1);
			for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
				int rand_index = input_distribution(generator);

				this->new_network->activate(this->obs_histories[rand_index]);

				double error = this->target_val_histories[rand_index] - this->new_network->output->acti_vals[0];

				this->new_network->backprop(error);
			}

			vector<double> network_outputs(this->obs_histories.size());
			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				this->new_network->activate(this->obs_histories[h_index]);

				network_outputs[h_index] = new_network->output->acti_vals[0];
			}

			int num_positive = 0;
			for (int i_index = 0; i_index < (int)this->obs_histories.size(); i_index++) {
				if (network_outputs[i_index] >= 0.0) {
					num_positive++;
				}
			}

			this->obs_histories.clear();
			this->target_val_histories.clear();

			#if defined(MDEBUG) && MDEBUG
			if (num_positive > 0 || rand()%4 != 0) {
			#else
			if (num_positive > 0) {
			#endif /* MDEBUG */
				this->num_refines++;
				if (this->num_refines >= MAX_NUM_REFINES) {
					this->total_count = 0;
					this->total_sum_scores = 0.0;

					this->sum_scores = 0.0;

					this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
					this->state_iter = 0;
				} else {
					this->state = BRANCH_EXPERIMENT_STATE_REFINE_EXISTING;
					this->state_iter = 0;
				}
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
