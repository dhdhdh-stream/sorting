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

void EvalPassThroughExperiment::measure_existing_backprop(
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	double starting_target_val;
	if (context.size() == 1) {
		starting_target_val = solution->curr_average_score;
	} else {
		starting_target_val = context[context.size()-2].scope->eval->calc_score(
			run_helper,
			history->outer_eval_history->start_scope_history);
	}

	double starting_predicted_score = this->eval_context->calc_score(
		run_helper,
		eval_history->start_scope_history);

	double starting_misguess = (starting_target_val - starting_predicted_score) * (starting_target_val - starting_predicted_score);
	this->score_misguess_histories.push_back(starting_misguess);

	this->eval_context->activate(problem,
								 run_helper,
								 eval_history->end_scope_history);
	double ending_predicted_score = this->eval_context->calc_score(
		run_helper,
		eval_history->end_scope_history);
	double ending_predicted_vs = this->eval_context->calc_vs(
		run_helper,
		eval_history);
	double ending_vs_predicted_score = starting_predicted_score + ending_predicted_vs;

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

	double ending_misguess = (ending_target_val - ending_predicted_score) * (ending_target_val - ending_predicted_score);
	this->score_misguess_histories.push_back(ending_misguess);

	double ending_vs_misguess = (ending_target_val - ending_vs_predicted_score) * (ending_target_val - ending_vs_predicted_score);
	this->vs_misguess_histories.push_back(ending_vs_misguess);

	if ((int)this->vs_misguess_histories.size() >= NUM_DATAPOINTS) {
		double sum_score_misguesses = 0.0;
		for (int m_index = 0; m_index < 2 * NUM_DATAPOINTS; m_index++) {
			sum_score_misguesses += this->score_misguess_histories[m_index];
		}
		this->existing_score_average_misguess = sum_score_misguesses / (2 * NUM_DATAPOINTS);

		double sum_score_misguess_variance = 0.0;
		for (int m_index = 0; m_index < 2 * NUM_DATAPOINTS; m_index++) {
			sum_score_misguess_variance += (this->score_misguess_histories[m_index] - this->existing_score_average_misguess) * (this->score_misguess_histories[m_index] - this->existing_score_average_misguess);
		}
		this->existing_score_misguess_standard_deviation = sqrt(sum_score_misguess_variance / (2 * NUM_DATAPOINTS));
		if (this->existing_score_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->score_misguess_histories.clear();

		double sum_vs_misguesses = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_vs_misguesses += this->vs_misguess_histories[m_index];
		}
		this->existing_vs_average_misguess = sum_vs_misguesses / NUM_DATAPOINTS;

		double sum_vs_misguess_variance = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_vs_misguess_variance += (this->vs_misguess_histories[m_index] - this->existing_vs_average_misguess) * (this->vs_misguess_histories[m_index] - this->existing_vs_average_misguess);
		}
		this->existing_vs_misguess_standard_deviation = sqrt(sum_vs_misguess_variance / NUM_DATAPOINTS);
		if (this->existing_vs_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_vs_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->vs_misguess_histories.clear();

		vector<AbstractNode*> possible_starts;
		this->scope_context->random_exit_activate(
			this->scope_context->nodes[0],
			possible_starts);

		uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
		this->node_context = possible_starts[start_distribution(generator)];

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				this->is_branch = false;

				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_INFO_SCOPE:
			{
				this->is_branch = false;

				InfoScopeNode* info_scope_node = (InfoScopeNode*)this->node_context;
				starting_node = info_scope_node->next_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				/**
				 * TODO: set this->is_branch more accurately
				 */
				uniform_int_distribution<int> is_branch_distribution(0, 1);
				this->is_branch = is_branch_distribution(generator);

				InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = info_branch_node->branch_next_node;
				} else {
					starting_node = info_branch_node->original_next_node;
				}
			}
			break;
		}

		this->node_context->experiments.push_back(this);

		vector<AbstractNode*> possible_exits;

		if (this->node_context->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context)->next_node == NULL) {
			possible_exits.push_back(NULL);
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
