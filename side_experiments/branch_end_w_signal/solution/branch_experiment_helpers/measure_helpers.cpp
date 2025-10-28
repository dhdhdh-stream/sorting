#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::measure_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper,
		BranchExperimentHistory* history) {
	BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void BranchExperiment::measure_step(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper,
									BranchExperimentState* experiment_state) {
	if (experiment_state->step_index == 0) {
		// temp
		{
			BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

			history->signal_sum_vals.push_back(0.0);
			history->signal_sum_counts.push_back(0);

			wrapper->experiment_callbacks.push_back(wrapper->branch_node_stack);
		}

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

void BranchExperiment::measure_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::measure_backprop(double target_val,
										SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int s_index = 0; s_index < (int)history->signal_sum_vals.size(); s_index++) {
			history->signal_sum_vals[s_index] += (target_val - wrapper->solution->curr_score);
			history->signal_sum_counts[s_index]++;

			double average_val = history->signal_sum_vals[s_index] / history->signal_sum_counts[s_index];

			this->target_val_histories.push_back(average_val);
		}

		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			// double new_score = this->sum_scores / (double)this->state_iter;

			// // temp
			// cout << "this->scope_context->id: " << this->scope_context->id << endl;
			// cout << "this->node_context->id: " << this->node_context->id << endl;
			// cout << "new_score: " << new_score << endl;
			// cout << "existing_score: " << existing_score << endl;

			double sum_signal = 0.0;
			for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
				sum_signal += this->target_val_histories[h_index];
			}
			double new_signal = sum_signal / (double)this->target_val_histories.size();

			// #if defined(MDEBUG) && MDEBUG
			// if (new_score - this->existing_score >= 0.0 || rand()%2 == 0) {
			// #else
			// if (new_score - this->existing_score >= 0.0) {
			// #endif /* MDEBUG */
			#if defined(MDEBUG) && MDEBUG
			if (new_signal - this->existing_signal >= 0.0 || rand()%2 == 0) {
			#else
			if (new_signal - this->existing_signal >= 0.0) {
			#endif /* MDEBUG */
				double average_hits_per_run = (double)MEASURE_ITERS / (double)this->total_count;

				// this->improvement = average_hits_per_run * (new_score - this->existing_score);
				this->improvement = average_hits_per_run * (new_signal - this->existing_signal);

				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index];
					} else {
						cout << " E" << this->best_scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->best_exit_next_node == NULL) {
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "average_hits_per_run: " << average_hits_per_run << endl;
				// cout << "new_score: " << new_score << endl;
				cout << "new_signal: " << new_signal << endl;
				// cout << "existing_score: " << existing_score << endl;
				cout << "existing_signal: " << existing_signal << endl;
				cout << "this->improvement: " << this->improvement << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
