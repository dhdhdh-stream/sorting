#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void BranchExperiment::explore_check_activate(
		SolutionWrapper* wrapper,
		BranchExperimentHistory* history) {
	this->num_instances_until_target--;
	if (history->existing_predicted_scores.size() == 0
			&& this->num_instances_until_target == 0) {
		double sum_vals = this->existing_average_score;
		for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
			double val;
			fetch_factor_helper(wrapper->scope_histories.back(),
								this->existing_factor_ids[f_index],
								val);
			sum_vals += this->existing_factor_weights[f_index] * val;
		}
		history->existing_predicted_scores.push_back(sum_vals);

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
				this->curr_actions.push_back(-1);

				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				this->curr_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				this->curr_actions.push_back(-1);

				this->curr_scopes.push_back(NULL);
			}
		}

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::explore_step(vector<double>& obs,
									int& action,
									bool& is_next,
									bool& fetch_action,
									SolutionWrapper* wrapper,
									BranchExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->curr_step_types.size()) {
		wrapper->node_context.back() = this->curr_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
			wrapper->confusion_context.push_back(NULL);

			if (this->curr_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
				this->curr_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
			}
		}
	}
}

void BranchExperiment::explore_set_action(int action,
										  BranchExperimentState* experiment_state) {
	this->curr_actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void BranchExperiment::explore_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	if (this->curr_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
		this->curr_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
	}

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();
	wrapper->confusion_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::explore_backprop(
		double target_val,
		BranchExperimentHistory* history) {
	uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (history->existing_predicted_scores.size() > 0) {
		double curr_surprise = target_val - history->existing_predicted_scores[0];

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
			uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
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
