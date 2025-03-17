#include "multi_branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "problem.h"

using namespace std;

MultiBranchExperiment::MultiBranchExperiment(
		Scope* scope_context,
		AbstractNode* node_context,
		bool is_branch) {
	this->type = EXPERIMENT_TYPE_MULTI_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	uniform_int_distribution<int> until_distribution(0, (int)this->node_context->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	this->state = MULTI_BRANCH_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void MultiBranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

MultiBranchExperimentHistory::MultiBranchExperimentHistory(MultiBranchExperiment* experiment) {
	this->experiment = experiment;

	uniform_int_distribution<int> experiment_active_distribution(0, 2);
	switch (experiment->state) {
	case MULTI_BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
	case MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
	case MULTI_BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		this->is_active = false;
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_EXPLORE:
	case MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
	case MULTI_BRANCH_EXPERIMENT_STATE_MEASURE:
		this->is_active = experiment_active_distribution(generator) == 0;
		break;
	}

	this->instance_count = 0;
}
