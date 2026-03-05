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

ExploreExperiment::ExploreExperiment(ObsNode* node_context,
									 AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_EXPLORE;

	this->node_context = node_context;
	this->exit_next_node = exit_next_node;

	this->sum_num_following_explores = 0;
	this->sum_num_instances = 0;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
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
}

ExploreExperimentHistory::ExploreExperimentHistory(
		ExploreExperiment* experiment,
		SolutionWrapper* wrapper) {
	this->is_on = false;
	switch (experiment->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
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
		break;
	}

	this->num_instances = 0;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
