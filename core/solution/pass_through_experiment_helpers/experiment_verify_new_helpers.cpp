#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "new_info_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void PassThroughExperiment::experiment_verify_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	switch (this->score_type) {
	case SCORE_TYPE_TRUTH:
		history->predicted_scores.push_back(vector<double>());
		break;
	case SCORE_TYPE_ALL:
		history->predicted_scores.push_back(vector<double>(context.size()-1));
		for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
			ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;

			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
		break;
	case SCORE_TYPE_LOCAL:
		{
			history->predicted_scores.push_back(vector<double>(1));

			ScopeHistory* scope_history = (ScopeHistory*)context[context.size()-2].scope_history;

			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(0);
		}
		break;
	}

	if (this->best_info_scope == NULL) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}
	} else {
		bool is_positive;
		this->best_info_scope->activate(problem,
										context,
										run_helper,
										is_positive);

		

		bool is_branch;
		if (this->best_is_negate) {
			if (is_positive) {
				is_branch = false;
			} else {
				is_branch = true;
			}
		} else {
			if (is_positive) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}

		InfoBranchNodeHistory* info_branch_node_history = new InfoBranchNodeHistory();
		info_branch_node_history->index = context.back().scope_history->node_histories.size();
		context.back().scope_history->node_histories[this->info_branch_node] = info_branch_node_history;
		if (is_branch) {
			info_branch_node_history->is_branch = true;

			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}
		} else {
			info_branch_node_history->is_branch = false;
		}
	}
}

void PassThroughExperiment::experiment_verify_new_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history);
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void PassThroughExperiment::experiment_verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double final_score;
		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			final_score = target_val - solution->average_score;
			break;
		case SCORE_TYPE_ALL:
			{
				double sum_score = target_val - solution->average_score;
				for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
					sum_score += history->predicted_scores[i_index][l_index];
				}
				final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
			}
			break;
		case SCORE_TYPE_LOCAL:
			final_score = history->predicted_scores[i_index][0];
			break;
		}

		this->target_val_histories.push_back(final_score);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (new_average_score > this->existing_average_score) {
		#endif /* MDEBUG */
			cout << "PassThrough experiment success" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else {
					cout << " E";
				}
			}
			cout << endl;

			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;

			/**
			 * - also finalize this->verify_experiments in finalize()
			 */

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			bool to_experiment = false;
			switch (this->verify_experiments.back()->type) {
			case EXPERIMENT_TYPE_BRANCH:
				{
					double combined_branch_weight = 1.0;
					AbstractExperiment* curr_experiment = this->verify_experiments.back();
					while (true) {
						if (curr_experiment == NULL) {
							break;
						}

						switch (curr_experiment->type) {
						case EXPERIMENT_TYPE_BRANCH:
							{
								BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
								combined_branch_weight *= branch_experiment->branch_weight;
							}
							break;
						case EXPERIMENT_TYPE_NEW_INFO:
							{
								NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
								combined_branch_weight *= new_info_experiment->branch_weight;
							}
							break;
						}
						curr_experiment = curr_experiment->parent_experiment;
					}

					BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
					if (branch_experiment->best_step_types.size() > 0
							&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
						#if defined(MDEBUG) && MDEBUG
						for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
							delete branch_experiment->verify_problems[p_index];
						}
						branch_experiment->verify_problems.clear();
						branch_experiment->verify_seeds.clear();
						branch_experiment->verify_scores.clear();
						/**
						 * - simply rely on leaf experiment to verify
						 */
						#endif /* MDEBUG */

						branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
						branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
						branch_experiment->experiment_iter = 0;

						to_experiment = true;
					}
				}
				break;
			case EXPERIMENT_TYPE_PASS_THROUGH:
				{
					PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
					if (pass_through_experiment->best_step_types.size() > 0) {
						pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
						pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
						pass_through_experiment->experiment_iter = 0;

						to_experiment = true;
					}
				}
				break;
			case EXPERIMENT_TYPE_NEW_INFO:
				{
					double combined_branch_weight = 1.0;
					AbstractExperiment* curr_experiment = this->verify_experiments.back();
					while (true) {
						if (curr_experiment == NULL) {
							break;
						}

						switch (curr_experiment->type) {
						case EXPERIMENT_TYPE_BRANCH:
							{
								BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
								combined_branch_weight *= branch_experiment->branch_weight;
							}
							break;
						case EXPERIMENT_TYPE_NEW_INFO:
							{
								NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
								combined_branch_weight *= new_info_experiment->branch_weight;
							}
							break;
						}
						curr_experiment = curr_experiment->parent_experiment;
					}

					NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)this->verify_experiments.back();
					if (new_info_experiment->best_step_types.size() > 0
							&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
						#if defined(MDEBUG) && MDEBUG
						for (int p_index = 0; p_index < (int)new_info_experiment->verify_problems.size(); p_index++) {
							delete new_info_experiment->verify_problems[p_index];
						}
						new_info_experiment->verify_problems.clear();
						new_info_experiment->verify_seeds.clear();
						new_info_experiment->verify_scores.clear();
						#endif /* MDEBUG */

						new_info_experiment->state = NEW_INFO_EXPERIMENT_STATE_EXPERIMENT;
						new_info_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
						new_info_experiment->experiment_iter = 0;

						to_experiment = true;
					}
				}
				break;
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
				this->verify_experiments.back()->finalize(NULL);
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
						curr_experiment->finalize(NULL);
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
