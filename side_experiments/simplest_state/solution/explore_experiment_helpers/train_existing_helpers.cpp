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
	vector<bool> curr_dependencies_is_hit(this->dependencies.size());
	vector<vector<double>> curr_dependencies_obs(this->dependencies.size());
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		bool is_hit;
		vector<double> obs;
		fetch_dependency_helper(wrapper->scope_histories.back(),
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

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_dependencies_is_hit_histories.begin(), this->existing_dependencies_is_hit_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_dependencies_obs_histories.begin(), this->existing_dependencies_obs_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_target_val_histories.begin(), this->existing_target_val_histories.end(), generator_copy);
		}

		int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_dependencies_is_hit_histories.size();

		this->existing_init_networks = vector<InitNetwork*>(this->dependencies.size());
		vector<int> init_states;
		for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
			init_states.push_back(wrapper->solution->num_states + s_index);
		}
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			this->existing_init_networks[d_index] = new InitNetwork(
				init_states,
				wrapper->solution->num_obs);
		}
		vector<double> hidden_1_average_max_updates(this->dependencies.size(), 0.0);
		vector<double> hidden_2_average_max_updates(this->dependencies.size(), 0.0);
		vector<double> output_average_max_updates(this->dependencies.size(), 0.0);
		this->existing_network = new ScoreNetwork(init_states);
		double hidden_1_average_max_update = 0.0;
		double hidden_2_average_max_update = 0.0;
		double output_average_max_update = 0.0;
		this->existing_state_means = vector<double>(NEW_STATE_NUM_ADD, 0.0);
		this->existing_state_diffs = vector<double>(NEW_STATE_NUM_ADD, 1.0);

		uniform_int_distribution<int> train_distribution(0, num_existing_train-1);
		uniform_int_distribution<int> noise_run_distribution(0, 3);
		uniform_int_distribution<int> is_noise_distribution(0, 9);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = train_distribution(generator);
			bool is_noise_run = noise_run_distribution(generator) == 0;

			vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);

			vector<bool> is_activate(this->dependencies.size(), false);
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				if (this->existing_dependencies_is_hit_histories[rand_index][d_index]) {
					if (is_noise_run
							&& is_noise_distribution(generator) == 0) {
						for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
							normal_distribution<double> distribution(0.0, this->existing_state_diffs[s_index]);
							new_state[s_index] += distribution(generator);
						}
					}

					this->existing_init_networks[d_index]->init_activate(
						new_state,
						this->existing_dependencies_obs_histories[rand_index][d_index]);
				}
			}

			this->existing_network->init_activate(new_state);

			for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
				this->existing_state_means[s_index] = 0.99999*this->existing_state_means[s_index] + 0.00001*new_state[s_index];
				double curr_diff = abs(new_state[s_index] - this->existing_state_means[s_index]);
				this->existing_state_diffs[s_index] = 0.99999*this->existing_state_diffs[s_index] + 0.00001*curr_diff;
			}

			vector<double> new_state_errors(NEW_STATE_NUM_ADD, 0.0);

			this->existing_network->init_backprop(this->existing_target_val_histories[rand_index],
												   new_state_errors);

			for (int d_index = (int)this->dependencies.size()-1; d_index >= 0; d_index--) {
				if (this->existing_dependencies_is_hit_histories[rand_index][d_index]) {
					this->existing_init_networks[d_index]->init_backprop(new_state_errors);
				}
			}

			if ((iter_index+1)%EPOCH_SIZE == 0) {
				for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
					this->existing_init_networks[d_index]->init_update(
						hidden_1_average_max_updates[d_index],
						hidden_2_average_max_updates[d_index],
						output_average_max_updates[d_index]);
				}
				this->existing_network->init_update(hidden_1_average_max_update,
													hidden_2_average_max_update,
													output_average_max_update);
			}
		}

		// // temp
		// for (int h_index = 0; h_index < 10; h_index++) {
		// 	vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
		// 	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		// 		if (this->existing_dependencies_is_hit_histories[h_index][d_index]) {
		// 			this->existing_init_networks[d_index]->init_activate(
		// 				new_state,
		// 				this->existing_dependencies_obs_histories[h_index][d_index]);
		// 		}
		// 	}
		// 	this->existing_network->init_activate(new_state);

		// 	cout << h_index << endl;
		// 	cout << "this->existing_network->output->acti_vals(0): " << this->existing_network->output->acti_vals(0) << endl;
		// 	cout << "this->existing_target_val_histories[h_index]: " << this->existing_target_val_histories[h_index] << endl;
		// }

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
