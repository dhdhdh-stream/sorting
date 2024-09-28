#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 5;
#else
const int EXPLORE_ITERS = 500;
#endif /* MDEBUG */

bool BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	run_helper.num_analyze += (1 + 2*this->new_analyze_size) * (1 + 2*this->new_analyze_size);

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
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)this->node_context;
				if (this->is_branch) {
					starting_node = return_node->passed_next_node;
				} else {
					starting_node = return_node->skipped_next_node;
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
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geo_distribution(0.5);
		if (random_index == 0) {
			new_num_steps = 1 + uniform_distribution(generator) + geo_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geo_distribution(generator);
		}

		uniform_int_distribution<int> default_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			if (default_distribution(generator) != 0) {
				ScopeNode* new_scope_node = create_existing();
				if (new_scope_node != NULL) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);

					this->curr_returns.push_back(NULL);

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_scopes.push_back(NULL);
				this->curr_returns.push_back(NULL);
			}
		}

		geometric_distribution<int> return_distribution(0.75);
		int num_returns = return_distribution(generator);
		uniform_int_distribution<int> relative_distribution(0, 1);
		for (int r_index = 0; r_index < num_returns; r_index++) {
			ReturnNode* new_return_node = new ReturnNode();
			uniform_int_distribution<int> location_distribution(0, context.back().location_history.size()-1);
			if (relative_distribution(generator) == 0) {
				map<AbstractNode*, vector<double>>::iterator it = next(context.back().location_history.begin(), location_distribution(generator));
				new_return_node->previous_location_id = it->first->id;
				new_return_node->previous_location = it->first;
				vector<double> previous_world_location = (*next(context.back().location_history.begin(), location_distribution(generator))).second;
				new_return_node->location = problem_type->world_to_relative(
					it->second, previous_world_location);
			} else {
				AbstractNode* previous_location = (*next(context.back().location_history.begin(), location_distribution(generator))).first;
				new_return_node->previous_location_id = previous_location->id;
				new_return_node->previous_location = previous_location;
				new_return_node->location = vector<double>(problem_type->num_dimensions(), 0.0);
			}
			uniform_int_distribution<int> step_distribution(0, new_num_steps);
			int step_index = step_distribution(generator);
			this->curr_step_types.insert(this->curr_step_types.begin() + step_index, STEP_TYPE_RETURN);
			this->curr_actions.insert(this->curr_actions.begin() + step_index, NULL);
			this->curr_scopes.insert(this->curr_scopes.begin() + step_index, NULL);
			this->curr_returns.insert(this->curr_returns.begin() + step_index, new_return_node);
		}

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				this->curr_actions[s_index]->explore_activate(
					problem,
					run_helper);
			} else if (this->curr_step_types[s_index] == STEP_TYPE_SCOPE) {
				this->curr_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			} else {
				this->curr_returns[s_index]->explore_activate(
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
			if (!run_helper.exceeded_limit) {
			#else
			if (!run_helper.exceeded_limit
					&& curr_surprise > this->best_surprise) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						delete this->best_scopes[s_index];
					} else {
						delete this->best_returns[s_index];
					}
				}

				this->best_surprise = curr_surprise;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_returns = this->curr_returns;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
				this->curr_returns.clear();
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else if (this->curr_step_types[s_index] == STEP_TYPE_SCOPE) {
						delete this->curr_scopes[s_index];
					} else {
						delete this->curr_returns[s_index];
					}
				}

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
				this->curr_returns.clear();
			}

			if (this->state_iter == EXPLORE_ITERS-1
					&& this->best_surprise > 0.0) {
				select = true;
			}
		} else if (this->explore_type == EXPLORE_TYPE_GOOD) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (!run_helper.exceeded_limit
					&& curr_surprise > 0.0) {
			#endif /* MDEBUG */
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_returns = this->curr_returns;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
				this->curr_returns.clear();

				select = true;
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else if (this->curr_step_types[s_index] == STEP_TYPE_SCOPE) {
						delete this->curr_scopes[s_index];
					} else {
						delete this->curr_returns[s_index];
					}
				}

				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
				this->curr_returns.clear();
			}
		}

		if (select) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_actions[s_index]->parent = this->scope_context;
					this->best_actions[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
					this->best_scopes[s_index]->parent = this->scope_context;
					this->best_scopes[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				} else {
					this->best_returns[s_index]->parent = this->scope_context;
					this->best_returns[s_index]->id = this->scope_context->node_counter;
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
					} else if (this->best_step_types[s_index+1] == STEP_TYPE_SCOPE) {
						next_node_id = this->best_scopes[s_index+1]->id;
						next_node = this->best_scopes[s_index+1];
					} else {
						next_node_id = this->best_returns[s_index+1]->id;
						next_node = this->best_returns[s_index+1];
					}
				}

				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_actions[s_index]->next_node_id = next_node_id;
					this->best_actions[s_index]->next_node = next_node;
				} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
					this->best_scopes[s_index]->next_node_id = next_node_id;
					this->best_scopes[s_index]->next_node = next_node;
				} else {
					this->best_returns[s_index]->passed_next_node_id = next_node_id;
					this->best_returns[s_index]->passed_next_node = next_node;
					this->best_returns[s_index]->skipped_next_node_id = next_node_id;
					this->best_returns[s_index]->skipped_next_node = next_node;
				}
			}

			this->obs_histories.reserve(NUM_DATAPOINTS);
			this->target_val_histories.reserve(NUM_DATAPOINTS);

			uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
