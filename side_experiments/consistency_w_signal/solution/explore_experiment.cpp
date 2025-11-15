#include "explore_experiment.h"

#include "abstract_node.h"
#include "network.h"

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

	this->existing_network = NULL;

	this->sum_num_instances = 0;

	this->total_count = 0;

	this->sum_scores = 0.0;

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

ExploreExperiment::~ExploreExperiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}
}

void ExploreExperiment::clean() {
	this->node_context->experiment = NULL;
}

ExploreExperimentHistory::ExploreExperimentHistory(ExploreExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;

	this->explore_instance = NULL;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
