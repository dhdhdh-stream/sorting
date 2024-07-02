#include "seed_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_set.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SEED_EXPLORE_ITERS = 5;
#else
const int SEED_EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void SeedExperiment::explore_seed_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		SeedExperimentHistory* history) {
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
		this->curr_seed_exit_next_node = possible_exits[random_index];

		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

		uniform_int_distribution<int> default_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			if (default_distribution(generator) != 0) {
				ScopeNode* new_scope_node = create_existing(parent_scope);
				if (new_scope_node != NULL) {
					this->curr_seed_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_seed_actions.push_back(NULL);

					this->curr_seed_scopes.push_back(new_scope_node);

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_seed_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_seed_actions.push_back(new_action_node);

				this->curr_seed_scopes.push_back(NULL);
			}
		}

		for (int s_index = 0; s_index < (int)this->curr_seed_step_types.size(); s_index++) {
			if (this->curr_seed_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->curr_seed_actions[s_index]->action);
			} else {
				this->curr_seed_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}
		}

		curr_node = this->curr_seed_exit_next_node;
	}
}

void SeedExperiment::explore_seed_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

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

void SeedExperiment::explore_seed_backprop(
		double target_val,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target) {
		double curr_surprise;
		if (!run_helper.exceeded_limit) {
			double final_score;
			switch (this->score_type) {
			case SCORE_TYPE_TRUTH:
				final_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
				break;
			case SCORE_TYPE_ALL:
				{
					double sum_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
					for (int l_index = 0; l_index < (int)history->predicted_scores[0].size(); l_index++) {
						sum_score += history->predicted_scores[0][l_index];
					}
					final_score = sum_score / ((int)history->predicted_scores[0].size() + 1);
				}
				break;
			}

			curr_surprise = final_score - this->existing_average_score;
		}

		#if defined(MDEBUG) && MDEBUG
		if (!run_helper.exceeded_limit) {
		#else
		if (!run_helper.exceeded_limit
				&& curr_surprise > this->best_seed_surprise) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
				if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_seed_actions[s_index];
				} else {
					delete this->best_seed_scopes[s_index];
				}
			}

			this->best_seed_surprise = curr_surprise;
			this->best_seed_step_types = this->curr_seed_step_types;
			this->best_seed_actions = this->curr_seed_actions;
			this->best_seed_scopes = this->curr_seed_scopes;
			this->best_seed_exit_next_node = this->curr_seed_exit_next_node;

			this->curr_seed_step_types.clear();
			this->curr_seed_actions.clear();
			this->curr_seed_scopes.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_seed_step_types.size(); s_index++) {
				if (this->curr_seed_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_seed_actions[s_index];
				} else {
					delete this->curr_seed_scopes[s_index];
				}
			}

			this->curr_seed_step_types.clear();
			this->curr_seed_actions.clear();
			this->curr_seed_scopes.clear();
		}

		if (this->state_iter == SEED_EXPLORE_ITERS-1
				&& this->best_seed_surprise > 0.0) {
			for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
				if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_seed_actions[s_index]->parent = this->scope_context;
					this->best_seed_actions[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				} else {
					this->best_seed_scopes[s_index]->parent = this->scope_context;
					this->best_seed_scopes[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				}
			}

			this->scope_histories.reserve(NUM_DATAPOINTS);
			this->target_val_histories.reserve(NUM_DATAPOINTS);

			uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->state = SEED_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			this->state_iter++;
			if (this->state_iter >= SEED_EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
