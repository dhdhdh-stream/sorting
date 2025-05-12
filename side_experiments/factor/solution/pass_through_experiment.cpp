#include "pass_through_experiment.h"

#include <iostream>
#include <limits>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(Scope* scope_context,
											 ObsNode* node_context) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->sum_score = 0.0;

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void PassThroughExperiment::decrement(ObsNode* experiment_node) {
	delete this;
}

PassThroughExperimentHistory::PassThroughExperimentHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;
}
