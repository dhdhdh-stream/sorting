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
	EvalHistory* outer_eval_history = NULL;
	if ((metrics.curr_explore_id == this->id
				&& metrics.curr_explore_type == EXPLORE_TYPE_EVAL)
			|| (metrics.next_explore_id == this->id
				&& metrics.next_explore_type == EXPLORE_TYPE_EVAL)) {
		if (context.size() != 1) {
			outer_eval_history = new EvalHistory(context[context.size()-2].scope->eval);
			context[context.size()-2].scope->eval->activate_start(
				problem,
				run_helper,
				outer_eval_history);
		}
	}

	int starting_num_actions;
	if (metrics.next_explore_id == this->id) {
		starting_num_actions = run_helper.num_actions;
		metrics.next_num_instances++;
	}

	EvalHistory* eval_history = NULL;
	if (metrics.curr_explore_id == this->id
			|| (metrics.next_explore_id == this->id
				&& metrics.next_explore_type == EXPLORE_TYPE_EVAL)) {
		eval_history = new EvalHistory(this->eval);
		this->eval->activate_start(problem,
								   run_helper,
								   eval_history);
	}

	int num_actions_until_random = -1;
	if (metrics.curr_explore_id == this->id
			&& metrics.curr_explore_type == EXPLORE_TYPE_EVAL) {
		uniform_int_distribution<int> random_distribution = uniform_int_distribution<int>(0, 2*(int)(solution->explore_scope_local_average_num_actions));
		num_actions_until_random = random_distribution(generator);
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

	double predicted_impact;
	if (metrics.curr_explore_id == this->id
			|| (metrics.next_explore_id == this->id
				&& metrics.next_explore_type == EXPLORE_TYPE_EVAL)) {
		this->eval->activate_end(problem,
								 run_helper,
								 eval_history);
		predicted_impact = this->eval->calc_impact(eval_history);
	}

	if (metrics.next_explore_id == this->id) {
		int scope_num_actions = run_helper.num_actions - starting_num_actions;
		if (scope_num_actions > metrics.next_max_num_actions) {
			metrics.next_max_num_actions = scope_num_actions;
		}
		metrics.next_local_sum_num_actions += (int)history->node_histories.size();
	}

	double target_impact;
	if ((metrics.curr_explore_id == this->id
				&& metrics.curr_explore_type == EXPLORE_TYPE_EVAL)
			|| (metrics.next_explore_id == this->id
				&& metrics.next_explore_type == EXPLORE_TYPE_EVAL)) {
		if (context.size() == 1) {
			target_impact = problem->score_result(run_helper.num_decisions);
		} else {
			context[context.size()-2].scope->eval->activate_end(
				problem,
				run_helper,
				outer_eval_history);
			target_impact = context[context.size()-2].scope->eval->calc_impact(outer_eval_history);
		}
	}

	if (metrics.curr_explore_id == this->id
			&& metrics.curr_explore_type == EXPLORE_TYPE_SCORE) {
		metrics.curr_sum_timestamp_score += predicted_impact;

		metrics.curr_num_instances++;
	}

	if (metrics.curr_explore_id == this->id
			&& metrics.curr_explore_type == EXPLORE_TYPE_EVAL) {
		double misguess = (target_impact - predicted_impact) * (target_impact - predicted_impact);
		metrics.curr_sum_timestamp_score -= misguess;

		metrics.curr_num_instances++;
	}

	if (metrics.next_explore_id == this->id
			&& metrics.next_explore_type == EXPLORE_TYPE_EVAL) {
		metrics.next_impacts.push_back(target_impact);
		double misguess = (target_impact - predicted_impact) * (target_impact - predicted_impact);
		metrics.next_misguesses.push_back(misguess);
	}

	if (eval_history != NULL) {
		delete eval_history;
	}
	if (outer_eval_history != NULL) {
		delete outer_eval_history;
	}
}
