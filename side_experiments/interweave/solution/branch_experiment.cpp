#include "branch_experiment.h"

#include "globals.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->state = BRANCH_EXPERIMENT_STATE_EXISTING_GATHER;
	this->run_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

BranchExperimentHistory::BranchExperimentHistory(
		BranchExperiment* experiment) {
	this->experiment = experiment;

	uniform_int_distribution<int> experiment_active_distribution(0, 3);
	this->is_active = experiment_active_distribution(generator) == 0;

	this->instance_count = 0;
}
