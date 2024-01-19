#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "solution_helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void PassThroughExperiment::verify_new_score_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
			this->best_potential_scopes[s_index]->activate(problem,
														   context,
														   run_helper,
														   potential_scope_node_history);
			delete potential_scope_node_history;
		} else {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_existing_scopes[s_index]);
			this->best_existing_scopes[s_index]->activate(problem,
														  context,
														  run_helper,
														  potential_scope_node_history);
			delete potential_scope_node_history;
		}
	}

	if (this->best_is_exit) {
		run_helper.has_exited = true;
	} else {
		if (this->best_exit_depth == 0) {
			curr_node = this->best_exit_node;
		} else {
			curr_node = NULL;

			exit_depth = this->best_exit_depth-1;
			exit_node = this->best_exit_node;
		}
	}
}

void PassThroughExperiment::verify_new_score_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.exceeded_limit) {
		this->new_exceeded_limit = true;
	}
	#endif /* MDEBUG */

	if (this->state == PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW_SCORE
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		if ((int)this->o_target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
			double sum_scores = 0.0;
			for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
				sum_scores += this->o_target_val_histories[d_index];
			}
			this->new_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			this->o_target_val_histories.clear();

			#if defined(MDEBUG) && MDEBUG
			if (!this->new_exceeded_limit && rand()%2 == 0) {
			#else
			double score_improvement = this->new_average_score - this->existing_average_score;
			double score_standard_deviation = sqrt(this->existing_score_variance);
			double score_improvement_t_score = score_improvement
				/ (score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

			if (score_improvement_t_score > 1.645) {	// >95%
			#endif /* MDEBUG */
				this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING_SCORE;
				this->state_iter = 0;
			#if defined(MDEBUG) && MDEBUG
			} else if (!this->new_exceeded_limit && rand()%2 == 0) {
			#else
			} else if (this->new_average_score >= this->existing_average_score) {
			#endif /* MDEBUG */
				this->new_is_better = false;

				this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING_SCORE;
				this->state_iter = 0;
			} else {
				cout << "PassThrough verify score 1st fail" << endl;
				this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
			}
		}
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
			double sum_scores = 0.0;
			for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
				sum_scores += this->o_target_val_histories[d_index];
			}
			this->new_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->o_target_val_histories.clear();

			double score_improvement = this->new_average_score - this->existing_average_score;
			double score_standard_deviation = sqrt(this->existing_score_variance);
			double score_improvement_t_score = score_improvement
				/ (score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

			#if defined(MDEBUG) && MDEBUG
			if (!this->new_exceeded_limit && rand()%2 == 0) {
			#else
			if (score_improvement_t_score > 1.645) {	// >95%
			#endif /* MDEBUG */
				if (this->new_is_better) {
					cout << "PassThrough" << endl;
					cout << "this->scope_context:" << endl;
					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						cout << c_index << ": " << this->scope_context[c_index] << endl;
					}
					cout << "this->node_context:" << endl;
					for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
						cout << c_index << ": " << this->node_context[c_index] << endl;
					}
					cout << "new explore path:";
					for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
						if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
							cout << " " << this->best_actions[s_index]->action.move;
						} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
							cout << " S";
						} else {
							cout << " E";
						}
					}
					cout << endl;

					cout << "this->best_is_exit: " << this->best_is_exit << endl;
					cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
					if (this->best_exit_node == NULL) {
						cout << "this->best_exit_node_id: " << -1 << endl;
					} else {
						cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
					}

					cout << "this->existing_average_score: " << this->existing_average_score << endl;
					cout << "this->new_average_score: " << this->new_average_score << endl;
					cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

					#if defined(MDEBUG) && MDEBUG
					this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
					this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

					this->state = PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY;
					this->state_iter = 0;
					#else
					score_finalize();
					#endif /* MDEBUG */
				} else {
					this->i_misguess_histories.reserve(solution->curr_num_datapoints);

					this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS;
					this->state_iter = 0;
				}
			#if defined(MDEBUG) && MDEBUG
			} else if (!this->new_exceeded_limit && rand()%2 == 0) {
			#else
			} else if (this->new_average_score >= this->existing_average_score) {
			#endif /* MDEBUG */
				this->i_misguess_histories.reserve(solution->curr_num_datapoints);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS;
				this->state_iter = 0;
			} else {
				cout << "PassThrough verify score 2nd fail" << endl;
				this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
			}
		}
	}
}
