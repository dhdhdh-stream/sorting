#include "pass_through_experiment.h"

#include <iostream>
#include <limits>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(Scope* scope_context,
											 AbstractNode* node_context,
											 bool is_branch) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->sum_score = 0.0;

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void PassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

PassThroughExperimentHistory::PassThroughExperimentHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;
}

PassThroughExperimentState::PassThroughExperimentState(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;
}

PassThroughExperimentState::~PassThroughExperimentState() {
	/**
	 * - catch early exit
	 */
	PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;
	switch (pass_through_experiment->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		if (!pass_through_experiment->is_init) {
			while (this->step_index < (int)pass_through_experiment->step_types.size()) {
				pass_through_experiment->step_types.pop_back();
				pass_through_experiment->actions.pop_back();
				pass_through_experiment->scopes.pop_back();
			}
		}
	}
}
