#include "branch_experiment.h"

#include <iostream>

#include "globals.h"

using namespace std;

void BranchExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	BranchExperimentHistory* history;
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
		= run_helper.experiment_histories.find(this);
	if (it == run_helper.experiment_histories.end()) {
		history = new BranchExperimentHistory(this);
		run_helper.experiment_histories[this] = history;
	} else {
		history = (BranchExperimentHistory*)it->second;
	}

	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_activate(scope_history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_activate(scope_history,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_activate(curr_node,
						 problem,
						 run_helper,
						 scope_history,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_activate(scope_history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_activate(curr_node,
						   problem,
						   run_helper,
						   scope_history,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_activate(curr_node,
						 problem,
						 run_helper,
						 scope_history,
						 history);
		break;
	}
}

void BranchExperiment::update(AbstractExperimentHistory* history,
							  double target_val) {
	BranchExperimentHistory* branch_experiment_history = (BranchExperimentHistory*)history;
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_update();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_update(branch_experiment_history,
							  target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_update(branch_experiment_history,
					   target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_update();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_update(branch_experiment_history,
						 target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_update(branch_experiment_history,
					   target_val);
		break;
	}
}
