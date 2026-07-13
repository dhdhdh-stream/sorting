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

const int GATHER_DEPENDENCIES_NUM_TRIES = 10;
const int MAX_NUM_DEPENDENCIES = 4;

void compare_index(vector<int>& left, vector<int>& right, bool& right_later) {
	int layer = 0;
	while (true) {
		if (layer >= (int)left.size()) {
			right_later = false;
			return;
		} else if (layer >= (int)right.size()) {
			right_later = true;
			return;
		} else if (left[layer] < right[layer]) {
			right_later = true;
			return;
		} else if (left[layer] > right[layer]) {
			right_later = false;
			return;
		} else {
			layer++;
		}
	}
}

void ExploreExperiment::explore_check_activate(vector<double>& obs,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->num_instances_until_target--;
		if (history->existing_predicted.size() == 0
				&& this->num_instances_until_target <= 0) {
			history->curr_dependencies.push_back(vector<int>{this->node_context->id});
			vector<vector<int>> indexes;
			for (int try_index = 0; try_index < GATHER_DEPENDENCIES_NUM_TRIES; try_index++) {
				vector<int> curr_context;
				vector<int> curr_index;
				int count = 0;
				vector<int> dependency;
				vector<int> index;
				gather_dependencies_helper(wrapper->scope_histories.back(),
										   curr_context,
										   curr_index,
										   count,
										   dependency,
										   index);
				bool matches_existing = false;
				for (int d_index = 0; d_index < (int)history->curr_dependencies.size(); d_index++) {
					if (dependency == history->curr_dependencies[d_index]) {
						matches_existing = true;
						break;
					}
				}
				if (!matches_existing) {
					int insert_index = 0;
					for (int i_index = 0; i_index < (int)indexes.size(); i_index++) {
						bool existing_later;
						compare_index(index, indexes[i_index], existing_later);
						if (existing_later) {
							break;
						} else {
							insert_index++;
						}
					}
					indexes.insert(indexes.begin() + insert_index, index);
					history->curr_dependencies.insert(history->curr_dependencies.begin() + insert_index, dependency);

					if (history->curr_dependencies.size() >= MAX_NUM_DEPENDENCIES) {
						break;
					}
				}
			}

			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					noop_node->score_network->activate(wrapper->state);
					history->existing_predicted.push_back(noop_node->score_network->output->acti_vals(0));
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					action_node->score_network->activate(wrapper->state);
					history->existing_predicted.push_back(action_node->score_network->output->acti_vals(0));
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					scope_node->score_network->activate(wrapper->state);
					history->existing_predicted.push_back(scope_node->score_network->output->acti_vals(0));
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						branch_node->branch_network->activate(wrapper->state);
						history->existing_predicted.push_back(branch_node->branch_network->output->acti_vals(0));
					} else {
						branch_node->original_network->activate(wrapper->state);
						history->existing_predicted.push_back(branch_node->original_network->output->acti_vals(0));
					}
				}
				break;
			}

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

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(history->curr_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(history->curr_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			history->curr_scopes[experiment_state->step_index]->experiment_start_activate(
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

	ActionNetwork* action_network = wrapper->solution->generic_action_networks[action];
	action_network->activate(wrapper->state);
	ActionNetworkHistory* action_network_history = new ActionNetworkHistory(action_network);
	action_network->save(action_network_history);
	wrapper->network_histories.push_back(action_network_history);

	experiment_state->step_index++;
}

void ExploreExperiment::explore_callback(vector<double>& obs,
										 SolutionWrapper* wrapper) {
	ObsNetwork* obs_network = wrapper->solution->generic_obs_network;
	obs_network->activate(wrapper->state,
						  obs);
	ObsNetworkHistory* obs_network_history = new ObsNetworkHistory(obs_network);
	obs_network->save(obs_network_history);
	wrapper->network_histories.push_back(obs_network_history);
}

void ExploreExperiment::explore_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::explore_backprop(double target_val,
										 ExploreExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
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
				this->best_dependencies = history->curr_dependencies;
			}

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->best_surprise >= 0.0) {
				#endif /* MDEBUG */
					for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
						set_dependency_helper(this->scope_context,
											  this->best_dependencies[d_index],
											  0,
											  this);
					}

					this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_NEW;
					this->state_iter = 0;
				} else {
					delete this;
				}
			}
		}
	}
}
