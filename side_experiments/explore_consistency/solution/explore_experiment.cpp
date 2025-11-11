#include "explore_experiment.h"

#include "abstract_node.h"

using namespace std;

ExploreExperiment::ExploreExperiment(Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_EXPLORE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void ExploreExperiment::clean() {
	this->node_context->experiment = NULL;
}

ExploreExperimentHistory::ExploreExperimentHistory(ExploreExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;

	this->num_instances = 0;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
