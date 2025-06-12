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

	this->state = COMMIT_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

CommitExperiment::~CommitExperiment() {
	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}
}

void CommitExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

CommitExperimentHistory::CommitExperimentHistory(CommitExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}

CommitExperimentState::CommitExperimentState(CommitExperiment* experiment) {
	this->experiment = experiment;
}

CommitExperimentState::~CommitExperimentState() {
	/**
	 * - catch early exit
	 */
	CommitExperiment* commit_experiment = (CommitExperiment*)this->experiment;
	switch (commit_experiment->state) {
	case COMMIT_EXPERIMENT_STATE_EXPLORE:
		while (this->step_index < (int)commit_experiment->curr_step_types.size()) {
			commit_experiment->curr_step_types.pop_back();
			commit_experiment->curr_actions.pop_back();
			commit_experiment->curr_scopes.pop_back();
		}
		break;
	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
		if (!commit_experiment->save_is_init) {
			while (this->step_index < (int)commit_experiment->save_step_types.size()) {
				commit_experiment->save_step_types.pop_back();
				commit_experiment->save_actions.pop_back();
				commit_experiment->save_scopes.pop_back();
			}
		}
		break;
	}
}
