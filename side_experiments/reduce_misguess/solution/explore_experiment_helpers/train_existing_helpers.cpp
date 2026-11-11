#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::train_existing_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	history->obs_histories.push_back(obs);
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	for (int i_index = 0; i_index < (int)history->obs_histories.size(); i_index++) {
		this->existing_obs_histories.push_back(history->obs_histories[i_index]);
		this->existing_target_val_histories.push_back(target_val);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_score_network = new Network(this->existing_obs_histories[0].size());
		double score_hidden_1_average_max_update = 0.0;
		double score_hidden_2_average_max_update = 0.0;
		double score_hidden_3_average_max_update = 0.0;
		double score_output_average_max_update = 0.0;
		this->existing_misguess_network = new Network(this->existing_obs_histories[0].size());
		double misguess_hidden_1_average_max_update = 0.0;
		double misguess_hidden_2_average_max_update = 0.0;
		double misguess_hidden_3_average_max_update = 0.0;
		double misguess_output_average_max_update = 0.0;

		uniform_int_distribution<int> train_distribution(0, this->existing_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_EXISTING_ITERS; iter_index++) {
			int rand_index = train_distribution(generator);

			this->existing_score_network->activate(this->existing_obs_histories[rand_index]);
			this->existing_misguess_network->activate(this->existing_obs_histories[rand_index]);

			double score_error = this->existing_target_val_histories[rand_index] - this->existing_score_network->output->acti_vals[0];

			double misguess = abs(score_error);
			double misguess_error = misguess - this->existing_misguess_network->output->acti_vals[0];

			this->existing_score_network->init_backprop(
				score_error,
				score_hidden_1_average_max_update,
				score_hidden_2_average_max_update,
				score_hidden_3_average_max_update,
				score_output_average_max_update);

			this->existing_misguess_network->init_backprop(
				misguess_error,
				misguess_hidden_1_average_max_update,
				misguess_hidden_2_average_max_update,
				misguess_hidden_3_average_max_update,
				misguess_output_average_max_update);
		}

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
