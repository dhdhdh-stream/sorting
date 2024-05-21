#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "problem.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void measure_node_activate_helper(AbstractNode*& curr_node,
								  Metrics& metrics,
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
			node->measure_activate(metrics,
								   curr_node,
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

void Scope::measure_activate(Metrics& metrics,
							 Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* history) {
	int starting_num_actions;
	if (metrics.next_explore_id == this->id) {
		starting_num_actions = run_helper.num_actions;
		metrics.next_num_instances++;
	}

	if (metrics.curr_explore_id == this->id) {
		if (metrics.curr_explore_type == EXPLORE_TYPE_SCORE) {
			EvalHistory* eval_history = new EvalHistory(this->eval);
			this->eval->activate(problem,
								 run_helper,
								 eval_history->start_scope_history);

			AbstractNode* curr_node = this->nodes[0];
			while (true) {
				if (curr_node == NULL) {
					break;
				}

				measure_node_activate_helper(curr_node,
											 metrics,
											 problem,
											 context,
											 run_helper,
											 history);

				run_helper.num_actions++;
				if (run_helper.num_actions > solution->num_actions_limit) {
					break;
				}
			}

			this->eval->activate(problem,
								 run_helper,
								 eval_history->end_scope_history);
			double predicted_vs = this->eval->calc_vs(
				run_helper,
				eval_history);

			metrics.curr_sum_timestamp_score += predicted_vs;

			delete eval_history;
		} else {
			EvalHistory* outer_eval_history;
			double starting_target_val;
			if (context.size() == 1) {
				starting_target_val = 1.0;
			} else {
				outer_eval_history = new EvalHistory(context[context.size()-2].scope->eval);
				context[context.size()-2].scope->eval->activate(
					problem,
					run_helper,
					outer_eval_history->start_scope_history);
				starting_target_val = context[context.size()-2].scope->eval->calc_score(
					run_helper,
					outer_eval_history->start_scope_history);
			}

			EvalHistory* eval_history = new EvalHistory(this->eval);
			this->eval->activate(problem,
								 run_helper,
								 eval_history->start_scope_history);
			double starting_predicted_score = this->eval->calc_score(
				run_helper,
				eval_history->start_scope_history);

			uniform_int_distribution<int> random_distribution = uniform_int_distribution<int>(0, 2*(int)(solution->explore_scope_local_average_num_actions));
			int num_actions_until_random = random_distribution(generator);

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

				measure_node_activate_helper(curr_node,
											 metrics,
											 problem,
											 context,
											 run_helper,
											 history);

				run_helper.num_actions++;
				if (run_helper.num_actions > solution->num_actions_limit) {
					break;
				}
			}

			this->eval->activate(problem,
								 run_helper,
								 eval_history->end_scope_history);
			double predicted_vs = this->eval->calc_vs(
				run_helper,
				eval_history);
			double ending_predicted_score = starting_predicted_score + predicted_vs;

			double ending_target_val;
			if (context.size() == 1) {
				ending_target_val = problem->score_result(run_helper.num_decisions);
			} else {
				context[context.size()-2].scope->eval->activate(
					problem,
					run_helper,
					outer_eval_history->end_scope_history);
				double ending_target_vs = context[context.size()-2].scope->eval->calc_vs(
					run_helper,
					outer_eval_history);
				ending_target_val = starting_target_val + ending_target_vs;

				delete outer_eval_history;
			}

			double ending_misguess = (ending_target_val - ending_predicted_score) * (ending_target_val - ending_predicted_score);
			metrics.curr_sum_timestamp_score -= ending_misguess;

			delete eval_history;
		}

		metrics.curr_num_instances++;
	} else {
		AbstractNode* curr_node = this->nodes[0];
		while (true) {
			if (curr_node == NULL) {
				break;
			}

			measure_node_activate_helper(curr_node,
										 metrics,
										 problem,
										 context,
										 run_helper,
										 history);

			run_helper.num_actions++;
			if (run_helper.num_actions > solution->num_actions_limit) {
				break;
			}
		}
	}

	if (metrics.next_explore_id == this->id) {
		int scope_num_actions = run_helper.num_actions - starting_num_actions;
		if (scope_num_actions > metrics.next_max_num_actions) {
			metrics.next_max_num_actions = scope_num_actions;
		}
		metrics.next_local_sum_num_actions += (int)history->node_histories.size();
	}
}
