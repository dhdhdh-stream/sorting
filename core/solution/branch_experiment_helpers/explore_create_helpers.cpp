#include "branch_experiment.h"

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

void BranchExperiment::explore_create_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
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

		/**
		 * - set this->exit_throw_id while still have full context
		 */
		uniform_int_distribution<int> throw_distribution(0, 3);
		// if (throw_distribution(generator) == 0) {
		if (false) {
			uniform_int_distribution<int> reuse_existing_throw_distribution(0, 1);
			if (reuse_existing_throw_distribution(generator) == 0) {
				/**
				 * - simply allow duplicates
				 */
				vector<int> possible_throw_ids;
				for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
					ScopeNode* scope_node = (ScopeNode*)context[c_index].node;
					for (map<int, AbstractNode*>::iterator it = scope_node->catches.begin();
							it != scope_node->catches.end(); it++) {
						possible_throw_ids.push_back(it->first);
					}
				}

				if (possible_throw_ids.size() > 0) {
					uniform_int_distribution<int> possible_distribution(0, possible_throw_ids.size()-1);
					this->curr_exit_throw_id = possible_throw_ids[possible_distribution(generator)];
				} else {
					this->curr_exit_throw_id = TEMP_THROW_ID;
				}
			} else {
				this->curr_exit_throw_id = TEMP_THROW_ID;
			}
		} else {
			this->curr_exit_throw_id = -1;
		}
	}
}

void branch_gather_possible_exits_helper(vector<int>& experiment_index,
										 vector<pair<int,AbstractNode*>>& possible_exits,
										 ScopeHistory* scope_history) {
	if (experiment_index.size() > 1) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[experiment_index[0]-1];

		vector<int> inner_experiment_index(experiment_index.begin()+1, experiment_index.end());
		branch_gather_possible_exits_helper(inner_experiment_index,
											possible_exits,
											scope_node_history->scope_history);
	}

	for (int h_index = experiment_index[0]; h_index < (int)scope_history->node_histories.size(); h_index++) {
		possible_exits.push_back({experiment_index.size()-1, scope_history->node_histories[h_index]->node});
	}

	possible_exits.push_back({experiment_index.size()-1, NULL});
}

void BranchExperiment::explore_create_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target
			&& !run_helper.exceeded_limit) {
		vector<pair<int,AbstractNode*>> possible_exits;

		branch_gather_possible_exits_helper(history->experiment_index,
											possible_exits,
											history->scope_history);

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

						this->curr_catch_throw_ids.push_back(set<int>());

						default_to_action = false;
					}
				}

				if (default_to_action) {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();
					this->curr_actions.push_back(new_action_node);

					this->curr_scopes.push_back(NULL);
					this->curr_catch_throw_ids.push_back(set<int>());
				}
			}

			this->state = BRANCH_EXPERIMENT_STATE_EXPLORE_MEASURE;
		}
	}
}
