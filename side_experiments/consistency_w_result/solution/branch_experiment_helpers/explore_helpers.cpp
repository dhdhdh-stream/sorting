#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;
	if (history->instance_count == 0
			&& this->num_instances_until_target == 0) {
		history->instance_count++;

		vector<AbstractNode*> possible_exits;

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
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
		int random_index = exit_distribution(generator);
		this->curr_exit_next_node = possible_exits[random_index];

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
		uniform_int_distribution<int> scope_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(Action());

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->curr_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				this->curr_actions.push_back(problem_type->random_action());

				this->curr_scopes.push_back(NULL);
			}
		}

		run_helper.check_match = true;

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->curr_actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[s_index]);
				this->curr_scopes[s_index]->experiment_activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->curr_exit_next_node;
	}
}

void BranchExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	uniform_int_distribution<int> until_distribution(0, max((int)this->node_context->average_instances_per_run-1, 0));
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (history->instance_count > 0) {
		double curr_surprise = target_val - run_helper.result;

		bool select = false;
		if (this->explore_type == EXPLORE_TYPE_BEST) {
			#if defined(MDEBUG) && MDEBUG
			if (true) {
			#else
			if (curr_surprise > this->best_surprise) {
			#endif /* MDEBUG */
				this->best_surprise = curr_surprise;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			} else {
				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}

			if (this->state_iter == BRANCH_EXPERIMENT_EXPLORE_ITERS-1
					&& this->best_surprise > 0.0) {
				select = true;
			}
		} else if (this->explore_type == EXPLORE_TYPE_GOOD) {
			#if defined(MDEBUG) && MDEBUG
			if (true) {
			#else
			if (curr_surprise > 0.0) {
			#endif /* MDEBUG */
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();

				select = true;
			} else {
				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}
		}

		if (select) {
			uniform_int_distribution<int> until_distribution(0, max((int)this->node_context->average_instances_per_run-1, 0));
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->state = BRANCH_EXPERIMENT_STATE_NEW_GATHER;
			this->state_iter = 0;
		} else {
			this->state_iter++;
			if (this->state_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
