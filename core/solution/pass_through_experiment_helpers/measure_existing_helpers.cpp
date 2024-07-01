#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_set.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::measure_existing_activate(
		vector<ContextLayer>& context,
		PassThroughExperimentHistory* history) {
	history->instance_count++;

	switch (this->score_type) {
	case SCORE_TYPE_TRUTH:
		history->predicted_scores.push_back(vector<double>());
		break;
	case SCORE_TYPE_ALL:
		history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
		for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
			ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
			Scope* scope = (Scope*)scope_history->scope;

			if (scope->eval_network != NULL) {
				scope_history->callback_experiment_history = history;
				scope_history->callback_experiment_indexes.push_back(
					(int)history->predicted_scores.size()-1);
				scope_history->callback_experiment_layers.push_back(l_index);
			}
		}
		break;
	}
}

void PassThroughExperiment::measure_existing_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history) / (int)scope_history->callback_experiment_indexes.size();
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void PassThroughExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double final_score;
		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			final_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
			break;
		case SCORE_TYPE_ALL:
			{
				double sum_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
				for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
					sum_score += history->predicted_scores[i_index][l_index];
				}
				final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
			}
			break;
		}

		this->target_val_histories.push_back(final_score);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

		Scope* parent_scope = (Scope*)this->scope_context;

		vector<AbstractNode*> possible_exits;

		if (this->node_context->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context)->next_node == NULL) {
			possible_exits.push_back(NULL);
		}

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = info_branch_node->branch_next_node;
				} else {
					starting_node = info_branch_node->original_next_node;
				}
			}
			break;
		}

		parent_scope->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->curr_exit_next_node = possible_exits[random_index];

		this->curr_info_scope = get_existing_info_scope(parent_scope);
		uniform_int_distribution<int> negate_distribution(0, 1);
		this->curr_is_negate = negate_distribution(generator) == 0;

		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		if (random_index == 0) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		uniform_int_distribution<int> default_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			if (default_distribution(generator) != 0) {
				ScopeNode* new_scope_node = create_existing(parent_scope);
				if (new_scope_node != NULL) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_scopes.push_back(NULL);
			}
		}

		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
		this->explore_iter = 0;
	}
}
