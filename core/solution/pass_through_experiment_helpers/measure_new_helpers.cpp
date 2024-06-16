#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::measure_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	history->predicted_scores.push_back(vector<double>(context.size(), 0.0));
	for (int l_index = 0; l_index < (int)context.size(); l_index++) {
		Scope* scope = (Scope*)context[l_index].scope;
		ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
		if (scope->eval_network != NULL) {
			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
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
		double inner_score;
		this->best_info_scope->activate(problem,
										run_helper,
										inner_score);

		if ((this->best_is_negate && inner_score < 0.0)
				|| (!this->best_is_negate && inner_score >= 0.0)) {
			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}
		}
	}
}

void PassThroughExperiment::measure_new_back_activate(
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

void PassThroughExperiment::measure_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
			sum_score += history->predicted_scores[i_index][l_index];
		}
		double final_score = (sum_score / (int)history->predicted_scores[i_index].size() + target_val - solution->average_score) / 2.0;
		this->target_val_histories.push_back(final_score);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		#if defined(MDEBUG) && MDEBUG
		this->target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

		if (new_average_score >= this->existing_average_score) {
		#endif /* MDEBUG */
			if (this->best_info_scope != NULL) {
				this->info_branch_node = new InfoBranchNode();
				this->info_branch_node->parent = this->scope_context;
				this->info_branch_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;
			}

			this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
