#include "eval_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void EvalPassThroughExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	if (this->info_scope == NULL) {
		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}
	} else {
		ScopeHistory* inner_scope_history;
		bool inner_is_positive;
		this->info_scope->activate(problem,
								   run_helper,
								   inner_scope_history,
								   inner_is_positive);

		delete inner_scope_history;

		if ((this->is_negate && !inner_is_positive)
				|| (!this->is_negate && inner_is_positive)) {
			if (this->step_types.size() == 0) {
				curr_node = this->exit_next_node;
			} else {
				if (this->step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->actions[0];
				} else {
					curr_node = this->scopes[0];
				}
			}
		}
	}
}

void EvalPassThroughExperiment::train_new_backprop(
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	double starting_target_val;
	if (context.size() == 1) {
		/**
		 * TODO: set to score if no actions were performed
		 */
		starting_target_val = 1.46;
	} else {
		starting_target_val = context[context.size()-2].scope->eval->calc_score(
			run_helper,
			history->outer_eval_history->start_scope_history);
	}

	this->start_scope_histories.push_back(new ScopeHistory(eval_history->start_scope_history));

	this->eval_context->activate(problem,
								 run_helper,
								 eval_history->end_scope_history);

	double ending_target_val;
	if (context.size() == 1) {
		ending_target_val = problem->score_result(run_helper.num_decisions);
	} else {
		context[context.size()-2].scope->eval->activate(
			problem,
			run_helper,
			history->outer_eval_history->end_scope_history);
		double ending_target_vs = context[context.size()-2].scope->eval->calc_vs(
			run_helper,
			history->outer_eval_history);
		ending_target_val = starting_target_val + ending_target_vs;
	}
	this->end_target_val_histories.push_back(ending_target_val);

	this->end_scope_histories.push_back(new ScopeHistory(eval_history->end_scope_history));

	if ((int)this->start_scope_histories.size() >= NUM_DATAPOINTS) {
		train_score();

		train_vs();

		for (int i_index = 0; i_index < (int)this->start_scope_histories.size(); i_index++) {
			delete this->start_scope_histories[i_index];
		}
		this->start_scope_histories.clear();
		for (int i_index = 0; i_index < (int)this->end_scope_histories.size(); i_index++) {
			delete this->end_scope_histories[i_index];
		}
		this->end_scope_histories.clear();
		this->end_target_val_histories.clear();

		this->score_misguess_histories.reserve(2 * NUM_DATAPOINTS);
		this->vs_misguess_histories.reserve(NUM_DATAPOINTS);

		this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
