#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::experiment_verify_new_activate(
		AbstractNode*& curr_node,
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

void PassThroughExperiment::experiment_verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->root_state == ROOT_EXPERIMENT_STATE_VERIFY_1ST
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

			this->root_state = ROOT_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			/**
			 * - leave this->experiment_iter unchanged
			 */
		} else {
			bool to_experiment = false;
			if (this->verify_experiments.back()->type == EXPERIMENT_TYPE_BRANCH) {
				double combined_branch_weight = 1.0;
				AbstractExperiment* curr_experiment = this->verify_experiments.back();
				while (true) {
					if (curr_experiment == NULL) {
						break;
					}

					if (curr_experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
						combined_branch_weight *= branch_experiment->branch_weight;
					}
					curr_experiment = curr_experiment->parent_experiment;
				}

				BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
				if (branch_experiment->step_types.size() > 0
						&& branch_experiment->combined_score >= branch_experiment->verify_existing_average_score
						&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
					#if defined(MDEBUG) && MDEBUG
					for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
						delete branch_experiment->verify_problems[p_index];
					}
					branch_experiment->verify_problems.clear();
					branch_experiment->verify_seeds.clear();
					branch_experiment->verify_original_scores.clear();
					branch_experiment->verify_branch_scores.clear();
					/**
					 * - simply rely on leaf experiment to verify
					 */
					#endif /* MDEBUG */

					branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			} else {
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
				if (pass_through_experiment->best_step_types.size() > 0) {
					pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			}

			if (!to_experiment) {
				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

				curr_experiment->experiment_iter++;
				int matching_index;
				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
						matching_index = c_index;
						break;
					}
				}
				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
				this->verify_experiments.back()->finalize();
				delete this->verify_experiments.back();

				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
					* pow(0.5, this->verify_experiments.size());
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						break;
					}

					if (curr_experiment->experiment_iter >= target_count) {
						AbstractExperiment* parent = curr_experiment->parent_experiment;

						parent->experiment_iter++;
						int matching_index;
						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
							if (parent->child_experiments[c_index] == curr_experiment) {
								matching_index = c_index;
								break;
							}
						}
						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
						curr_experiment->finalize();
						delete curr_experiment;

						curr_experiment = parent;
						target_count *= 2.0;
					} else {
						break;
					}
				}
			}

			this->verify_experiments.clear();

			if (this->state_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			}
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
		if (score_improvement_t_score > 1.960) {
		#endif /* MDEBUG */
			cout << "PassThrough experiment success" << endl;
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
					cout << " E";
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

			/**
			 * - also finalize this->verify_experiments in finalize()
			 */

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			bool to_experiment = false;
			if (this->verify_experiments.back()->type == EXPERIMENT_TYPE_BRANCH) {
				double combined_branch_weight = 1.0;
				AbstractExperiment* curr_experiment = this->verify_experiments.back();
				while (true) {
					if (curr_experiment == NULL) {
						break;
					}

					if (curr_experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
						combined_branch_weight *= branch_experiment->branch_weight;
					}
					curr_experiment = curr_experiment->parent_experiment;
				}

				BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
				if (branch_experiment->step_types.size() > 0
						&& branch_experiment->combined_score >= branch_experiment->verify_existing_average_score
						&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
					#if defined(MDEBUG) && MDEBUG
					for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
						delete branch_experiment->verify_problems[p_index];
					}
					branch_experiment->verify_problems.clear();
					branch_experiment->verify_seeds.clear();
					branch_experiment->verify_original_scores.clear();
					branch_experiment->verify_branch_scores.clear();
					/**
					 * - simply rely on leaf experiment to verify
					 */
					#endif /* MDEBUG */

					branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					branch_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			} else {
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
				if (pass_through_experiment->best_step_types.size() > 0) {
					pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
					pass_through_experiment->experiment_iter = 0;

					to_experiment = true;
				}
			}

			if (!to_experiment) {
				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

				curr_experiment->experiment_iter++;
				int matching_index;
				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
						matching_index = c_index;
						break;
					}
				}
				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
				this->verify_experiments.back()->finalize();
				delete this->verify_experiments.back();

				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
					* pow(0.5, this->verify_experiments.size());
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						break;
					}

					if (curr_experiment->experiment_iter >= target_count) {
						AbstractExperiment* parent = curr_experiment->parent_experiment;

						parent->experiment_iter++;
						int matching_index;
						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
							if (parent->child_experiments[c_index] == curr_experiment) {
								matching_index = c_index;
								break;
							}
						}
						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
						curr_experiment->finalize();
						delete curr_experiment;

						curr_experiment = parent;
						target_count *= 2.0;
					} else {
						break;
					}
				}
			}

			this->verify_experiments.clear();

			if (this->state_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			}
		}
	}
}
