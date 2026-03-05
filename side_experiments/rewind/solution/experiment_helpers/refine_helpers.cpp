#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int REFINE_ITERS = 10;
#else
const int REFINE_ITERS = 4000;
#endif /* MDEBUG */

void Experiment::refine_check_activate(SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::refine_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	ScopeHistory* scope_history = wrapper->scope_histories.back();

	if (experiment_state->step_index == 0) {
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
			this->num_branch++;

			ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

			this->new_obs_histories.push_back(obs);

			this->existing_true_network->activate(obs);
			history->existing_predicted_trues.push_back(
				this->existing_true_network->output->acti_vals[0]);
		} else {
			this->num_original++;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		}
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
			ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[experiment_state->step_index];

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

void Experiment::refine_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::refine_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	if (this->num_original > 20000) {
		double branch_ratio = (double)this->num_branch / ((double)this->num_original + (double)this->num_branch);
		if (branch_ratio < 0.05) {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->existing_predicted_trues.size() > 0) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= REFINE_ITERS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_true_histories.begin(), this->new_true_histories.end(), generator_copy);
			}

			double best_val_average = numeric_limits<double>::lowest();
			Network* new_network = NULL;
			train_and_eval_helper(best_val_average,
								  new_network,
								  this->best_refine_is_binarize);

			this->new_obs_histories.clear();
			this->new_true_histories.clear();

			if (new_network != NULL) {
				this->new_networks.push_back(new_network);

				this->num_original = 0;
				this->num_branch = 0;

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->state = EXPERIMENT_STATE_REMEASURE_EXISTING;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
