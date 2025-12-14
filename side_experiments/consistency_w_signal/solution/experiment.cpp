#include "experiment.h"

#include "abstract_node.h"
#include "network.h"

using namespace std;

Experiment::Experiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch,
					   SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	this->existing_network = NULL;

	this->sum_num_instances = 0;

	this->total_count = 0;

	this->sum_scores = 0.0;

	this->state = EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

Experiment::~Experiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}
}

ExperimentHistory::ExperimentHistory(Experiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

ExperimentState::ExperimentState(Experiment* experiment) {
	this->experiment = experiment;
}
