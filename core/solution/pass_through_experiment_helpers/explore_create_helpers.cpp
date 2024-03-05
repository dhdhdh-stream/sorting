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

		vector<pair<int,AbstractNode*>> possible_exits;
		gather_possible_exits(possible_exits,
							  this->scope_context,
							  this->node_context,
							  this->is_branch,
							  this->throw_id,
							  run_helper);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->curr_exit_depth = possible_exits[random_index].first;
		this->curr_exit_next_node = possible_exits[random_index].second;

		uniform_int_distribution<int> throw_distribution(0, 3);
		if (throw_distribution(generator) == 0) {
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

		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 2);
		geometric_distribution<int> geometric_distribution(0.5);
		if (this->curr_exit_depth == 0
				&& this->curr_exit_next_node == curr_node) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		uniform_int_distribution<int> new_scope_distribution(0, 3);
		uniform_int_distribution<int> random_scope_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			ScopeNode* new_scope_node = NULL;
			if (new_scope_distribution(generator) == 0) {
				if (random_scope_distribution(generator) == 0) {
					uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
					Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
					new_scope_node = create_scope(scope,
												  run_helper);
				} else {
					new_scope_node = create_scope(context[context.size() - this->scope_context.size()].scope,
												  run_helper);
				}
			}
			if (new_scope_node != NULL) {
				this->curr_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
				this->curr_actions.push_back(NULL);
				this->curr_existing_scopes.push_back(NULL);

				this->curr_potential_scopes.push_back(new_scope_node);
			} else {
				ScopeNode* new_existing_scope_node = reuse_existing();
				if (new_existing_scope_node != NULL) {
					this->curr_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_existing_scopes.push_back(new_existing_scope_node);

					this->curr_potential_scopes.push_back(NULL);
				} else {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();
					this->curr_actions.push_back(new_action_node);

					this->curr_existing_scopes.push_back(NULL);
					this->curr_potential_scopes.push_back(NULL);
				}
			}
		}
	}
}

void PassThroughExperiment::explore_create_backprop(
		PassThroughExperimentHistory* history) {
	if (history->has_target) {
		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_MEASURE;
		/**
		 * - leave this->state_iter unchanged
		 */
		this->sub_state_iter = 0;
	}
}
