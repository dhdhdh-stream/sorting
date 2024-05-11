#include "eval_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void EvalPassThroughExperiment::measure_existing_back_activate(
		ScopeHistory*& subscope_history,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	vector<double> input_vals(solution->eval->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)solution->eval->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.find(
			solution->eval->input_node_contexts[i_index]);
		if (it != subscope_history->node_histories.end()) {
			switch (solution->eval->input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[solution->eval->input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					if (info_scope_node_history->is_positive) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
					if (info_branch_node_history->is_branch) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
				}
				break;
			}
		}
	}

	double score = solution->eval->average_score;
	for (int i_index = 0; i_index < (int)solution->eval->linear_input_indexes.size(); i_index++) {
		score += input_vals[solution->eval->linear_input_indexes[i_index]] * solution->eval->linear_weights[i_index];
	}
	if (solution->eval->network != NULL) {
		vector<vector<double>> network_input_vals(solution->eval->network_input_indexes.size());
		for (int i_index = 0; i_index < (int)solution->eval->network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(solution->eval->network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)solution->eval->network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[solution->eval->network_input_indexes[i_index][v_index]];
			}
		}
		solution->eval->network->activate(network_input_vals);
		score += solution->eval->network->output->acti_vals[0];
	}

	history->predicted_scores.push_back(score);
}

void EvalPassThroughExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int p_index = 0; p_index < (int)history->predicted_scores.size(); p_index++) {
		double misguess = (target_val - history->predicted_scores[p_index]) * (target_val - history->predicted_scores[p_index]);
		this->misguess_histories.push_back(misguess);
	}

	this->state_iter++;
	if (this->state_iter >= NUM_DATAPOINTS) {
		int num_instances = (int)this->misguess_histories.size();

		double sum_misguesses = 0.0;
		for (int m_index = 0; m_index < num_instances; m_index++) {
			sum_misguesses += this->misguess_histories[m_index];
		}
		this->existing_average_misguess = sum_misguesses / num_instances;

		this->misguess_histories.clear();

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
		case NODE_TYPE_INFO_SCOPE:
			{
				InfoScopeNode* info_scope_node = (InfoScopeNode*)this->node_context;
				starting_node = info_scope_node->next_node;
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

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->exit_next_node = possible_exits[random_index];

		this->info_scope = get_existing_info_scope();
		uniform_int_distribution<int> negate_distribution(0, 1);
		this->is_negate = negate_distribution(generator) == 0;

		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		if (random_index == 0) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			InfoScopeNode* new_scope_node = create_existing_info_scope_node();
			if (new_scope_node != NULL) {
				this->step_types.push_back(STEP_TYPE_SCOPE);
				this->actions.push_back(NULL);

				this->scopes.push_back(new_scope_node);
			} else {
				this->step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->actions.push_back(new_action_node);

				this->scopes.push_back(NULL);
			}
		}

		this->new_score = 0.0;

		this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
