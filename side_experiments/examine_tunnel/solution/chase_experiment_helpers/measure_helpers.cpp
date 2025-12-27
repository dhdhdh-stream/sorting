#include "chase_experiment.h"

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
#include "tunnel.h"
#include "utilities.h"

using namespace std;

void ChaseExperiment::measure_check_activate(
		SolutionWrapper* wrapper) {
	ChaseExperimentState* new_experiment_state = new ChaseExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ChaseExperiment::measure_step(vector<double>& obs,
								   int& action,
								   bool& is_next,
								   SolutionWrapper* wrapper) {
	ChaseExperimentState* experiment_state = (ChaseExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		this->new_signal_network->activate(obs);

		bool decision_is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (this->new_signal_network->output->acti_vals[0] >= 0.0) {
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

void ChaseExperiment::measure_exit_step(SolutionWrapper* wrapper) {
	ChaseExperimentState* experiment_state = (ChaseExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ChaseExperiment::measure_backprop(double target_val,
									   SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		this->sum_true += target_val;

		double signal;
		if (wrapper->tunnel == NULL) {
			signal = target_val;
		} else {
			signal = wrapper->tunnel->get_signal(wrapper);
		}
		this->sum_signal += signal;

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			double new_true = this->sum_true / this->state_iter;
			double new_signal = this->sum_signal / this->state_iter;

			#if defined(MDEBUG) && MDEBUG
			if ((new_true >= this->existing_true && new_signal >= this->existing_signal) || rand()%2 == 0) {
			#else
			if (new_true >= this->existing_true && new_signal >= this->existing_signal) {
			#endif /* MDEBUG */
				if (wrapper->tunnel != NULL) {
					wrapper->tunnel->num_tries++;
					wrapper->tunnel->num_success++;
				}

				double average_hits_per_run = (double)MEASURE_ITERS / (double)this->total_count;

				this->improvement = average_hits_per_run * (new_signal - this->existing_signal);

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

				this->state = CHASE_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				if (wrapper->tunnel != NULL) {
					wrapper->tunnel->num_tries++;
					wrapper->tunnel->num_measure_fail++;
				}

				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
