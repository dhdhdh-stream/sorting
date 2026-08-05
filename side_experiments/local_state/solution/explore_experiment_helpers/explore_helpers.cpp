#include "explore_experiment.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 10;
#else
const int EXPLORE_ITERS = 400;
#endif /* MDEBUG */

void ExploreExperiment::explore_check_activate(vector<double>& obs,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		this->num_instances_until_target--;
		if (history->existing_predicted.size() == 0
				&& this->num_instances_until_target <= 0) {
			this->existing_network->activate(wrapper->states.back());
			history->existing_predicted.push_back(this->existing_network->output->acti_vals(0));

			bool exit_is_next;
			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					if (this->exit_next_node == noop_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					if (this->exit_next_node == action_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					if (this->exit_next_node == scope_node->next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
				break;
			default:
			// case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						if (this->exit_next_node == branch_node->branch_next_node) {
							exit_is_next = true;
						} else {
							exit_is_next = false;
						}
					} else {
						if (this->exit_next_node == branch_node->original_next_node) {
							exit_is_next = true;
						} else {
							exit_is_next = false;
						}
					}
				}
				break;
			}

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (exit_is_next) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
				if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					history->curr_step_types.push_back(STEP_TYPE_SCOPE);
					history->curr_actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					history->curr_scopes.push_back(this->node_context->parent->child_scopes[child_index]);
				} else {
					history->curr_step_types.push_back(STEP_TYPE_ACTION);

					history->curr_actions.push_back(-1);

					history->curr_scopes.push_back(NULL);
				}
			}

			history->signal_histories.push_back(0.0);
			wrapper->scope_histories.back()->experiment_callback_histories.push_back(history);
			wrapper->scope_histories.back()->experiment_callback_indexes.push_back(history->signal_histories.size()-1);

			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::explore_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];

	if (experiment_state->step_index >= (int)history->curr_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (history->curr_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->run_num_actions++;
		} else {
			Scope* inner_scope = history->curr_scopes[experiment_state->step_index];
			ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(inner_scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			wrapper->states.push_back(Eigen::VectorXf());
			wrapper->states.back().resize(inner_scope->num_states);
			wrapper->states.back().setConstant(0.0);

			inner_scope->experiment_start_activate(
				obs,
				wrapper);
		}
	}
}

void ExploreExperiment::explore_set_action(int action,
										   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];

	history->curr_actions[experiment_state->step_index] = action;
}

void ExploreExperiment::explore_callback(vector<double>& obs,
										 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];

	int action = history->curr_actions[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];

	generic_action_node->action_network->activate(wrapper->states.back());

	generic_action_node->obs_network->activate(wrapper->states.back(),
											   obs);

	experiment_state->step_index++;
}

void ExploreExperiment::explore_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::explore_backprop(double target_val,
										 ExploreExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		double average_instances_per_hit;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				average_instances_per_hit = noop_node->average_instances_per_hit;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				average_instances_per_hit = action_node->average_instances_per_hit;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				average_instances_per_hit = scope_node->average_instances_per_hit;
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					average_instances_per_hit = branch_node->branch_average_instances_per_hit;
				} else {
					average_instances_per_hit = branch_node->original_average_instances_per_hit;
				}
			}
			break;
		}
		uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
		this->num_instances_until_target = until_distribution(generator);

		if (history->existing_predicted.size() != 0) {
			double curr_surprise = target_val - history->existing_predicted[0];

			#if defined(MDEBUG) && MDEBUG
			if (curr_surprise > this->best_surprise || true) {
			#else
			if (curr_surprise > this->best_surprise) {
			#endif /* MDEBUG */
				this->best_surprise = curr_surprise;
				this->best_step_types = history->curr_step_types;
				this->best_actions = history->curr_actions;
				this->best_scopes = history->curr_scopes;
			}

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->best_surprise >= 0.0) {
				#endif /* MDEBUG */
					this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_NEW;
					this->state_iter = 0;
				} else {
					delete this;
				}
			}
		}
	}
}
