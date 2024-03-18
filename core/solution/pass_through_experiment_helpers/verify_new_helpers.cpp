#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::verify_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	if (this->throw_id != -1) {
		run_helper.throw_id = -1;
	}

	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			curr_node = this->exit_node;
		} else {
			curr_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else {
			curr_node = this->best_scopes[0];
		}
	}
}

void PassThroughExperiment::verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->state == PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		#if defined(MDEBUG) && MDEBUG
		this->o_target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_improvement_t_score = score_improvement
			/ (this->existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		if (score_improvement_t_score > 1.960) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		#if defined(MDEBUG) && MDEBUG
		} else if (this->best_step_types.size() > 0
				&& rand()%2 == 0) {
		#else
		} else if (this->best_step_types.size() > 0
				&& score_improvement_t_score > -0.2) {
		#endif /* MDEBUG */
			this->new_is_better = false;

			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_improvement_t_score = score_improvement
			/ (this->existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (score_improvement_t_score > 1.960 && this->new_is_better) {
		#endif /* MDEBUG */
			cout << "PassThrough" << endl;
			cout << "this->parent_experiment: " << this->parent_experiment << endl;
			cout << "this->scope_context:" << endl;
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				cout << c_index << ": " << this->scope_context[c_index]->id << endl;
			}
			cout << "this->node_context:" << endl;
			for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				cout << c_index << ": " << this->node_context[c_index]->id << endl;
			}
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "this->throw_id: " << this->throw_id << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else {
					cout << " P";
				}
			}
			cout << endl;

			cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}
			cout << "this->best_exit_throw_id: " << this->best_exit_throw_id << endl;

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;
			cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

			if (this->parent_experiment == NULL) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				vector<AbstractExperiment*> verify_experiments;
				PassThroughExperiment* curr_experiment = this;
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						/**
						 * - don't include root
						 */
						break;
					} else {
						verify_experiments.insert(verify_experiments.begin(), curr_experiment);
						curr_experiment = curr_experiment->parent_experiment;
					}
				}

				this->root_experiment->verify_experiments = verify_experiments;

				this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

				this->root_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING;

				this->state = PASS_THROUGH_EXPERIMENT_STATE_ROOT_VERIFY;
			}
		#if defined(MDEBUG) && MDEBUG
		} else if (this->best_step_types.size() > 0
				&& rand()%2 == 0) {
		#else
		} else if (this->best_step_types.size() > 0
				&& score_improvement_t_score > -0.2) {
		#endif /* MDEBUG */
			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
