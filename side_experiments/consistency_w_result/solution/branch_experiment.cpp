#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	uniform_int_distribution<int> good_distribution(0, 3);
	if (good_distribution(generator) == 0) {
		this->explore_type = EXPLORE_TYPE_GOOD;
	} else {
		this->explore_type = EXPLORE_TYPE_BEST;
	}

	this->best_surprise = 0.0;

	uniform_int_distribution<int> until_distribution(0, (int)this->node_context->average_instances_per_run-1.0);
	this->num_instances_until_target = 1 + until_distribution(generator);

	this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

BranchExperiment::~BranchExperiment() {
	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}
