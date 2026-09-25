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

#if defined(MDEBUG) && MDEBUG
const int PREDICT_CYCLE_NUM_TRIES = 2;
#else
const int PREDICT_CYCLE_NUM_TRIES = 10;
#endif /* MDEBUG */

void ExploreExperiment::train_existing_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	history->obs_histories.push_back(obs);
	history->state_histories.push_back(wrapper->states.back());
}

void ExploreExperiment::train_existing_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if ((int)this->existing_obs_histories.size() < TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_obs_histories.push_back(history->obs_histories);
		this->existing_state_histories.push_back(history->state_histories);
		this->existing_target_val_histories.push_back(target_val);
	} else {
		this->existing_obs_histories[this->existing_index] = history->obs_histories;
		this->existing_state_histories[this->existing_index] = history->state_histories;
		this->existing_target_val_histories[this->existing_index] = target_val;
	}
	this->existing_index++;
	if (this->existing_index >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_index = 0;
	}

	this->state_iter++;
}

void ExploreExperiment::train_existing_helper(SolutionWrapper* wrapper) {
	this->existing_network = new Network(this->existing_obs_histories[0][0].size());
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double hidden_3_average_max_update = 0.0;
	double output_average_max_update = 0.0;

	uniform_int_distribution<int> run_distribution(0, this->existing_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_EXISTING_ITERS; iter_index++) {
		int run_index = run_distribution(generator);

		uniform_int_distribution<int> instance_distribution(0, this->existing_obs_histories[run_index].size()-1);
		int instance_index = instance_distribution(generator);

		this->existing_network->activate(this->existing_obs_histories[run_index][instance_index]);

		double error = this->existing_target_val_histories[run_index] - this->existing_network->output->acti_vals[0];

		this->existing_network->init_backprop(error,
											  hidden_1_average_max_update,
											  hidden_2_average_max_update,
											  hidden_3_average_max_update,
											  output_average_max_update);
	}

	bool predict_success = false;
	if (wrapper->solution->cycle_index != -1
			&& wrapper->solution->iter_index != 0) {
		for (int i_index = 0; i_index < PREDICT_CYCLE_NUM_TRIES; i_index++) {
			bool is_success = predict_cycle();
			if (is_success) {
				predict_success = true;
				break;
			}
		}
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

	if (predict_success) {
		this->sum_improvement = 0.0;

		this->state = EXPLORE_EXPERIMENT_STATE_PREDICT_MEASURE;
		this->state_iter = 0;
	} else {
		this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
