#include "branch_experiment.h"

using namespace std;

bool BranchExperiment::commit_activate(AbstractNode*& curr_node,
									   Problem* problem,
									   RunHelper& run_helper,
									   ScopeHistory* scope_history,
									   BranchExperimentHistory* history) {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_activate(scope_history);
		return false;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_activate(run_helper,
								scope_history,
								history);
		return false;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_commit_activate(curr_node,
								problem,
								run_helper,
								scope_history,
								history);
		return true;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_activate(scope_history);
		return false;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_activate(curr_node,
						   problem,
						   run_helper,
						   scope_history,
						   history);
		return true;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		return measure_commit_activate(curr_node,
									   problem,
									   run_helper,
									   scope_history);
	}

	return false;
}

void BranchExperiment::commit_backprop(double target_val,
									   RunHelper& run_helper,
									   BranchExperimentHistory* history) {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_commit_backprop(target_val);
		break;
	}
}
