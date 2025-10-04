#include "explore_experiment.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

ExploreExperiment::ExploreExperiment(AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_EXPLORE;

	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->sum_num_following_explores = 0;
	this->sum_num_instances = 0;

	this->curr_new_scope = NULL;
	this->curr_scope_history = NULL;
	this->best_new_scope = NULL;
	this->best_scope_history = NULL;

	this->new_network = NULL;

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

ExploreExperiment::~ExploreExperiment() {
	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	if (this->curr_scope_history != NULL) {
		delete this->curr_scope_history;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}

	if (this->best_scope_history != NULL) {
		delete this->best_scope_history;
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
			&& experiment->state != EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING
			&& experiment->last_num_following_explores.size() > 0) {
		double average_num_follow = (double)experiment->sum_num_following_explores
			/ (double)experiment->last_num_following_explores.size();
		uniform_real_distribution<double> distribution(0.0, 1.0);
		double rand_val = distribution(generator);
		if (rand_val <= 1.0 / (1.0 + average_num_follow)) {
			uniform_int_distribution<int> on_distribution(0, 19);
			if (on_distribution(generator) == 0) {
				wrapper->curr_explore = experiment;
				this->is_on = true;
			}
		}
	}

	this->num_instances = 0;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
