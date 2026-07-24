#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
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
	vector<Eigen::VectorXf> curr_dependencies_state(this->dependencies.size());
	vector<vector<double>> curr_dependencies_obs(this->dependencies.size());
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		bool is_hit;
		Eigen::VectorXf state;
		vector<double> obs;
		fetch_dependency_helper(wrapper->scope_histories.back(),
								this->dependencies[d_index],
								0,
								is_hit,
								state,
								obs);
		curr_dependencies_is_hit[d_index] = is_hit;
		curr_dependencies_state[d_index] = state;
		curr_dependencies_obs[d_index] = obs;
	}
	history->dependencies_is_hit_histories.push_back(curr_dependencies_is_hit);
	history->dependencies_state_histories.push_back(curr_dependencies_state);
	history->dependencies_obs_histories.push_back(curr_dependencies_obs);
	history->state_histories.push_back(wrapper->state);
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)history->state_histories.size(); i_index++) {
		this->existing_dependencies_is_hit_histories.push_back(history->dependencies_is_hit_histories[i_index]);
		this->existing_dependencies_state_histories.push_back(history->dependencies_state_histories[i_index]);
		this->existing_dependencies_obs_histories.push_back(history->dependencies_obs_histories[i_index]);
		this->existing_state_histories.push_back(history->state_histories[i_index]);
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
			shuffle(this->existing_dependencies_state_histories.begin(), this->existing_dependencies_state_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_dependencies_obs_histories.begin(), this->existing_dependencies_obs_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_state_histories.begin(), this->existing_state_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_target_val_histories.begin(), this->existing_target_val_histories.end(), generator_copy);
		}

		int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_dependencies_is_hit_histories.size();

		this->existing_network = new ScoreNetwork(this->existing_state_histories[0].size());
		double hidden_1_average_max_update = 0.0;
		double hidden_2_average_max_update = 0.0;
		double output_average_max_update = 0.0;

		uniform_int_distribution<int> train_distribution(0, num_existing_train-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = train_distribution(generator);

			this->existing_network->activate(this->existing_state_histories[rand_index]);

			this->existing_network->init_backprop(this->existing_target_val_histories[rand_index]);

			if ((iter_index+1)%EPOCH_SIZE == 0) {
				this->existing_network->init_update(hidden_1_average_max_update,
													hidden_2_average_max_update,
													output_average_max_update);
			}
		}
		for (int s_index = 0; s_index < (int)this->existing_network->state_input->errors.size(); s_index++) {
			this->existing_network->state_input->errors(s_index) = 0.0;
		}

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
