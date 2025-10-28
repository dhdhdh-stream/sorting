#include "pass_through_experiment.h"

#include "abstract_node.h"
#include "scope.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(
		Scope* scope_context,
		AbstractNode* node_context,
		bool is_branch,
		SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->new_scope = NULL;

	this->node_context->experiment = this;

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

PassThroughExperiment::~PassThroughExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

PassThroughExperimentHistory::PassThroughExperimentHistory(PassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

PassThroughExperimentState::PassThroughExperimentState(PassThroughExperiment* experiment) {
	this->experiment = experiment;
}
