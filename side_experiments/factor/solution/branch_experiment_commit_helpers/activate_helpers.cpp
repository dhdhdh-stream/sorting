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
		return commit_explore_activate(curr_node,
									   problem,
									   run_helper,
									   scope_history,
									   history);
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_activate(scope_history);
		return false;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		return commit_train_new_activate(curr_node,
										 problem,
										 run_helper,
										 scope_history,
										 history);
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		return commit_measure_activate(curr_node,
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
		commit_explore_backprop(target_val,
								run_helper,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		commit_train_new_backprop(target_val,
								  run_helper,
								  history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		commit_measure_backprop(target_val);
		break;
	}
}
