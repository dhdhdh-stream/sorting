#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->curr_scope_history = NULL;
	this->best_scope_history = NULL;

	this->sum_num_instances = 0;

	this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

BranchExperiment::~BranchExperiment() {
	if (this->curr_scope_history != NULL) {
		delete this->curr_scope_history;
	}

	if (this->best_scope_history != NULL) {
		delete this->best_scope_history;
	}

	for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
		delete this->scope_histories[h_index];
	}
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
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
