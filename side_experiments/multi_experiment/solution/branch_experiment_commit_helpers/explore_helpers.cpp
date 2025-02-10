#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "commit_experiment.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::explore_commit_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		ScopeHistory* temp_history,
		BranchExperimentHistory* history) {
	double sum_vals = this->existing_average_score;
	for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
		{
			double val;
			fetch_factor_helper(scope_history,
								this->existing_factor_ids[f_index],
								val);
			if (val != 0.0) {
				sum_vals += this->existing_factor_weights[f_index] * val;
			}
		}
		{
			double val;
			fetch_factor_helper(temp_history,
								this->existing_factor_ids[f_index],
								val);
			if (val != 0.0) {
				sum_vals += this->existing_factor_weights[f_index] * val;
			}
		}
	}
	history->existing_predicted_score = sum_vals;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (this->parent_experiment->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->parent_experiment->node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->parent_experiment->node_context;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->parent_experiment->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->parent_experiment->node_context;
			starting_node = obs_node->next_node;
		}
		break;
	}

	this->scope_context->random_exit_activate(
		starting_node,
		possible_exits);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	history->curr_exit_next_node = possible_exits[random_index];

	int new_num_steps;
	geometric_distribution<int> geo_distribution(0.2);
	if (random_index == 0) {
		new_num_steps = 1 + geo_distribution(generator);
	} else {
		new_num_steps = geo_distribution(generator);
	}

	/**
	 * - always give raw actions a large weight
	 *   - existing scopes often learned to avoid certain patterns
	 *     - which can prevent innovation
	 */
	uniform_int_distribution<int> type_distribution(0, 2);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		int type = type_distribution(generator);
		if (type >= 2 && this->scope_context->child_scopes.size() > 0) {
			history->curr_step_types.push_back(STEP_TYPE_SCOPE);
			history->curr_actions.push_back(Action());

			uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
			history->curr_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
		} else if (type >= 1 && this->scope_context->existing_scopes.size() > 0) {
			history->curr_step_types.push_back(STEP_TYPE_SCOPE);
			history->curr_actions.push_back(Action());

			uniform_int_distribution<int> existing_scope_distribution(0, this->scope_context->existing_scopes.size()-1);
			history->curr_scopes.push_back(this->scope_context->existing_scopes[existing_scope_distribution(generator)]);
		} else {
			history->curr_step_types.push_back(STEP_TYPE_ACTION);

			history->curr_actions.push_back(problem_type->random_action());

			history->curr_scopes.push_back(NULL);
		}
	}

	for (int s_index = 0; s_index < (int)history->curr_step_types.size(); s_index++) {
		if (history->curr_step_types[s_index] == STEP_TYPE_ACTION) {
			double score = problem->perform_action(history->curr_actions[s_index]);
			run_helper.sum_score += score;
			run_helper.num_actions++;
			double individual_impact = score / run_helper.num_actions;
			for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size(); h_index++) {
				run_helper.experiment_histories[h_index]->impact += individual_impact;
			}
			if (score < 0.0) {
				run_helper.early_exit = true;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(history->curr_scopes[s_index]);
			history->curr_scopes[s_index]->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;
		}

		if (run_helper.early_exit) {
			break;
		}
	}

	curr_node = history->curr_exit_next_node;
}
