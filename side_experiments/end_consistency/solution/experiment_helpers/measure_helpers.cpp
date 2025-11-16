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

void Experiment::measure_result_check_activate(
		SolutionWrapper* wrapper) {
	wrapper->has_explore = true;
}

void Experiment::measure_result_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		double sum_consistency = 0.0;
		int count = 0;
		for (int h_index = 0; h_index < (int)wrapper->post_scope_histories.size(); h_index++) {
			Scope* scope = wrapper->post_scope_histories[h_index]->scope;
			if (scope->consistency_network != NULL) {
				vector<double> input = wrapper->post_scope_histories[h_index]->pre_obs;
				input.insert(input.end(), wrapper->post_scope_histories[h_index]->post_obs.begin(),
					wrapper->post_scope_histories[h_index]->post_obs.end());

				scope->consistency_network->activate(input);
				double consistency = scope->consistency_network->output->acti_vals[0];
				/**
				 * - allow to go below -1.0 to help distinguish between bad and very bad
				 *   - sigmoid not better (?)
				 */
				if (consistency >= 3.0) {
					sum_consistency += 3.0;
					count++;
				} else if (consistency <= -3.0) {
					sum_consistency += -3.0;
					count++;
				} else {
					sum_consistency += consistency;
					count++;
				}
			}
		}

		if (count == 0) {
			history->existing_consistency = 0.0;
		} else {
			history->existing_consistency = sum_consistency / (double)count;
		}

		for (int h_index = 0; h_index < (int)wrapper->post_scope_histories.size(); h_index++) {
			delete wrapper->post_scope_histories[h_index];
		}
		wrapper->post_scope_histories.clear();
		wrapper->has_explore = false;
	} else {
		this->total_count++;
		this->total_sum_scores += target_val;
	}
}

void Experiment::measure_check_activate(
		SolutionWrapper* wrapper) {
	wrapper->has_explore = true;

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
		this->new_val_network->activate(obs);

		bool decision_is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (this->new_val_network->output->acti_vals[0] >= 0.0) {
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

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::measure_exit_step(SolutionWrapper* wrapper,
								   ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::measure_backprop(double target_val,
								  SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	this->total_count++;
	this->total_sum_scores += target_val;

	double sum_consistency = 0.0;
	int count = 0;
	for (int h_index = 0; h_index < (int)wrapper->post_scope_histories.size(); h_index++) {
		Scope* scope = wrapper->post_scope_histories[h_index]->scope;
		if (scope->consistency_network != NULL) {
			vector<double> input = wrapper->post_scope_histories[h_index]->pre_obs;
			input.insert(input.end(), wrapper->post_scope_histories[h_index]->post_obs.begin(),
				wrapper->post_scope_histories[h_index]->post_obs.end());

			scope->consistency_network->activate(input);
			double consistency = scope->consistency_network->output->acti_vals[0];
			/**
			 * - allow to go below -1.0 to help distinguish between bad and very bad
			 *   - sigmoid not better (?)
			 */
			if (consistency >= 3.0) {
				sum_consistency += 3.0;
				count++;
			} else if (consistency <= -3.0) {
				sum_consistency += -3.0;
				count++;
			} else {
				sum_consistency += consistency;
				count++;
			}
		}
	}

	double new_consistency;
	if (count == 0) {
		new_consistency = 0.0;
	} else {
		new_consistency = sum_consistency / (double)count;
	}

	for (int h_index = 0; h_index < (int)wrapper->post_scope_histories.size(); h_index++) {
		delete wrapper->post_scope_histories[h_index];
	}
	wrapper->post_scope_histories.clear();
	wrapper->has_explore = false;

	this->sum_consistency += (new_consistency - history->existing_consistency);

	this->sum_scores += (target_val - wrapper->existing_result);

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		double new_score = this->sum_scores / this->state_iter;

		bool is_success;
		if (allow_break_consistency(wrapper)) {
			// if (new_score >= 0.0) {
			if (new_score > 0.0) {
				is_success = true;
			} else {
				is_success = false;
			}
		} else {
			// if (new_score >= 0.0 && this->sum_consistency >= 0.0) {
			if (new_score > 0.0 && this->sum_consistency >= 0.0) {
				is_success = true;
			} else {
				is_success = false;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%2 == 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
			double average_hits_per_run = (double)MEASURE_ITERS / (double)this->total_count;

			this->improvement = average_hits_per_run * new_score;

			cout << "branch" << endl;
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
