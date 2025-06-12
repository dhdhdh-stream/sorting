#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
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
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

void BranchExperiment::abort() {
	this->result = EXPERIMENT_RESULT_FAIL;
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}

BranchExperimentState::BranchExperimentState(BranchExperiment* experiment) {
	this->experiment = experiment;
}

BranchExperimentState::~BranchExperimentState() {
	/**
	 * - catch early exit
	 */
	BranchExperiment* branch_experiment = (BranchExperiment*)this->experiment;
	if (branch_experiment->state == BRANCH_EXPERIMENT_STATE_EXPLORE) {
		while (this->step_index < (int)branch_experiment->curr_step_types.size()) {
			branch_experiment->curr_step_types.pop_back();
			branch_experiment->curr_actions.pop_back();
			branch_experiment->curr_scopes.pop_back();
		}
	}
}
