#include "branch_experiment.h"

using namespace std;

bool BranchExperiment::commit_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   RunHelper& run_helper,
									   ScopeHistory* scope_history,
									   ScopeHistory* temp_history,
									   BranchExperimentHistory* history) {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_commit_activate(scope_history,
										temp_history);
		return false;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_commit_activate(scope_history,
									   temp_history);
		return false;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_commit_activate(curr_node,
								problem,
								run_helper,
								scope_history,
								temp_history,
								history);
		return true;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_commit_activate(scope_history,
								   temp_history);
		return false;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_commit_activate(curr_node,
								  problem,
								  run_helper,
								  scope_history,
								  temp_history,
								  history);
		return true;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		return measure_commit_activate(curr_node,
									   problem,
									   run_helper,
									   scope_history,
									   temp_history);
	}

	return false;
}
