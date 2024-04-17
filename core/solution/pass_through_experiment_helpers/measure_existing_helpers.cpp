#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::measure_existing_activate(
		PassThroughExperimentHistory* history) {
	history->instance_count++;
}

void PassThroughExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	this->o_target_val_histories.push_back(target_val);

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (this->parent_experiment == NULL) {
		if (!run_helper.exceeded_limit) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;
			}

			if (run_helper.num_actions > solution->max_num_actions) {
				solution->max_num_actions = run_helper.num_actions;
			}
		}
	}

	if ((int)this->o_target_val_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		uniform_int_distribution<int> in_place_distribution(0, 1);
		// if (in_place_distribution(generator) == 0) {
		if (false) {
			this->curr_exit_depth = 0;
			switch (this->node_context.back()->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context.back();
					this->curr_exit_next_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
					this->curr_exit_next_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context.back();
					if (this->is_branch) {
						this->curr_exit_next_node = branch_node->branch_next_node;
					} else {
						this->curr_exit_next_node = branch_node->original_next_node;
					}
				}
				break;
			}

			Scope* new_scope = new Scope();
			new_scope->id = -1;

			ActionNode* starting_noop_node = new ActionNode();
			starting_noop_node->parent = new_scope;
			starting_noop_node->id = 0;
			starting_noop_node->action = Action(ACTION_NOOP);
			new_scope->nodes[0] = starting_noop_node;
			new_scope->node_counter = 1;

			uniform_int_distribution<int> uniform_distribution(0, 1);
			geometric_distribution<int> geometric_distribution(0.5);
			int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

			uniform_int_distribution<int> default_distribution(0, 3);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool default_to_action = true;
				if (default_distribution(generator) != 0) {
					ScopeNode* new_scope_node = create_existing();
					if (new_scope_node != NULL) {
						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						default_to_action = false;
					}
				}

				if (default_to_action) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();

					new_action_node->parent = new_scope;
					new_action_node->id = new_scope->node_counter;
					new_scope->node_counter++;
					new_scope->nodes[new_action_node->id] = new_action_node;
				}
			}

			ActionNode* ending_noop_node = new ActionNode();
			ending_noop_node->parent = new_scope;
			ending_noop_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			ending_noop_node->action = Action(ACTION_NOOP);
			ending_noop_node->next_node_id = -1;
			ending_noop_node->next_node = NULL;
			new_scope->nodes[ending_noop_node->id] = ending_noop_node;

			for (int n_index = 0; n_index < (int)new_scope->nodes.size()-1; n_index++) {
				if (new_scope->nodes[n_index]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)new_scope->nodes[n_index];
					action_node->next_node_id = n_index+1;
					action_node->next_node = new_scope->nodes[n_index+1];
				} else {
					ScopeNode* scope_node = (ScopeNode*)new_scope->nodes[n_index];
					scope_node->next_node_id = n_index+1;
					scope_node->next_node = new_scope->nodes[n_index+1];
				}
			}

			{
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->scope = new_scope;

				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(NULL);

				this->curr_scopes.push_back(new_scope_node);
			}
			{
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->scope = new_scope;

				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(NULL);

				this->curr_scopes.push_back(new_scope_node);
			}
			{
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->scope = new_scope;

				this->curr_step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_actions.push_back(NULL);

				this->curr_scopes.push_back(new_scope_node);
			}
		} else {
			vector<pair<int,AbstractNode*>> possible_exits;

			if (this->node_context.back()->type == NODE_TYPE_ACTION
					&& ((ActionNode*)this->node_context.back())->next_node == NULL) {
				possible_exits.push_back({0, NULL});
			}

			gather_possible_exits(possible_exits,
								  this->scope_context,
								  this->node_context,
								  this->is_branch);

			if (possible_exits.size() == 0) {
				switch (this->node_context.back()->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)this->node_context.back();
						possible_exits.push_back({0, action_node->next_node});
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
						possible_exits.push_back({0, scope_node->next_node});
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)this->node_context.back();
						if (this->is_branch) {
							possible_exits.push_back({0, branch_node->branch_next_node});
						} else {
							possible_exits.push_back({0, branch_node->original_next_node});
						}
					}
					break;
				}
			}

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->curr_exit_depth = possible_exits[random_index].first;
			this->curr_exit_next_node = possible_exits[random_index].second;

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
		}

		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
