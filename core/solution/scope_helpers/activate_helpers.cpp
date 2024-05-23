/**
 * TODO: kill run if explored into too bad of a situation for faster restart
 */

#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "abstract_experiment.h"
#include "eval.h"
#include "eval_pass_through_experiment.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "new_action_experiment.h"
#include "problem.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  ScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* node = (ScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* node = (BranchNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* node = (InfoScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* node = (InfoBranchNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	}
}

void Scope::activate(Problem* problem,
					 vector<ContextLayer>& context,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	bool is_selected = false;
	if (solution->explore_id == this->id) {
		if (run_helper.experiment_scope_history == NULL) {
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < 1.0 / solution->explore_average_instances_per_run) {
				is_selected = true;
			}
		}
	}
	if (is_selected) {
		#if defined(MDEBUG) && MDEBUG
		run_helper.problem_snapshot = problem->copy_snapshot();
		run_helper.run_seed_snapshot = run_helper.curr_run_seed;
		#endif /* MDEBUG */

		run_helper.experiment_scope_history = history;

		EvalHistory* eval_history = new EvalHistory(this->eval);
		int num_actions_until_random = -1;
		if (solution->explore_type == EXPLORE_TYPE_SCORE) {
			this->eval->activate(problem,
								 context,
								 run_helper,
								 eval_history);
		} else {
			// if (this->eval->experiment == NULL) {
			// 	this->eval->experiment = new EvalPassThroughExperiment(this->eval);
			// }
			// EvalPassThroughExperiment* eval_pass_through_experiment = (EvalPassThroughExperiment*)this->eval->experiment;
			// history->experiments_seen_order.push_back(eval_pass_through_experiment);
			// EvalPassThroughExperimentHistory* experiment_history = new EvalPassThroughExperimentHistory(eval_pass_through_experiment);
			// history->experiment_histories.push_back(experiment_history);

			// /**
			//  * - in base case, only have access to final score (and starting average)
			//  *   - so simply don't evaluate eval sequence without scope sequence
			//  */
			// if (context.size() != 1) {
			// 	experiment_history->outer_eval_history = new EvalHistory(context[context.size()-2].scope->eval);
			// 	context[context.size()-2].scope->eval->activate(
			// 		problem,
			// 		run_helper,
			// 		experiment_history->outer_eval_history->start_scope_history);
			// }

			// if (eval_pass_through_experiment->state != EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE) {
			// 	this->eval->activate(problem,
			// 						 run_helper,
			// 						 eval_history->start_scope_history);

			// 	uniform_int_distribution<int> random_distribution = uniform_int_distribution<int>(0, 2*(int)(solution->explore_scope_local_average_num_actions));
			// 	num_actions_until_random = random_distribution(generator);
			// }
		}

		AbstractNode* curr_node = this->nodes[0];
		while (true) {
			if (curr_node == NULL) {
				break;
			}

			if (num_actions_until_random > 0) {
				num_actions_until_random--;
				if (num_actions_until_random == 0) {
					random_sequence(curr_node,
									problem,
									context,
									run_helper);
					break;
				}
			}

			node_activate_helper(curr_node,
								 problem,
								 context,
								 run_helper,
								 history);

			run_helper.num_actions++;
			if (run_helper.num_actions > solution->num_actions_limit) {
				break;
			}
			if (run_helper.num_actions_limit > 0) {
				run_helper.num_actions_limit--;
				if (run_helper.num_actions_limit == 0) {
					break;
				}
			}
		}

		if (run_helper.num_actions <= solution->num_actions_limit) {
			if (solution->explore_type == EXPLORE_TYPE_SCORE) {
				if (history->experiments_seen_order.size() == 0) {
					create_experiment(history);
				}
			}

			if (history->experiment_histories.size() > 0) {
				for (int e_index = 0; e_index < (int)history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)history->experiments_seen_order.size()-1 - e_index
							+ history->experiment_histories[0]->experiment->average_remaining_experiments_from_start);
				}
				for (int h_index = 0; h_index < (int)history->experiment_histories.size()-1; h_index++) {
					AbstractExperimentHistory* experiment_history = history->experiment_histories[h_index];
					for (int e_index = 0; e_index < (int)experiment_history->experiments_seen_order.size(); e_index++) {
						AbstractExperiment* experiment = experiment_history->experiments_seen_order[e_index];
						experiment->average_remaining_experiments_from_start =
							0.9 * experiment->average_remaining_experiments_from_start
							+ 0.1 * ((int)experiment_history->experiments_seen_order.size()-1 - e_index
								+ history->experiment_histories[h_index+1]->experiment->average_remaining_experiments_from_start);
					}
				}
				{
					/**
					 * - non-empty if EXPERIMENT_STATE_EXPERIMENT
					 */
					AbstractExperimentHistory* experiment_history = history->experiment_histories.back();
					for (int e_index = 0; e_index < (int)experiment_history->experiments_seen_order.size(); e_index++) {
						AbstractExperiment* experiment = experiment_history->experiments_seen_order[e_index];
						experiment->average_remaining_experiments_from_start =
							0.9 * experiment->average_remaining_experiments_from_start
							+ 0.1 * ((int)experiment_history->experiments_seen_order.size()-1 - e_index);
					}
				}

				history->experiment_histories.back()->experiment->backprop(
					eval_history,
					problem,
					context,
					run_helper);
				if (history->experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
					if (history->experiment_histories.size() == 1) {
						history->experiment_histories.back()->experiment->finalize(NULL);
						delete history->experiment_histories.back()->experiment;
					} else {
						AbstractExperiment* curr_experiment = history->experiment_histories.back()->experiment->parent_experiment;

						curr_experiment->experiment_iter++;
						int matching_index;
						for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
							if (curr_experiment->child_experiments[c_index] == history->experiment_histories.back()->experiment) {
								matching_index = c_index;
								break;
							}
						}
						curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

						history->experiment_histories.back()->experiment->result = EXPERIMENT_RESULT_FAIL;
						history->experiment_histories.back()->experiment->finalize(NULL);
						delete history->experiment_histories.back()->experiment;

						double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
							* pow(0.5, history->experiment_histories.size()-1);
						while (true) {
							if (curr_experiment == NULL) {
								break;
							}

							if (curr_experiment->experiment_iter >= target_count) {
								AbstractExperiment* parent = curr_experiment->parent_experiment;

								if (parent != NULL) {
									parent->experiment_iter++;
									int matching_index;
									for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
										if (parent->child_experiments[c_index] == curr_experiment) {
											matching_index = c_index;
											break;
										}
									}
									parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);
								}

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
				} else if (history->experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					/**
					 * - history->experiment_histories.size() == 1
					 */
					Solution* duplicate = new Solution(solution);
					history->experiment_histories.back()->experiment->finalize(duplicate);
					delete history->experiment_histories.back()->experiment;

					run_helper.success_duplicate = duplicate;
				}
			} else {
				for (int e_index = 0; e_index < (int)history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)history->experiments_seen_order.size()-1 - e_index);
				}
			}
		}

		delete eval_history;

		#if defined(MDEBUG) && MDEBUG
		if (run_helper.problem_snapshot != NULL) {
			delete run_helper.problem_snapshot;
			run_helper.problem_snapshot = NULL;
		}
		#endif /* MDEBUG */

		run_helper.experiment_scope_history = NULL;
	} else {
		AbstractNode* curr_node = this->nodes[0];
		while (true) {
			if (curr_node == NULL
					|| run_helper.success_duplicate != NULL) {
				break;
			}

			node_activate_helper(curr_node,
								 problem,
								 context,
								 run_helper,
								 history);

			run_helper.num_actions++;
			if (run_helper.num_actions > solution->num_actions_limit) {
				break;
			}
			if (run_helper.num_actions_limit > 0) {
				run_helper.num_actions_limit--;
				if (run_helper.num_actions_limit == 0) {
					break;
				}
			}
		}
	}
}

void Scope::activate(AbstractNode* starting_node,
					 Problem* problem,
					 vector<ContextLayer>& context,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	AbstractNode* curr_node = starting_node;
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		node_activate_helper(curr_node,
							 problem,
							 context,
							 run_helper,
							 history);

		run_helper.num_actions++;
		if (run_helper.num_actions > solution->num_actions_limit) {
			break;
		}
		if (run_helper.num_actions_limit > 0) {
			run_helper.num_actions_limit--;
			if (run_helper.num_actions_limit == 0) {
				break;
			}
		}
	}
}
