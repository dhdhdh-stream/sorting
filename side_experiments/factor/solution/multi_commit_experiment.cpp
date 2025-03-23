#include "multi_commit_experiment.h"

#include "abstract_node.h"
#include "branch_experiment.h"
#include "globals.h"

using namespace std;

MultiCommitExperiment::MultiCommitExperiment(
		Scope* scope_context,
		AbstractNode* node_context,
		bool is_branch) {
	this->type = EXPERIMENT_TYPE_MULTI_COMMIT;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	uniform_int_distribution<int> until_distribution(0, (int)this->node_context->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	this->state = MULTI_COMMIT_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

MultiCommitExperiment::~MultiCommitExperiment() {
	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}
}

void MultiCommitExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

MultiCommitExperimentHistory::MultiCommitExperimentHistory(MultiCommitExperiment* experiment) {
	this->experiment = experiment;

	uniform_int_distribution<int> experiment_active_distribution(0, 2);
	switch (experiment->state) {
	case MULTI_COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
	case MULTI_COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
		this->is_active = false;
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_EXPLORE:
	case MULTI_COMMIT_EXPERIMENT_STATE_FIND_SAVE:
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER:
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
	case MULTI_COMMIT_EXPERIMENT_STATE_MEASURE:
	case MULTI_COMMIT_EXPERIMENT_STATE_OUTER_MEASURE:
		this->is_active = experiment_active_distribution(generator) == 0;
		break;
	}

	this->instance_count = 0;
}
