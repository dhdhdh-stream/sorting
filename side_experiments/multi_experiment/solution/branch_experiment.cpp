#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "problem.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch,
								   AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->state = BRANCH_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	uniform_int_distribution<int> experiment_active_distribution(0, 2);
	switch (experiment->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		this->is_active = false;
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		this->is_active = experiment_active_distribution(generator) == 0;
		break;
	}

	this->instance_count = 0;
}
