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

	this->instance_count = 0;
}
