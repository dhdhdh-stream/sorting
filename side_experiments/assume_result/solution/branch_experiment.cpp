#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	uniform_int_distribution<int> front_distribution(0, 1);
	geometric_distribution<int> back_distribution(0.7);
	this->new_analyze_size = front_distribution(generator) + back_distribution(generator);

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */

	this->new_network = NULL;

	this->ending_node = NULL;
	this->branch_node = NULL;

	uniform_int_distribution<int> best_distribution(0, 1);
	if (best_distribution(generator) == 0) {
		this->explore_type = EXPLORE_TYPE_BEST;

		this->best_surprise = 0.0;
	} else {
		this->explore_type = EXPLORE_TYPE_GOOD;
	}

	this->obs_histories.reserve(NUM_DATAPOINTS);
	this->target_val_histories.reserve(NUM_DATAPOINTS);

	uniform_int_distribution<int> until_distribution(0, (int)this->scope_context->average_instances_per_run-1.0);
	this->num_instances_until_target = 1 + until_distribution(generator);

	this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

BranchExperiment::~BranchExperiment() {
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	for (int s_index = 0; s_index < (int)this->curr_actions.size(); s_index++) {
		if (this->curr_actions[s_index] != NULL) {
			delete this->curr_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_scopes.size(); s_index++) {
		if (this->curr_scopes[s_index] != NULL) {
			delete this->curr_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_returns.size(); s_index++) {
		if (this->curr_returns[s_index] != NULL) {
			delete this->curr_returns[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_scopes.size(); s_index++) {
		if (this->best_scopes[s_index] != NULL) {
			delete this->best_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_returns.size(); s_index++) {
		if (this->best_returns[s_index] != NULL) {
			delete this->best_returns[s_index];
		}
	}

	if (this->ending_node != NULL) {
		delete this->ending_node;
	}
	if (this->branch_node != NULL) {
		delete this->branch_node;
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

	this->instance_count = 0;

	this->has_target = false;
}
