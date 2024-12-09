#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

bool BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;
	if (!history->has_target
			&& this->num_instances_until_target == 0) {
		history->has_target = true;

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
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
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
		uniform_int_distribution<int> scope_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (this->scope_context->child_scopes.size() > 0
					&& scope_distribution(generator) == 0) {
				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(NULL);

				ScopeNode* new_scope_node = new ScopeNode();
				uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
				new_scope_node->scope = this->scope_context->child_scopes[child_scope_distribution(generator)];
				this->curr_scopes.push_back(new_scope_node);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_scopes.push_back(NULL);
			}
		}

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				this->curr_actions[s_index]->explore_activate(
					problem,
					run_helper);
			} else {
				this->curr_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}
		}

		curr_node = this->curr_exit_next_node;

		return true;
	}  else {
		return false;
	}
}

void BranchExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	uniform_int_distribution<int> until_distribution(0, (int)this->node_context->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (history->has_target) {
		double curr_surprise = target_val - run_helper.result;

		bool select = false;
		if (this->explore_type == EXPLORE_TYPE_BEST) {
			#if defined(MDEBUG) && MDEBUG
			if (true) {
			#else
			if (curr_surprise > this->best_surprise) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_surprise = curr_surprise;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else {
						delete this->curr_scopes[s_index];
					}
				}

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
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else {
						delete this->curr_scopes[s_index];
					}
				}

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}
		}

		if (select) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_actions[s_index]->parent = this->scope_context;
					this->best_actions[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				} else {
					this->best_scopes[s_index]->parent = this->scope_context;
					this->best_scopes[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				}
			}

			int exit_node_id;
			AbstractNode* exit_node;
			if (this->best_exit_next_node == NULL) {
				ActionNode* new_ending_node = new ActionNode();
				new_ending_node->parent = this->scope_context;
				new_ending_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				new_ending_node->action = Action(ACTION_NOOP);

				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;

				this->ending_node = new_ending_node;

				exit_node_id = new_ending_node->id;
				exit_node = new_ending_node;
			} else {
				exit_node_id = this->best_exit_next_node->id;
				exit_node = this->best_exit_next_node;
			}

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				int next_node_id;
				AbstractNode* next_node;
				if (s_index == (int)this->best_step_types.size()-1) {
					next_node_id = exit_node_id;
					next_node = exit_node;
				} else {
					if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
						next_node_id = this->best_actions[s_index+1]->id;
						next_node = this->best_actions[s_index+1];
					} else {
						next_node_id = this->best_scopes[s_index+1]->id;
						next_node = this->best_scopes[s_index+1];
					}
				}

				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_actions[s_index]->next_node_id = next_node_id;
					this->best_actions[s_index]->next_node = next_node;
				} else {
					this->best_scopes[s_index]->next_node_id = next_node_id;
					this->best_scopes[s_index]->next_node = next_node;
				}
			}

			this->scope_histories.reserve(NUM_DATAPOINTS);
			this->target_val_histories.reserve(NUM_DATAPOINTS);

			uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			this->state_iter++;
			if (this->state_iter >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
