#include "commit_experiment.h"

#include "abstract_node.h"
#include "branch_experiment.h"
#include "globals.h"

using namespace std;

CommitExperiment::CommitExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_COMMIT;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */

	uniform_int_distribution<int> until_distribution(0, (int)this->node_context->average_instances_per_run-1);
	this->num_instances_until_target = 1 + until_distribution(generator);

	this->state = COMMIT_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->best_experiment = NULL;
	this->curr_experiment = NULL;

	this->result = EXPERIMENT_RESULT_NA;
}

CommitExperiment::~CommitExperiment() {
	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}

	if (this->best_experiment != NULL) {
		delete this->best_experiment;
	}
	if (this->curr_experiment != NULL) {
		delete this->curr_experiment;
	}
}

void CommitExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

CommitExperimentHistory::CommitExperimentHistory(CommitExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->branch_experiment_history = NULL;
}

CommitExperimentHistory::~CommitExperimentHistory() {
	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}
}
