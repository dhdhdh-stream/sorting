#include "explore_experiment.h"

#include "abstract_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

ExploreExperiment::ExploreExperiment(Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->sum_num_instances = 0;

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

ExploreExperiment::~ExploreExperiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}

	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}
}

bool ExploreExperiment::further_than(ExploreExperiment* other) {
	if (this->state < other->state) {
		return false;
	} else if (this->state > other->state) {
		return true;
	} else {
		if (this->state_iter <= other->state_iter) {
			return false;
		} else {
			return true;
		}
	}
}

ExploreExperimentHistory::ExploreExperimentHistory(ExploreExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
