#include "commit_experiment.h"

#include "abstract_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "problem.h"

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

	this->state = COMMIT_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

CommitExperiment::~CommitExperiment() {
	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void CommitExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

CommitExperimentHistory::CommitExperimentHistory(CommitExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}
