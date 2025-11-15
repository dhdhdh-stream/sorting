#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_instance.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_EXPERIMENT_EXPLORE_ITERS = 10;
#else
const int EXPLORE_EXPERIMENT_EXPLORE_ITERS = 1000;
#endif /* MDEBUG */

void ExploreExperiment::explore_check_activate(SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	this->num_instances_until_target--;
	if (history->explore_instance == NULL
			&& this->num_instances_until_target <= 0) {
		wrapper->has_explore = true;

		ExploreInstance* explore_instance = new ExploreInstance();
		explore_instance->experiment = this;
		history->explore_instance = explore_instance;

		vector<AbstractNode*> possible_exits;

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				starting_node = start_node->next_node;
			}
			break;
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
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				starting_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		explore_instance->exit_next_node = possible_exits[random_index];

		uniform_int_distribution<int> new_scope_distribution(0, 1);
		if (new_scope_distribution(generator) == 0) {
			explore_instance->new_scope = create_new_scope(this->node_context->parent);
		}
		if (explore_instance->new_scope != NULL) {
			explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
			explore_instance->actions.push_back(-1);
			explore_instance->scopes.push_back(explore_instance->new_scope);
		} else {
			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (random_index == 0) {
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
					explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
					explore_instance->actions.push_back(-1);

					int child_index = possible_child_indexes[child_index_distribution(generator)];
					explore_instance->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
				} else {
					explore_instance->step_types.push_back(STEP_TYPE_ACTION);

					explore_instance->actions.push_back(-1);

					explore_instance->scopes.push_back(NULL);
				}
			}
		}

		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::explore_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		this->existing_network->activate(obs);
		history->existing_predicted_score = this->existing_network->output->acti_vals[0];

		history->stack_traces.push_back(wrapper->scope_histories);
	}

	if (experiment_state->step_index >= (int)history->explore_instance->step_types.size()) {
		wrapper->node_context.back() = history->explore_instance->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (history->explore_instance->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			is_next = true;
			fetch_action = true;

			wrapper->num_actions++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(history->explore_instance->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(history->explore_instance->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			inner_scope_history->pre_obs = wrapper->problem->get_observations();
		}
	}
}

void ExploreExperiment::explore_set_action(int action,
										   SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	history->explore_instance->actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void ExploreExperiment::explore_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.back()->post_obs = wrapper->problem->get_observations();

	wrapper->post_scope_histories.push_back(wrapper->scope_histories.back()->copy_signal());

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::explore_backprop(double target_val,
										 SolutionWrapper* wrapper) {
	ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;

	uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
	this->num_instances_until_target = until_distribution(generator);

	if (history->explore_instance != NULL) {
		history->explore_instance->post_scope_histories = wrapper->post_scope_histories;
		wrapper->post_scope_histories.clear();
		wrapper->has_explore = false;

		double sum_vals = target_val - wrapper->solution->curr_score;
		int sum_counts = 1;

		for (int l_index = 0; l_index < (int)history->stack_traces[0].size(); l_index++) {
			ScopeHistory* scope_history = history->stack_traces[0][l_index];
			Scope* scope = scope_history->scope;

			if (scope->pre_network != NULL) {
				if (!scope_history->signal_initialized) {
					scope->pre_network->activate(scope_history->pre_obs);
					scope_history->pre_val = scope->pre_network->output->acti_vals[0];

					vector<double> inputs = scope_history->pre_obs;
					inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

					scope->post_network->activate(inputs);
					scope_history->post_val = scope->post_network->output->acti_vals[0];

					scope_history->signal_initialized = true;
				}

				sum_vals += (scope_history->post_val - scope_history->pre_val);
				sum_counts++;
			}

			if (this->state_iter < EXPERIMENT_SAMPLES) {
				if ((int)scope->consistency_explore_pre_obs.size() < MAX_SAMPLES) {
					scope->consistency_explore_pre_obs.push_back(scope_history->pre_obs);
					scope->consistency_explore_post_obs.push_back(scope_history->post_obs);
				} else {
					scope->consistency_explore_pre_obs[scope->consistency_explore_index] = scope_history->pre_obs;
					scope->consistency_explore_post_obs[scope->consistency_explore_index] = scope_history->post_obs;
				}
				scope->consistency_explore_index++;
				if (scope->consistency_explore_index >= MAX_SAMPLES) {
					scope->consistency_explore_index = 0;
				}
			}
		}

		double average_val = sum_vals / sum_counts;
		history->explore_instance->surprise = this->average_hits_per_run * (average_val - history->existing_predicted_score);

		if ((wrapper->best_explore_instances.back() == NULL
				|| wrapper->best_explore_instances.back()->surprise < history->explore_instance->surprise)) {
			if (wrapper->best_explore_instances.back() != NULL) {
				delete wrapper->best_explore_instances.back();
			}
			wrapper->best_explore_instances.back() = history->explore_instance;

			int index = wrapper->best_explore_instances.size()-1;
			while (true) {
				if (wrapper->best_explore_instances[index-1] == NULL
						|| wrapper->best_explore_instances[index-1]->surprise < wrapper->best_explore_instances[index]->surprise) {
					ExploreInstance* temp = wrapper->best_explore_instances[index];
					wrapper->best_explore_instances[index] = wrapper->best_explore_instances[index-1];
					wrapper->best_explore_instances[index-1] = temp;
				} else {
					break;
				}

				index--;
				if (index == 0) {
					break;
				}
			}
		} else {
			delete history->explore_instance;
		}

		this->state_iter++;
		if (this->state_iter >= EXPLORE_EXPERIMENT_EXPLORE_ITERS) {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
