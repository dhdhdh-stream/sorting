#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

ExploreExperiment::ExploreExperiment(AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node,
									 double& existing_constant,
									 vector<Input>& existing_inputs,
									 vector<double>& existing_input_averages,
									 vector<double>& existing_input_standard_deviations,
									 vector<double>& existing_weights,
									 vector<Input>& existing_network_inputs,
									 Network*& existing_network) {
	this->type = EXPERIMENT_TYPE_EXPLORE;

	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->sum_num_following_explores = 0;
	this->sum_num_instances = 0;

	this->existing_constant = existing_constant;
	this->existing_inputs = existing_inputs;
	this->existing_input_averages = existing_input_averages;
	this->existing_input_standard_deviations = existing_input_standard_deviations;
	this->existing_weights = existing_weights;
	this->existing_network_inputs = existing_network_inputs;
	this->existing_network = existing_network;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->new_network = NULL;

	this->best_surprise = 0.0;

	/**
	 * - simply initialize to 1
	 */
	this->num_instances_until_target = 1;

	this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
	this->state_iter = 0;
}

ExploreExperiment::~ExploreExperiment() {
	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}

	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}

	for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
		delete this->scope_histories[h_index];
	}
}

ExploreExperimentHistory::ExploreExperimentHistory(
		ExploreExperiment* experiment,
		SolutionWrapper* wrapper) {
	this->is_on = false;
	if (wrapper->should_explore
			&& wrapper->curr_explore == NULL
			&& experiment->last_num_following_explores.size() > 0) {
		double average_num_follow = (double)experiment->sum_num_following_explores
			/ (double)experiment->last_num_following_explores.size();
		uniform_real_distribution<double> distribution(0.0, 1.0);
		double rand_val = distribution(generator);
		if (rand_val <= 1.0 / (1.0 + average_num_follow)) {
			wrapper->curr_explore = experiment;
			this->is_on = true;
		}
	}

	this->num_instances = 0;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
