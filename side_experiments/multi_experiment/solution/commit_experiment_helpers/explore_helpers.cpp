#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int COMMIT_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int COMMIT_EXPERIMENT_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void CommitExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		CommitExperimentHistory* history) {
	double sum_vals = this->existing_average_score;
	for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
		double val;
		fetch_factor_helper(scope_history,
							this->existing_factor_ids[f_index],
							val);
		sum_vals += this->existing_factor_weights[f_index] * val;
	}
	history->existing_predicted_score = sum_vals;

	/**
	 * - exit in-place to not delete existing nodes
	 */
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			history->curr_exit_next_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			history->curr_exit_next_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				history->curr_exit_next_node = branch_node->branch_next_node;
			} else {
				history->curr_exit_next_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;
			history->curr_exit_next_node = obs_node->next_node;
		}
		break;
	}

	geometric_distribution<int> geo_distribution(0.2);
	int new_num_steps = 3 + geo_distribution(generator);

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

void CommitExperiment::explore_backprop(CommitExperimentHistory* history) {
	this->state_iter++;

	double curr_surprise = history->impact - history->existing_predicted_score;
	#if defined(MDEBUG) && MDEBUG
	if (true) {
	#else
	if (curr_surprise > this->best_surprise) {
	#endif /* MDEBUG */
		this->best_surprise = curr_surprise;
		this->best_step_types = history->curr_step_types;
		this->best_actions = history->curr_actions;
		this->best_scopes = history->curr_scopes;
		this->best_exit_next_node = history->curr_exit_next_node;
	}
}

void CommitExperiment::explore_update() {
	if (this->state_iter >= COMMIT_EXPERIMENT_EXPLORE_ITERS) {
		if (this->best_surprise > 0.0) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->parent = this->scope_context;
					new_action_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_action_node->action = this->best_actions[s_index];

					this->new_nodes.push_back(new_action_node);
				} else {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->parent = this->scope_context;
					new_scope_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_scope_node->scope = this->best_scopes[s_index];

					this->new_nodes.push_back(new_scope_node);
				}

				ObsNode* new_obs_node = new ObsNode();
				new_obs_node->parent = this->scope_context;
				new_obs_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				this->new_nodes.push_back(new_obs_node);
			}

			uniform_int_distribution<int> experiment_node_distribution(0, this->new_nodes.size() / 2);
			this->experiment_index = 2 * experiment_node_distribution(generator) + 1;

			this->curr_experiment = new BranchExperiment(this->scope_context,
														 this->new_nodes[this->experiment_index],
														 false);
			this->curr_experiment->parent_experiment = this;

			this->state = COMMIT_EXPERIMENT_STATE_EXPERIMENT;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
