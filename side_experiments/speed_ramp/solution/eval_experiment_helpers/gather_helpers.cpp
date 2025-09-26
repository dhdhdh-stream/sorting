#include "eval_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void EvalExperiment::gather_backprop(double target_val,
									 EvalExperimentHistory* history,
									 SolutionWrapper* wrapper) {
	if (history->is_on) {
		for (int i_index = 0; i_index < (int)history->signal_is_set.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				this->new_signals.push_back(history->signal_vals[i_index]);
			} else {
				this->new_signals.push_back(target_val);
			}
		}
	} else {
		for (int i_index = 0; i_index < (int)history->signal_is_set.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				this->existing_signals.push_back(history->signal_vals[i_index]);
			} else {
				this->existing_signals.push_back(target_val);
			}
		}
	}

	/**
	 * - simply gather another RAMP_EPOCH_NUM_ITERS iters for now
	 */
	this->state_iter++;
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
		double existing_sum_signal = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_signals.size(); h_index++) {
			existing_sum_signal += this->existing_signals[h_index];
		}
		double existing_signal_average = existing_sum_signal / (double)this->existing_signals.size();

		double new_sum_signal = 0.0;
		for (int h_index = 0; h_index < (int)this->new_signals.size(); h_index++) {
			new_sum_signal += this->new_signals[h_index];
		}
		double new_signal_average = new_sum_signal / (double)this->new_signals.size();

		double improvement = new_signal_average - existing_signal_average;

		bool is_success;
		if (wrapper->solution->last_experiment_scores.size() >= MIN_NUM_LAST_EXPERIMENT_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = wrapper->solution->last_experiment_scores.begin();
					it != wrapper->solution->last_experiment_scores.end(); it++) {
				if (improvement >= *it) {
					num_better_than++;
				}
			}

			int target_better_than = LAST_EXPERIMENT_BETTER_THAN_RATIO * (double)wrapper->solution->last_experiment_scores.size();

			if (num_better_than >= target_better_than) {
				is_success = true;
			} else {
				is_success = false;
			}

			wrapper->solution->last_experiment_scores.pop_front();
			wrapper->solution->last_experiment_scores.push_back(improvement);
		} else {
			is_success = false;

			wrapper->solution->last_experiment_scores.push_back(improvement);
		}

		if (is_success) {
			cout << "success" << endl;
			cout << "existing_signal_average: " << existing_signal_average << endl;
			cout << "new_signal_average: " << new_signal_average << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->actions[s_index];
				} else {
					cout << " E" << this->scopes[s_index]->id;
				}
			}
			cout << endl;

			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}

			cout << "this->select_percentage: " << this->select_percentage << endl;

			cout << endl;

			this->result = EVAL_RESULT_SUCCESS;

			this->curr_ramp++;

			this->state = EVAL_EXPERIMENT_STATE_WRAPUP;
			this->state_iter = 0;
		} else {
			this->result = EVAL_RESULT_FAIL;

			this->curr_ramp--;

			this->state = EVAL_EXPERIMENT_STATE_WRAPUP;
			this->state_iter = 0;
		}

	}
}
