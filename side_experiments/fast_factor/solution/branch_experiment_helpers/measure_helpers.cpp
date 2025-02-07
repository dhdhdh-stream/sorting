#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void BranchExperiment::measure_activate(AbstractNode*& curr_node,
										Problem* problem,
										RunHelper& run_helper,
										ScopeHistory* scope_history) {
	if (this->select_percentage == 1.0) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
				this->best_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->best_exit_next_node;
	} else {
		double sum_vals = this->new_average_score;
		for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
			double val;
			fetch_factor_helper(run_helper,
								scope_history,
								this->new_factor_ids[f_index],
								val);
			sum_vals += this->new_factor_weights[f_index] * val;
		}

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (sum_vals >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		#endif /* MDEBUG */

		if (is_branch) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->best_actions[s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
					this->best_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
			}

			curr_node = this->best_exit_next_node;
		}
	}
}

void BranchExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
		double new_average_score = (double)this->combined_score / (double)this->state_iter;
		if (new_average_score > this->o_existing_average_score) {
			cout << "BranchExperiment" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index].move;
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

			cout << "this->new_average_score: " << this->new_average_score << endl;
			cout << "this->select_percentage: " << this->select_percentage << endl;

			cout << "new_average_score: " << new_average_score << endl;
			cout << "this->o_existing_average_score: " << this->o_existing_average_score << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			if (this->select_percentage == 1.0) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
			}
			#else
			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
