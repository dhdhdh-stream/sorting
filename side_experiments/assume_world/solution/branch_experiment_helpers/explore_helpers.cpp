#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_set.h"

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
	run_helper.num_analyze += (1 + 2*EXISTING_ANALYZE_SIZE) * (1 + 2*EXISTING_ANALYZE_SIZE);

	history->instance_count++;

	bool is_target = false;
	if (!history->has_target) {
		double target_probability;
		if (history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		history->has_target = true;

		vector<vector<double>> input_vals(1 + 2*EXISTING_ANALYZE_SIZE);
		for (int x_index = 0; x_index < 1 + 2*EXISTING_ANALYZE_SIZE; x_index++) {
			input_vals[x_index] = vector<double>(1 + 2*EXISTING_ANALYZE_SIZE);
		}

		Minesweeper* minesweeper = (Minesweeper*)problem;
		for (int x_index = -EXISTING_ANALYZE_SIZE; x_index < EXISTING_ANALYZE_SIZE+1; x_index++) {
			for (int y_index = -EXISTING_ANALYZE_SIZE; y_index < EXISTING_ANALYZE_SIZE+1; y_index++) {
				input_vals[x_index + EXISTING_ANALYZE_SIZE][y_index + EXISTING_ANALYZE_SIZE]
					= minesweeper->get_observation_helper(
						minesweeper->current_x + x_index,
						minesweeper->current_y + y_index);
			}
		}
		this->existing_network->activate(input_vals);
		double predicted_score = this->existing_network->output->acti_vals[0];

		history->existing_predicted_scores.push_back(predicted_score);

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
		}

		parent_scope->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->curr_exit_next_node = possible_exits[random_index];

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
				ScopeNode* new_scope_node = create_existing();
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

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->curr_actions[s_index]->action);
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

	if (history->has_target) {
		double final_score = target_val - solution_set->average_score;
		double curr_surprise = final_score - history->existing_predicted_scores[0];

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

			if (this->state_iter == EXPLORE_ITERS-1
					&& this->best_surprise > 0.0) {
				select = true;
			}
		} else if (this->explore_type == EXPLORE_TYPE_NEUTRAL) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (!run_helper.exceeded_limit
					&& curr_surprise >= 0.0) {
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
		} else if (this->explore_type == EXPLORE_TYPE_GOOD) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (!run_helper.exceeded_limit
					&& curr_surprise >= this->existing_score_standard_deviation) {
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

				new_ending_node->action = Action(ACTION_NOOP, 0);

				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;

				this->ending_node = new_ending_node;

				exit_node_id = new_ending_node->id;
				exit_node = new_ending_node;
			} else {
				exit_node_id = this->best_exit_next_node->id;
				exit_node = this->best_exit_next_node;
			}

			this->branch_node = new BranchNode();
			this->branch_node->parent = this->scope_context;
			this->branch_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;

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

			this->obs_histories.reserve(NUM_DATAPOINTS);
			this->target_val_histories.reserve(NUM_DATAPOINTS);

			uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
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
