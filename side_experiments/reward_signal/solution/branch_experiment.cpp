#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch,
								   Input reward_signal) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	if (reward_signal.scope_context.size() != 0) {
		this->reward_signal = reward_signal;

		ObsNode* obs_node = (ObsNode*)this->reward_signal.scope_context.back()
			->nodes[this->reward_signal.node_context.back()];
		Factor* factor = obs_node->factors[this->reward_signal.factor_index];
		this->reward_signal_average = factor->average;
		this->reward_signal_standard_deviation = factor->standard_deviation;
	}

	this->curr_scope_history = NULL;
	this->best_scope_history = NULL;

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

	this->has_explore = false;
}

BranchExperimentState::BranchExperimentState(BranchExperiment* experiment) {
	this->experiment = experiment;
}
