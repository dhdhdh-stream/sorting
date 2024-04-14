#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::explore_create_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
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

		context[context.size() - this->scope_context.size()].scope_history->experiment_history = history;

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			history->experiment_index.push_back(context[context.size() - this->scope_context.size() + c_index].scope_history->node_histories.size());
		}
	}
}

void pass_through_gather_possible_exits_helper(
		vector<int>& experiment_index,
		vector<pair<int,AbstractNode*>>& possible_exits,
		ScopeHistory* scope_history) {
	if (experiment_index.size() > 1) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[experiment_index[0]-1];

		vector<int> inner_experiment_index(experiment_index.begin()+1, experiment_index.end());
		pass_through_gather_possible_exits_helper(
			inner_experiment_index,
			possible_exits,
			scope_node_history->scope_history);
	}

	for (int h_index = experiment_index[0]; h_index < (int)scope_history->node_histories.size(); h_index++) {
		possible_exits.push_back({experiment_index.size()-1, scope_history->node_histories[h_index]->node});
	}
}

void PassThroughExperiment::explore_create_backprop(
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target
			&& !run_helper.exceeded_limit) {
		vector<pair<int,AbstractNode*>> possible_exits;

		if (this->node_context.back()->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context.back())->next_node == NULL) {
			possible_exits.push_back({0, NULL});
		}

		pass_through_gather_possible_exits_helper(
			history->experiment_index,
			possible_exits,
			history->scope_history);

		/**
		 * - can be empty from ScopeNode exit_nodes
		 */
		if (possible_exits.size() > 0) {
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
			uniform_int_distribution<int> curr_scope_distribution(0, 2);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool default_to_action = true;
				if (default_distribution(generator) != 0) {
					Scope* scope;
					if (solution->scopes[solution->curr_scope_id]->layer == 0
							|| curr_scope_distribution(generator) == 0) {
						scope = solution->scopes[solution->curr_scope_id];
					} else {
						uniform_int_distribution<int> child_distribution(0, MAX_NUM_CHILDREN-1);
						int scope_id = solution->scopes[solution->curr_scope_id]->child_ids[child_distribution(generator)];
						scope = solution->scopes[scope_id];
					}
					ScopeNode* new_scope_node = create_existing(scope,
																run_helper);
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

			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_MEASURE;
			/**
			 * - leave this->state_iter unchanged
			 */
			this->sub_state_iter = 0;
		}
	}
}
