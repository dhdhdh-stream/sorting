#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::train_new_check_activate(vector<double>& obs,
												 ExploreExperimentHistory* history,
												 SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		vector<bool> curr_dependencies_is_hit(this->dependencies.size());
		vector<vector<double>> curr_dependencies_obs(this->dependencies.size());
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			bool is_hit;
			Eigen::VectorXf state;
			vector<double> obs;
			fetch_dependency_helper(scope_history,
									this->dependencies[d_index],
									0,
									is_hit,
									obs);
			curr_dependencies_is_hit[d_index] = is_hit;
			curr_dependencies_obs[d_index] = obs;
		}
		history->dependencies_is_hit_histories.push_back(curr_dependencies_is_hit);
		history->dependencies_obs_histories.push_back(curr_dependencies_obs);
		history->obs_histories.push_back(obs);

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
		uniform_int_distribution<int> until_distribution(1, average_instances_per_hit);
		this->num_instances_until_target = until_distribution(generator);

		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->run_num_actions++;
		} else {
			Scope* scope = this->scope_context->child_scopes[this->best_indexes[experiment_state->step_index]];
			ScopeHistory* inner_scope_history = new ScopeHistory(scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			wrapper->states.push_back(Eigen::VectorXf());
			wrapper->states.back().resize(scope->num_states);
			wrapper->states.back().setConstant(0.0);
		}
	}
}

void ExploreExperiment::train_new_callback(vector<double>& obs,
										   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_exit_step(vector<double>& obs,
											SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_backprop(double target_val,
										   ExploreExperimentHistory* history,
										   SolutionWrapper* wrapper) {
	if (history->dependencies_is_hit_histories.size() > 0) {
		for (int i_index = 0; i_index < (int)history->dependencies_is_hit_histories.size(); i_index++) {
			this->new_dependencies_is_hit_histories.push_back(history->dependencies_is_hit_histories[i_index]);
			this->new_dependencies_obs_histories.push_back(history->dependencies_obs_histories[i_index]);
			this->new_obs_histories.push_back(history->obs_histories[i_index]);
			this->new_target_val_histories.push_back(target_val);
		}

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_TRAIN_NEW_NUM_DATAPOINTS) {
			new_state_helper(wrapper);
		}
	}
}
