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
			ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
			history->hit_branch = true;
		} else {
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

	if (history->hit_branch) {
		this->num_branch++;
	} else {
		this->num_original++;
		if (this->num_original == BRANCH_RATIO_CHECK_ITER) {
			double branch_ratio = (double)this->num_branch / ((double)this->num_original + (double)this->num_branch);
			if (branch_ratio < BRANCH_MIN_RATIO) {
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			}
		}
	}

	if (history->hit_branch) {
		this->new_scores.push_back(target_val);

		// double existing_result = get_existing_result(wrapper);
		// this->sum_true += target_val - existing_result;
		// this->hit_count++;

		this->state_iter++;
		if (this->state_iter >= MEASURE_STEP_NUM_ITERS) {
			double existing_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_vals += this->existing_scores[h_index];
			}
			double existing_true = existing_sum_vals / (double)this->existing_scores.size();
			double new_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_vals += this->new_scores[h_index];
			}
			double new_true = new_sum_vals / (double)this->new_scores.size();
			this->local_improvement = new_true - existing_true;
			double average_hits_per_run = (double)this->new_scores.size() / (double)this->total_count;
			this->global_improvement = average_hits_per_run * this->local_improvement;
			double sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				sum_variance += (this->new_scores[h_index] - new_true) * (this->new_scores[h_index] - new_true);
			}
			this->score_standard_deviation = sqrt(sum_variance / (double)this->new_scores.size());

			double t_score = this->local_improvement
				/ (this->score_standard_deviation / sqrt((double)this->new_scores.size()));

			// // temp
			// cout << "existing_true: " << existing_true << endl;
			// cout << "new_true: " << new_true << endl;
			// cout << "this->new_scores.size(): " << this->new_scores.size() << endl;
			// cout << "average_hits_per_run: " << average_hits_per_run << endl;
			// cout << "t_score: " << t_score << endl;

			#if defined(MDEBUG) && MDEBUG
			if (t_score >= SUCCESS_T_SCORE || rand()%3 == 0) {
			#else
			if (t_score >= SUCCESS_T_SCORE) {
			#endif /* MDEBUG */
				bool is_success = false;
				if (wrapper->solution->last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = wrapper->solution->last_scores.begin();
							it != wrapper->solution->last_scores.end(); it++) {
						if (this->global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (wrapper->solution->last_scores.size() >= NUM_LAST_TRACK) {
						wrapper->solution->last_scores.pop_front();
					}
					wrapper->solution->last_scores.push_back(this->global_improvement);
				} else {
					wrapper->solution->last_scores.push_back(this->global_improvement);
				}

				if (is_success) {
					cout << "this->scope_context->id: " << this->scope_context->id << endl;
					cout << "this->node_context->id: " << this->node_context->id << endl;
					cout << "this->is_branch: " << this->is_branch << endl;
					if (this->best_new_scope != NULL) {
						if (this->best_parent_scope == this->scope_context) {
							cout << "new local scope" << endl;
						} else {
							cout << "new outer scope" << endl;
						}
					}
					cout << "new explore path:";
					for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
						if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
							cout << " " << this->best_actions[s_index];
						} else {
							if (this->best_scopes[s_index]->is_outer) {
								cout << " O" << this->best_scopes[s_index]->id;
							} else {
								cout << " E" << this->best_scopes[s_index]->id;
							}
						}
					}
					cout << endl;

					if (this->exit_next_node == NULL) {
						cout << "this->exit_next_node->id: " << -1 << endl;
					} else {
						cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
					}

					cout << "this->local_improvement: " << this->local_improvement << endl;
					cout << "average_hits_per_run: " << average_hits_per_run << endl;
					cout << "this->global_improvement: " << this->global_improvement << endl;

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
			#if defined(MDEBUG) && MDEBUG
			} else if (t_score < FAIL_T_SCORE && rand()%2 == 0) {
			#else
			} else if (t_score < FAIL_T_SCORE) {
			#endif /* MDEBUG */
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->state = EXPERIMENT_STATE_REMEASURE_EXISTING;
				this->state_iter = 0;
			}
		}
	}
}
