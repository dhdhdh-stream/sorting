#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch,
								   SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->node_context->experiment = this;

	this->sum_scores = 0.0;

	this->sum_instances = 0;

	this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

BranchExperiment::~BranchExperiment() {
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

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

BranchExperimentState::BranchExperimentState(BranchExperiment* experiment) {
	this->experiment = experiment;
}
