#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::train_existing_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
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
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)history->dependencies_is_hit_histories.size(); i_index++) {
		this->existing_dependencies_is_hit_histories.push_back(history->dependencies_is_hit_histories[i_index]);
		this->existing_dependencies_obs_histories.push_back(history->dependencies_obs_histories[i_index]);
		this->existing_target_val_histories.push_back(target_val);
	}

	this->sum_vals += target_val;

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_val_average = this->sum_vals / this->state_iter;

		vector<InitNetwork*> potential_init_networks(this->dependencies.size());
		ScoreNetwork* potential_new_network;
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			Scope* scope = get_dependency_scope(this->scope_context,
												this->dependencies[d_index],
												0);
			vector<int> init_states;
			for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
				init_states.push_back(scope->num_states + s_index);
			}
			potential_init_networks[d_index] = new InitNetwork(
				init_states,
				wrapper->solution->num_obs);
		}
		{
			vector<int> init_states;
			for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
				init_states.push_back(this->scope_context->num_states + s_index);
			}
			potential_new_network = new ScoreNetwork(init_states);
		}

		uniform_int_distribution<int> new_train_distribution(0, this->existing_dependencies_is_hit_histories.size()-1);
		uniform_int_distribution<int> partial_distribution(0, 19);
		for (int iter_index = 0; iter_index < NEW_STATE_TRAIN_ITERS; iter_index++) {
			int rand_index = new_train_distribution(generator);

			vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);

			vector<bool> is_activate(this->dependencies.size(), false);
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				if (this->existing_dependencies_is_hit_histories[rand_index][d_index]) {
					if (iter_index < NEW_STATE_PARTIAL_START_ITERS
							|| partial_distribution(generator) != 0) {
						is_activate[d_index] = true;

						potential_init_networks[d_index]->init_activate(
							new_state,
							this->existing_dependencies_obs_histories[rand_index][d_index]);
					}
				}
			}

			potential_new_network->init_activate(new_state);

			vector<double> new_state_errors(NEW_STATE_NUM_ADD, 0.0);

			potential_new_network->init_backprop(this->existing_target_val_histories[rand_index],
												 new_state_errors);

			for (int d_index = (int)this->dependencies.size()-1; d_index >= 0; d_index--) {
				if (is_activate[d_index]) {
					potential_init_networks[d_index]->init_backprop(new_state_errors);
				}
			}

			if ((iter_index+1)%INIT_EPOCH_SIZE == 0) {
				for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
					potential_init_networks[d_index]->init_update();
				}
				potential_new_network->init_update();
			}
		}
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			for (int i_index = 0; i_index < (int)potential_init_networks[d_index]->state_input->errors.size(); i_index++) {
				potential_init_networks[d_index]->state_input->errors(i_index) = 0.0;
			}
		}
		for (int i_index = 0; i_index < (int)potential_new_network->state_input->errors.size(); i_index++) {
			potential_new_network->state_input->errors(i_index) = 0.0;
		}

		this->existing_init_networks = potential_init_networks;
		this->existing_network = potential_new_network;

		this->best_surprise = numeric_limits<double>::lowest();

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

		this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
