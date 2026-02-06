#include "experiment.h"

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

void Experiment::measure_check_activate(
		SolutionWrapper* wrapper) {
	wrapper->measure_decisions = true;

	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::measure_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		this->new_true_network->activate(obs);

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (this->new_true_network->output->acti_vals[0] >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		#endif /* MDEBUG */

		if (!is_branch) {
			this->original_count++;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		} else {
			this->branch_count++;
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

void Experiment::measure_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::measure_backprop(double target_val,
								  SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->sum_true += target_val;
		this->hit_count++;

		wrapper->measure_decisions = false;
	}

	if (this->hit_count >= MEASURE_ITERS) {
		double new_true = this->sum_true / this->hit_count;
		double average_hits_per_run = (double)this->hit_count / (double)this->total_count;
		this->improvement = average_hits_per_run * (new_true - this->existing_true);

		double new_decision_cost = wrapper->solution->calc_decision_cost();
		{
			double weight = (double)this->original_count
				/ (double)(this->original_count + this->branch_count);
			if (weight > 0.5) {
				weight = 1.0 - weight;
			}

			new_decision_cost += weight * (this->original_count + this->branch_count);
		}
		cout << "this->existing_decision_cost: " << this->existing_decision_cost << endl;
		cout << "new_decision_cost: " << new_decision_cost << endl;

		bool is_success = false;
		if (this->improvement >= 0.0) {
			if (wrapper->solution->last_experiment_scores.size() >= MIN_NUM_LAST_EXPERIMENT_TRACK) {
				int num_better_than = 0;
				// // temp
				// cout << "last_experiment_scores:";
				for (list<double>::iterator it = wrapper->solution->last_experiment_scores.begin();
						it != wrapper->solution->last_experiment_scores.end(); it++) {
					// // temp
					// cout << " " << *it;
					if (improvement >= *it) {
						num_better_than++;
					}
				}
				// // temp
				// cout << endl;

				int target_better_than = LAST_EXPERIMENT_BETTER_THAN_RATIO * (double)wrapper->solution->last_experiment_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->last_experiment_scores.size() >= NUM_LAST_EXPERIMENT_TRACK) {
					wrapper->solution->last_experiment_scores.pop_front();
				}
				wrapper->solution->last_experiment_scores.push_back(improvement);
			} else {
				wrapper->solution->last_experiment_scores.push_back(improvement);
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%2 == 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
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
			cout << "this->improvement: " << this->improvement << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			this->state = EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
